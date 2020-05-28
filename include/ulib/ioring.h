#ifdef HAVE_SCHED_GETAFFINITY && LINUX_VERSION_CODE >= KERNEL_VERSION(5,7,0))

#pragma once

#include <linux/io_uring.h>

/* From tools/include/linux/compiler.h */
/* Optimization barrier */
/* The "volatile" is due to gcc bugs */
#define io_uring_barrier()	__asm__ __volatile__("": : :"memory")

/* From tools/virtio/linux/compiler.h */
#define IO_URING_WRITE_ONCE(var, val) 	(*((volatile __typeof(val) *)(&(var))) = (val))
#define IO_URING_READ_ONCE(var) 			(*((volatile __typeof(var) *)(&(var))))

/* Adapted from arch/x86/include/asm/barrier.h */
#define io_uring_smp_store_release(p, v)	\
do {						\
	io_uring_barrier();			\
	IO_URING_WRITE_ONCE(*(p), (v));		\
} while (0)

#define io_uring_smp_load_acquire(p)			\
({							\
	__typeof(*p) ___p1 = IO_URING_READ_ONCE(*(p));	\
	io_uring_barrier();				\
	___p1;						\
})

#define NUMBER_OF_READ_BUFFERS 3
#define READ_BUFFER_SIZE	3'670'016 // 3.5 MB in bytes
#define READ_BUFFER_GROUP_ID 0xA

#define NUMBER_OF_WRITE_BUFFERS 3
#define WRITE_BUFFER_SIZE	3'670'016 // 3.5 MB in bytes
#define WRITE_BUFFER_GROUP_ID 0xB

// make sure all sockets are nonblocking

class UIORingSlave {
public:

	enum class SocketOperation : uint8_t
	{
		none,
		read,
		write,
		accept
	};

	int fd;
	SocketOperation pendingOp; // last operation
	int8_t usingWriteBufferAtIndex;

	virtual void handlerAccept(int fd);
	virtual bool handlerRead(const UString& message); // turn false if we should close the socket

	UIORingSlave() : usingWriteBufferAtIndex(-1), pendingOp(SocketOperation::none), fd(-1) {}
};

class UIORing {
private:

	struct SubmissionQueue {

		uint32_t *head; 			// the head is incremented by the kernel when the I/O has been successfully submitted
		uint32_t *tail; 			// The tail is incremented by the application when submitting new I/O
		uint32_t *ring_mask;  	// Determining the index of the head or tail into the ring is accomplished by applying a mask aka... index = tail & ring_mask;
		uint32_t *ring_entries; 
		uint32_t *flags; 			// IORING_SQ_NEED_WAKEUP set here if kernel thread goes idle
		uint32_t *dropped; 		// The dropped member is incremented for each invalid submission queue entry encountered in the ring buffer.
		uint32_t *array; 

		struct io_uring_sqe *sqes;

		uint32_t sqe_head;
		uint32_t sqe_tail;

		size_t size;
		void *pointer;

		// maybe we can use a fixed write buffer, but rotating reader buffers IORING_OP_WRITE_FIXED

		// submission queue ring is of size IORING_MAX_ENTRIES
		// completion queue ring is of size IORING_MAX_ENTRIES * 2
		// you can submit up to IORING_MAX_ENTRIES per batch, but you can have an unlimited number of pending submissions, but must make sure the completion queue doesn't overflow

		void map(io_uring_params& params, int ring_fd)
		{
			head = pointer + params.sq_off.head;
			tail = pointer + params.sq_off.tail;
			ring_mask = pointer + params.sq_off.ring_mask;
			ring_entries = pointer + params.sq_off.ring_entries;
			flags = pointer + params.sq_off.flags;
			dropped = pointer + params.sq_off.dropped;
			array = pointer + params.sq_off.array;

			sqes = mmap(NULL, params.sq_entries * sizeof(struct io_uring_sqe), (PROT_READ | PROT_WRITE), (MAP_SHARED | MAP_POPULATE), ring_fd, IORING_OFF_SQES);
		}

		struct io_uring_sqe* entryFor(UIORingSlave *slave, int op, const void *addr = U_NULLPTR, uint32_t len = 0, uint64_t offset = 0)
		{
			// we could check for overflow like such ((sqe_tail + 1) - io_uring_smp_load_acquire(head) <= ring_entries), but in reality we will never send more than the maximum... which is 32768 sqes per submission
			struct io_uring_sqe *sqe = sqes[sqe_tail++ & *ring_mask];

			sqe->opcode = op;
			sqe->flags = IOSQE_FIXED_FILE; // signals that we pass an index into socketfds rather than a file descriptor
			sqe->ioprio = 0;
			sqe->fd = slave->fd; // index of the worker's fd in socketfds, but equal to fd
			sqe->off = offset;
			sqe->addr = (unsigned long) addr;
			sqe->len = len;
			sqe->rw_flags = 0;
			sqe->user_data = worker;
			sqe->__pad2[0] = sqe->__pad2[1] = sqe->__pad2[2] = 0;
			
			return sqe;
		}

		// increment submisison queue tail so the kernel will know we added work
		// does not require a syscall to io_uring_enter because we set IORING_SETUP_SQPOLL, and we set the kernel thread to never goes to sleep
		static int flush()
		{
			const unsigned mask = *(ring_mask);
			uint32_t newTail, to_submit;

			if (sqe_head == sqe_tail) 
			{
				newTail = *tail;
				goto out;
			}

			newTail = *tail;
			to_submit = sqe_tail - sqe_head;

			while (to_submit--) 
			{
				array[newTail & mask] = sqe_head & mask;
				newTail++;
				sqe_head++;
			}

			// when this changes the tail value, the kernel polling thread will by notified (unless it's gone to sleep)
			io_uring_smp_store_release(tail, newTail);

		out:
			return newTail - *head;
		}
	};

	struct CompletionQueue {

		uint32_t *head;
		uint32_t *tail;
		uint32_t *ring_mask;
		uint32_t *ring_entries;
		uint32_t *flags;
		uint32_t *overflow;

		struct io_uring_cqe *cqes;

		size_t size;
		void *pointer;

		void map(io_uring_params& params)
		{
			head = pointer + params.cq_off.head;
			tail = pointer + params.cq_off.tail;
			ring_mask = pointer + params.cq_off.ring_mask;
			ring_entries = pointer + params.cq_off.ring_entries;
			overflow = pointer + params.cq_off.overflow;
			
			cqes = pointer + params.cq_off.cqes;

			if (params.cq_off.flags) flags = pointer + params.cq_off.flags;
		}

		io_uring_cqe* peek_cqe()
		{
			return (*head != io_uring_smp_load_acquire(tail) ? &cqes[*head & *ring_mask] : NULL);
		}

		io_uring_cqe* getNextAndWait(int32_t ringfd)
		{
			struct io_uring_cqe *cqe = NULL;

			// check if there's a completion already waiting
			cqe = completionQueue.peek_cqe();

			// this will block until something is ready
			if (!cqe) 
			{
				io_uring_enter(ringfd, 0, 1, IORING_ENTER_GETEVENTS, NULL);
				cqe = completionQueue.peek_cqe();
			}

			return cqe;
		}

		void advance()
		{
			io_uring_smp_store_release(head, *head + 1);
		}
	};

	struct SubmissionQueue submissionQueue;
	struct CompletionQueue completionQueue;
	int32_t ringfd;
	int32_t *socketfds;

	uint8_t readBuffers[NUMBER_OF_READ_BUFFERS][READ_BUFFER_SIZE];
	uint8_t writeBuffers[NUMBER_OF_WRITE_BUFFERS][WRITE_BUFFER_SIZE];
	struct iovec writeBufferVecs[NUMBER_OF_WRITE_BUFFERS];
	uint8_t availableWriteBuffers[NUMBER_OF_WRITE_BUFFERS];

	void registerBuffers();
	void mapInStructuresFromKernel(io_uring_params& params);

	void toggleFD(UIORingSlave *slave)
	{
		/*
		IORING_REGISTER_FILES_UPDATE

			This operation replaces existing files in the registered file set with new ones, either turning a sparse entry (one where fd is equal to -1) into a real one, removing an existing entry (new one is set to -1), or replacing an existing entry with a new existing entry. arg must contain a pointer to a struct io_uring_files_update, which contains an offset on which to start the update, and an array of file descriptors to use for the update. nr_args must contain the number of descriptors in the passed in array. Available since 5.5.
		*/

		struct io_uring_files_update update = {.offset = slave->fd, .fds	= (unsigned long)allPossibleFDs};

		// result < 0 if error	
		(void) io_uring_register(ioring.fd, IORING_REGISTER_FILES_UPDATE, &update, 1);
	}

	~UIORing()
	{
		// Closing the file descriptor returned by io_uring_setup(2) will free all resources associated with the io_uring context.

		munmap(submissionQueue.sqes, *(submissionQueue.ring_entries) * sizeof(struct io_uring_sqe));

		if (completionQueue.pointer != submissionQueue.pointer) munmap(completionQueue.pointer, completionQueue.size);
		munmap(submissionQueue.pointer, submissionQueue.size);

		close(ringfd);
	}

	void accept(UIORingSlave* accepter)
	{
		// we issue a prospective accept, and as soon as a client starts to accept this will automatically trigger
		struct io_uring_sqe *sqe = submissionQueue.entryFor(accepter, IORING_OP_ACCEPT);
		submissionQueue.flush();
	}

	void waitForEvent()
	{
		struct io_uring_cqe *cqe;

		while (true)
		{
			cqe = completionQueue.getNextAndWait(ringfd);

			UIORingSlave *slave = (UIORingSlave *)cqe->user_data;

			switch (slave->pendingOp)
			{
				case SocketOperation::accept:
				{
					// standard accept errors -> ENETDOWN, EPROTO, ENOPROTOOPT, EHOSTDOWN, ENONET, EHOSTUNREACH, EOPNOTSUPP, ENETUNREACH
					if (UNLIKELY(cqe->res < 0))
					{

					}
					else
					{
						int newfd = cqe->res;
						slave->handlerAccept(newfd); // make sure this queues a read
						toggleFD(newfd);

						// queue another accept
						accept(slave);
					}
					
					break;
				}
				case SocketOperation::read:
				{
					if (UNLIKELY(cqe->res < 0))
					{
						// we ran out of read buffers 
						if (cqe->res == -ENOBUFS) read(slave);
					}
					else
					{
						// all our read operations will have this set because we only used buffered reads
						//if (cqe->flags & IORING_CQE_F_BUFFER) 
						int bufferID = (cqe->flags >> 16);

						UString message;
						message.setConstant(readBuffers[bufferID], cqe->res);

						// queue another read
						if (slave->handlerRead(message)) read(slave);
						else 										close(slave);
					}
					
					break;
				}
				case SocketOperation::write:
				{
					if (UNLIKELY(cqe->res < 0))
					{

					}
					else
					{
						if (writer->usingWriteBufferAtIndex > -1) availableWriteBuffers[writer->usingWriteBufferAtIndex] = 1;
					}
					break;
				}
				default: break; // never
			}

			// signal that we consumed one
			completionQueue.advance();
		};
	}

public:

	UIORing(uint32_t maxConnections);

	void read(UIORingSlave *reader)
	{
		struct io_uring_sqe *sqe = submissionQueue.entryFor(reader, IORING_OP_READ);
		sqe->flags |= IOSQE_BUFFER_SELECT;
		sqe->buf_group = READ_BUFFER_GROUP_ID;
		submissionQueue.flush();
	}

	void close(UIORingSlave *closer)
	{
		struct io_uring_sqe *sqe = submissionQueue.entryFor(writer, IORING_OP_CLOSE);
		submissionQueue.flush();
		toggleFD(closer);
	}

	// get a zero-copy write buffer is available. these can not be resized so your write must fit within the prespecified length, otherwise use a copy write
	uint8_t* getWriteBuffer(UIORingSlave *writer)
	{
		for (uint8_t index = 0; index < NUMBER_OF_WRITE_BUFFERS; index++)
		{
			if (availableWriteBuffers[index] == 1)
			{
				availableWriteBuffers[index] = 0;
				writer->usingWriteBufferAtIndex = index;
				return writeBuffers[index];
			}
		}

		return U_NULLPTR;
	}

	void write(UIORingSlave *writer, uint8_t *buffer, uint32_t length)
	{
		// they used a fixed write buffer
		if (writer->usingWriteBufferAtIndex > -1)
		{
			struct io_uring_sqe *sqe = submissionQueue.entryFor(writer, IORING_OP_WRITE_FIXED, buffer, length);
			sqe->buf_index = writer->usingWriteBufferAtIndex;
		}
		else
		{
			struct io_uring_sqe *sqe = submissionQueue.entryFor(writer, IORING_OP_WRITE, buffer, length);
		}

		submissionQueue.flush();
	}

	void startServer(UIORingSlave* server)
	{
		toggleFD(server);
		accept(server);
		waitForEvent();
	}
};

#endif
