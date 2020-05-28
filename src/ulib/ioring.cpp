#include "UIORing.h"

#define IORING_MAX_ENTRIES	32768

UIORing::UIORing(uint32_t maxConnections)
{
	// // +100 so we can add some client sockets if need be
	maxConnections += 100;

	// struct io_uring_params {
	//     __u32 sq_entries;
	//     __u32 cq_entries;
	//     __u32 flags;
	//     __u32 sq_thread_cpu;
	//     __u32 sq_thread_idle;
	//     __u32 features;
	//     __u32 resv[4];
	//     struct io_sqring_offsets sq_off;
	//     struct io_cqring_offsets cq_off;
	// };

	io_uring_params params;
  	memset(&params, 0, sizeof(params));

  	// Pin kernel submission polling thread to same CPU as we pinned in src/ulib/base/utility.c
   cpu_set_t affinity;
   CPU_ZERO(&affinity);
   sched_get_affinity(0, sizeof(affinity), &affinity);

   for (size_t n = 0; n < CPU_SET_SIZE; n++)
   {
      if (CPU_ISSET(n, &affinity))
      {
         params.sq_thread_cpu = n;
         break;
      }
   }

   /*
  		IORING_SETUP_SQPOLL
  			* kernel side polling for submission queue additions, so as soon as we push to the ring, the kernel will pick it up. must be run as root to use this
  		IORING_SETUP_SQ_AFF
  			* pin IORING_SETUP_SQPOLL pin the IORING_SETUP_SQPOLL poll thread to the CPU specified by sq_thread_cpu
  	*/

  	params.flags = (IORING_SETUP_SQPOLL | IORING_SETUP_SQ_AFF);

  	// as soon as we stop generating submission queue entries, a timer starts that will put the kernel side polling thread to sleep
  	// as long as we keep driving IO, the kernel thread will always stay active... otherwise it will set IORING_SQ_NEED_WAKEUP bit in the flags field of the struct io_sq_ring and you have to call enter to wake it up
  	params.sq_thread_idle = UINT32_MAX; // aka tell the kernel to never stop polling

  	// kernel will set sqeEntries to IORING_MAX_ENTRIES and cqeEntries to IORING_MAX_ENTRIES * 2
  	ringfd = io_uring_setup(IORING_MAX_ENTRIES, &params);
  	flags = params.flags;
  	mapInStructuresFromKernel(params);

  	/* 
		IORING_REGISTER_FILES

	  		* IORING_SETUP_SQPOLL requires this
	  		* adds amazing performance benefits
	  		* To make use of the registered files, the IOSQE_FIXED_FILE flag must be set in the flags member of the struct io_uring_sqe, and the fd member is set to the index of the file in the file descriptor array.
	  		* The file set may be sparse, meaning that the fd field in the array may be set to -1. See IORING_REGISTER_FILES_UPDATE for how to update files in place.
  	*/

  	fds = (int32_t*) UMemoryPool::u_malloc(maxConnections, sizeof(int32_t), true);
  	// make sparse with -1
	memset(&fds, -1, sizeof(int32_t));
  	// register an array large enough to contain all possible file descriptors for io
  	int result = io_uring_register(ringfd, IORING_REGISTER_FILES, &fds, maxConnections);

  	registerBuffers();
}

void UIORing::registerBuffers()
{
	for (uint32_t index = 0; index < NUMBER_OF_WRITE_BUFFERS; index++) 
	{
		writeBufferVecs[index].iov_base = writeBuffers[index];
		writeBufferVecs[index].iov_len = WRITE_BUFFER_SIZE;
	}

	io_uring_register(ringfd, IORING_REGISTER_BUFFERS, &writeBufferVecs, NUMBER_OF_WRITE_BUFFERS);

	for (uint32_t index = 0; i < NUMBER_OF_READ_BUFFERS; index++) 
	{
		struct io_uring_sqe *sqe = submissionQueue.entryFor(U_NULLPTR, IORING_OP_PROVIDE_BUFFERS, (readBuffers + index * READ_BUFFER_SIZE), READ_BUFFER_SIZE, index);
		sqe->buf_group = READ_BUFFER_GROUP_ID;
	}

	submissionQueue.flush();

	for (uint32_t index = 0; i < NUMBER_OF_READ_BUFFERS; index++) 
	{
		io_uring_cqe* cqe = completionQueue.getNextAndWait(ringfd);

		// if (cqe->res < 0) error

		completionQueue.advance();
	}
}

void UIORing::mapInStructuresFromKernel(io_uring_params& params)
{
	submissionQueue.size = params.sq_off.array + params.sq_entries * sizeof(uint32_t);
	completionQueue.size = params.cq_off.cqes + params.cq_entries * sizeof(struct io_uring_cqe);

	if (params.features & IORING_FEAT_SINGLE_MMAP) 
	{
		if (completionQueue.size > submissionQueue.size) submissionQueue.size = completionQueue.size;
		completionQueue.size = submissionQueue.size;
	}

	submissionQueue.pointer = mmap(NULL, submissionQueue.size, (PROT_READ | PROT_WRITE), (MAP_SHARED | MAP_POPULATE), fd, IORING_OFF_SQ_RING);
	submissionQueue.map(params, fd);

	if (params.features & IORING_FEAT_SINGLE_MMAP) 
	{
		completionQueue.pointer = submissionQueue.pointer;
	}
	else 
	{
		completionQueue.pointer = mmap(NULL, completionQueue.size, (PROT_READ | PROT_WRITE), (MAP_SHARED | MAP_POPULATE), fd, IORING_OFF_CQ_RING);
	}

	completionQueue.map(params);
}

/*
 * Ensure that the mmap'ed rings aren't available to a child after a fork(2).
 * This uses madvise(..., MADV_DONTFORK) on the mmap'ed ranges.
 */
// int io_uring_ring_dontfork(struct io_uring *ring)
// {
// 	size_t len;
// 	int ret;

// 	if (!ring->sq.ring_ptr || !ring->sq.sqes || !ring->cq.ring_ptr)
// 		return -EINVAL;

// 	len = *ring->sq.kring_entries * sizeof(struct io_uring_sqe);
// 	ret = madvise(ring->sq.sqes, len, MADV_DONTFORK);
// 	if (ret == -1)
// 		return -errno;

// 	len = ring->sq.ring_sz;
// 	ret = madvise(ring->sq.ring_ptr, len, MADV_DONTFORK);
// 	if (ret == -1)
// 		return -errno;

// 	if (ring->cq.ring_ptr != ring->sq.ring_ptr) {
// 		len = ring->cq.ring_sz;
// 		ret = madvise(ring->cq.ring_ptr, len, MADV_DONTFORK);
// 		if (ret == -1)
// 			return -errno;
// 	}

// 	return 0;
// }

// bool isKernelThreadAsleep()
	// {
	// 	return (IO_URING_READ_ONCE(*submissionQueue.flags) & IORING_SQ_NEED_WAKEUP);
	// }

	// int submit()
	// {
	// 	// notify the kernel thread that we added a new item
	// 	unsigned numberOfSubmittedEntries = submissionQueue.flush();

	// 	// unless the kernel has fallen asleep it will already have picked up our submissions
	// 	if (isKernelThreadAsleep()) 
	// 	{	
	// 		int ret = io_uring_enter(ringfd, numberOfSubmittedEntries, 0, (IORING_ENTER_SQ_WAKEUP | IORING_ENTER_GETEVENTS), NULL);

	// 		// When the system call returns that a certain amount of SQEs have been consumed and submitted, it's safe to reuse SQE entries in the ring. 

	// 		if (ret < 0) return -errno;
	// 	} 

	// 	return numberOfSubmittedEntries;
	// }
