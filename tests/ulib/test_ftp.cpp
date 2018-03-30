// test_ftp.cpp

#include <ulib/url.h>
#include <ulib/file.h>
#include <ulib/net/client/ftp.h>

struct proc_data {
   int fd;
   size_t offset, size;
   UString server, path;
};

static char* map;

static void download(struct proc_data* p)
{
   U_TRACE(5, "download(%p)", p)

   U_INTERNAL_DUMP("offset = %ld size = %ld", p->offset, p->size)

   ssize_t value;
   char* start = map + p->offset;
   size_t bytes_read = 0;

   do {
      value       = U_SYSCALL(recv, "%d,%p,%u,%d", p->fd, start + bytes_read, p->size - bytes_read, 0);
      bytes_read += value;

   // info->bytes_read += value;

      U_INTERNAL_DUMP("bytes_read = %.8ld", bytes_read)
   // U_INTERNAL_DUMP("bytes_read = %.8ld info->bytes_read = %.8ld", bytes_read, info->bytes_read)
      }
   while (value > 0 && bytes_read < p->size);
}

int U_EXPORT main(int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   struct proc_data vp;

   UFile file;
   UFtpClient ftp;
   UString tmp(argv[1], strlen(argv[1]));
   Url url(tmp);

   vp.server = url.getHost().copy();
   vp.path   = url.getPath();

   UString name = vp.path.substr(vp.path.find_last_of('/') + 1);

   file.setPath(name);

   if (file.creat() == false) return 1;

   vp.offset = 0;
   vp.fd     = ftp.download(vp.server, vp.path, vp.offset);
   vp.size   = ftp.getFileSize();

   if (vp.fd == -1 || file.ftruncate(vp.size) == false) return 1;

   file.memmap(PROT_READ | PROT_WRITE);

   map = file.getMap();

   download(&vp);

   file.munmap();
}
