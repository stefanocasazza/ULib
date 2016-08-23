/* ziptool.c - functions for zip archive utility */

/*
#define DEBUG_DEBUG
*/

#include <ulib/base/utility.h>
#include <ulib/base/zip/ziptool.h>
#include <ulib/base/zip/dostime.h>
#include <ulib/base/zip/zipentry.h>
#include <ulib/base/zip/zipfile.h>
#include <ulib/base/zip/pushback.h>
#include <ulib/base/zip/compress.h>

/* global variables */

static zipentry* ziplist;     /* linked list of entries */
static zipentry* ziptail;     /* tail of the linked list */
static int number_of_entries; /* number of entries in the linked list */

static ub1 file_header[30];
static ub1 data_descriptor[16];

static void init_headers(void)
{
   U_INTERNAL_TRACE("init_headers()")

   /* packing file header */

   /* magic number */
   file_header[0] = 0x50;
   file_header[1] = 0x4b;
   file_header[2] = 0x03;
   file_header[3] = 0x04;
   /* version number */
   file_header[4] = 10;
   file_header[5] = 0;
   /* bit flag (normal deflation)*/
   file_header[6] = 0x00;

   file_header[7] = 0x00;
   /* do_compression method (deflation) */
   file_header[8] = 0;
   file_header[9] = 0;

   /* last mod file time (MS-DOS format) */
   file_header[10] = 0;
   file_header[11] = 0;
   /* last mod file date (MS-DOS format) */
   file_header[12] = 0;
   file_header[13] = 0;
   /* CRC 32 */
   file_header[14] = 0;
   file_header[15] = 0;
   file_header[16] = 0;
   file_header[17] = 0;
   /* compressed size */
   file_header[18] = 0;
   file_header[19] = 0;
   file_header[20] = 0;
   file_header[21] = 0;
   /* uncompressed size */
   file_header[22] = 0;
   file_header[23] = 0;
   file_header[24] = 0;
   file_header[25] = 0;
   /* filename length */
   file_header[26] = 0;
   file_header[27] = 0;
   /* extra field length */
   file_header[28] = 0;
   file_header[29] = 0;

   /* Initialize the compression DS */
   PACK_UB4(data_descriptor, 0, 0x08074b50);
}

static int create_central_header(int fd)
{
   zipentry* ze;
   int dir_size;
   int start_offset;
   ub1 header[46];
   ub1 end_header[22];
   int total_in = 0, total_out = 22;

   U_INTERNAL_TRACE("create_central_header(%d)", fd)

   /* magic number */
   header[0] = 'P';
   header[1] = 'K';
   header[2] = 1;
   header[3] = 2;
   /* version made by */
   header[4] = 20;
   header[5] = 0;
   /* version needed to extract */
   header[6] = 10;
   header[7] = 0;
   /* bit flag */
   header[8] = 0;
   header[9] = 0;
   /* compression method */
   header[10] = 0;
   header[11] = 0;
   /* file mod time */
   header[12] = 0;
   header[13] = 0;
   /* file mod date */
   header[14] = 0;
   header[15] = 0;
   /* crc 32 */
   header[16] = 0;
   header[17] = 0;
   header[18] = 0;
   header[19] = 0;
   /* compressed size */
   header[20] = 0;
   header[21] = 0;
   header[22] = 0;
   header[23] = 0;
   /* uncompressed size */
   header[24] = 0;
   header[25] = 0;
   header[26] = 0;
   header[27] = 0;
   /* filename length */
   header[28] = 0;
   header[29] = 0;
   /* extra field length */
   header[30] = 0;
   header[31] = 0;
   /* file comment length */
   header[32] = 0;
   header[33] = 0;
   /* disk number start */
   header[34] = 0;
   header[35] = 0;
   /* internal file attribs */
   header[36] = 0;
   header[37] = 0;
   /* external file attribs */
   header[38] = 0;
   header[39] = 0;
   header[40] = 0;
   header[41] = 0;
   /* relative offset of local header */
   header[42] = 0;
   header[43] = 0;
   header[44] = 0;
   header[45] = 0;

   start_offset = lseek(fd, 0, SEEK_CUR);

   for (ze = ziptail; ze != 0; ze = ze->next_entry)
      {
      total_in  += ze->usize;
      total_out += ze->csize + 76 + u__strlen(ze->filename, __PRETTY_FUNCTION__) * 2;

      if (ze->compressed)
         {
         PACK_UB2(header, CEN_COMP, 8);
         }
      else
         {
         PACK_UB2(header, CEN_COMP, 0);
         }

      PACK_UB2(header, CEN_MODTIME, ze->mod_time);
      PACK_UB2(header, CEN_MODDATE, ze->mod_date);
      PACK_UB4(header, CEN_CRC,     ze->crc);
      PACK_UB4(header, CEN_CSIZE,   ze->csize);
      PACK_UB4(header, CEN_USIZE,   ze->usize);
      PACK_UB2(header, CEN_FNLEN,   u__strlen(ze->filename, __PRETTY_FUNCTION__));
      PACK_UB4(header, CEN_OFFSET,  ze->offset);

      (void) write(fd, header,       46);
      (void) write(fd, ze->filename, u__strlen(ze->filename, __PRETTY_FUNCTION__));
      }

   dir_size = lseek(fd, 0, SEEK_CUR) - start_offset;

   /* signature for ZIP end of central directory record */
   end_header[0] = 0x50;
   end_header[1] = 0x4b;
   end_header[2] = 0x05;
   end_header[3] = 0x06;
   /* number of this disk */
   end_header[4] = 0;
   end_header[5] = 0;
   /* number of disk w/ start of central header */
   end_header[6] = 0;
   end_header[7] = 0;
   /* total number of entries in central dir on this disk*/
   PACK_UB2(end_header, 8, number_of_entries);
   /* total number of entries in central dir*/
   PACK_UB2(end_header, 10, number_of_entries);
   /* size of central dir. */
   PACK_UB4(end_header, 12, dir_size);
   /* offset of start of central dir */
   PACK_UB4(end_header, 16, start_offset);
   /* zipfile comment length */
   end_header[20] = 0;
   end_header[21] = 0;

   (void) write(fd, end_header, 22);

   U_INTERNAL_TRACE("Total:\n------\n(in = %d) (out = %d) (%s %d%%)", total_in, total_out,
                    "deflated", (int)(total_in ? ((1 - (total_out / (float)total_in)) * 100) : 0))

   return 0;
}

static inline void add_entry(struct zipentry* ze)
{
   U_INTERNAL_TRACE("add_entry(%p)", ze)

   if (ziplist == 0)
      {
      ziplist = ze;
      ziptail = ziplist;
      }
   else
      {
      ziplist->next_entry = ze;
      ziplist             = ze;
      }

   ++number_of_entries;
}

static int add_file_to_zip(int jfd, int ffd, const char* fname, struct stat* statbuf)
{
   int rdamt;
   off_t offset = 0;
   ub1 rd_buff[RDSZ];
   struct zipentry* ze;
   unsigned long mod_time;
   unsigned short file_name_length;

   U_INTERNAL_TRACE("add_file_to_zip(%d,%d,%s,%p)", jfd, ffd, fname, statbuf)

   mod_time         = unix2dostime(&(statbuf->st_mtime));
   file_name_length = u__strlen(fname, __PRETTY_FUNCTION__);

   ze = (zipentry*) calloc(1, sizeof(zipentry)); /* clear all the fields */

   if (ze == 0)
      {
      perror("malloc");

      return 1;
      }

   ze->filename = (char*) malloc((file_name_length + 1) * sizeof(char));

   u__strcpy(ze->filename, fname);

   ze->csize    = statbuf->st_size;
   ze->usize    = ze->csize;
   ze->offset   = lseek(jfd, 0, SEEK_CUR);
   ze->mod_time = (ub2) (mod_time & 0x0000ffff);
   ze->mod_date = (ub2)((mod_time & 0xffff0000) >> 16);

   /**
    * If a MIME type for a document that makes use of packages is existing, then the package should
    * contain a stream called "mimetype". This stream should be first stream of the package's zip file, it
    * shall not be compressed, and it shall not use an 'extra field' in its header.
    * The purpose is to allow packaged files to be identified through 'magic number' mechanisms, such
    * as Unix's file/magic utility. If a ZIP file contains a stream at the beginning of the file that is
    * uncompressed, and has no extra data in the header, then the stream name and the stream
    * content can be found at fixed positions. More specifically, one will find:
    *
    *  - a string 'PK' at position 0 of all zip files
    *  - a string 'mimetype' at position 30 of all such package files
    *  - the mimetype itself at position 38 of such a package
    */

   ze->compressed = (strcmp(fname, "mimetype") != 0 && statbuf->st_size > 0);

   add_entry(ze);

   /* data descriptor */
   PACK_UB2(file_header, LOC_EXTRA, 0);

   file_header[LOC_COMP]   = (ze->compressed ? 8 : 0);
   file_header[LOC_COMP+1] = 0;

   PACK_UB4(file_header, LOC_MODTIME, mod_time);
   PACK_UB2(file_header, LOC_FNLEN, file_name_length);

   (void) memset((file_header + LOC_CRC), '\0', 12); /* clear crc/usize/csize */

   /* Write the local header */
   (void) write(jfd, file_header, 30);

   /* write the file name to the zip file */
   (void) write(jfd, fname, file_name_length);

   U_INTERNAL_TRACE("adding: %s", fname)

   /* compress the file */
   compress_file(ffd, jfd, ze);

   while ((rdamt = read(ffd, rd_buff, RDSZ)) > 0)
      {
      ze->crc = crc32(ze->crc, rd_buff, rdamt);

      if (write(jfd, rd_buff, rdamt) != rdamt)
         {
         perror("write");

         return 1;
         }
      }

   (void) close(ffd);

   /* write out data descriptor */
   PACK_UB4(data_descriptor,  4, ze->crc);
   PACK_UB4(data_descriptor,  8, ze->csize);
   PACK_UB4(data_descriptor, 12, ze->usize);

   /* we need to seek back and fill the header */
   offset = (ze->csize + u__strlen(ze->filename, __PRETTY_FUNCTION__) + 16);

   if (lseek(jfd, -offset, SEEK_CUR) == (off_t)-1)
      {
      perror("lseek");

      return 1;
      }

   if (write(jfd, (data_descriptor + 4), 12) != 12)
      {
      perror("write");

      return 1;
      }

   offset -= 12;

   if (lseek(jfd, offset, SEEK_CUR) == (off_t)-1)
      {
      perror("lseek");

      return 1;
      }

   U_INTERNAL_TRACE("(in=%d) (out=%d) (%s %d%%)", (int)ze->usize, (int)ze->csize,
                    "deflated", (int)(ze->usize ? ((1 - ze->csize/(float)ze->usize) * 100) : 0))

   return 0;
}

static int add_to_zip(int fd, const char* file)
{
   DIR* dir = 0;
   int result = 0;
   struct stat statbuf;

   U_INTERNAL_TRACE("add_to_zip(%d,%s)", fd, file)

   while (*file     == '.' &&
          *(file+1) == '/')
      {
      file += 2;
      }

#ifndef U_COVERITY_FALSE_POSITIVE
   /* coverity[toctou] */
   if (stat(file, &statbuf) == -1)
      {
      perror(file);

      return 1;
      }

   if (S_ISDIR(statbuf.st_mode))
      {
      int nlen;
      char* t_ptr;
      zipentry* ze;
      char* fullname;
      struct dirent* de;
      unsigned d_namlen;
      unsigned long mod_time;

      dir = opendir(file);

      if (dir == 0)
         {
         perror("opendir");

         return 1;
         }

      nlen = u__strlen(file, __PRETTY_FUNCTION__) + 256;

      fullname = (char*) calloc(1, nlen);

      u__strcpy(fullname, file);

      nlen = u__strlen(file, __PRETTY_FUNCTION__);

      if (fullname[nlen - 1] != '/')
         {
         fullname[nlen] = '/';

         t_ptr = (fullname + nlen + 1);
         }
      else
         {
         t_ptr = (fullname + nlen);
         }

      (void) memset((file_header + 12), '\0', 16); /* clear mod time, crc, size fields */

      nlen = (t_ptr - fullname);

      mod_time = unix2dostime(&statbuf.st_mtime);

      PACK_UB2(file_header, LOC_EXTRA,   0);
      PACK_UB2(file_header, LOC_COMP,    0);
      PACK_UB2(file_header, LOC_FNLEN,   nlen);
      PACK_UB4(file_header, LOC_MODTIME, mod_time);

      U_INTERNAL_TRACE("adding: %s (in=0) (out=0) (stored 0%%)", fullname)

      ze = (zipentry*) calloc(1, sizeof(zipentry)); /* clear all the fields */

      if (ze == 0)
         {
         perror("malloc");

         result = 1;

         free(fullname);

         goto end;
         }

                ze->filename = (char*) malloc((nlen + 1) * sizeof(char) + 1);
      u__strcpy(ze->filename, fullname);
                ze->filename[nlen] = '\0';

      ze->offset     = lseek(fd, 0, SEEK_CUR);
      ze->mod_time   = (ub2) (mod_time & 0x0000ffff);
      ze->mod_date   = (ub2)((mod_time & 0xffff0000) >> 16);
      ze->compressed = 0;

      add_entry(ze);

      (void) write(fd, file_header, 30);
      (void) write(fd, fullname,    nlen);

      while ((de = readdir(dir)) != 0)
         {
         if (U_ISDOTS(de->d_name)) continue;

         d_namlen = NAMLEN(de);

         (void) u__strncpy(t_ptr, de->d_name, d_namlen);

         t_ptr[d_namlen] = '\0';

         if (add_to_zip(fd, fullname))
            {
            U_INTERNAL_TRACE("Error adding file to zip")

            result = 1;

            free(fullname);

            goto end;
            }
         }

      free(fullname);
      }
   else if (S_ISREG(statbuf.st_mode))
      {
      int add_fd = open(file, O_RDONLY | O_BINARY);

      if (add_fd < 0)
         {
         U_INTERNAL_TRACE("Error opening %s", file)

         return 1;
         }

      if (add_file_to_zip(fd, add_fd, file, &statbuf))
         {
         U_INTERNAL_TRACE("Error adding file to zip")

         return 1;
         }
      }
   else
      {
      U_INTERNAL_TRACE("Illegal file specified: %s", file)

      return 0;
      }

end:
   if (dir) (void) closedir(dir);
#endif

   return result;
}

static int zipfd;
static ub1* filename;
static int filename_len;

static void zip_close(void)
{
   U_INTERNAL_TRACE("zip_close()")

   if (filename != 0)
      {
      free(filename);

      filename     = 0;
      filename_len = 0;
      }

   if (zipfd == -1 ||
       close(zipfd) != 0)
      {
      U_INTERNAL_TRACE("Error closing zip archive")
      }
}

/* create new archive */

int zip_archive(const char* zipfile, const char* files[])
{
   int i;

   U_INTERNAL_TRACE("zip_archive(%s,%p)", zipfile, files)

   ziplist = 0;
   number_of_entries = 0;

   /* create the zipfile */

   zipfd = open(zipfile, O_CREAT | O_WRONLY | O_TRUNC | O_BINARY, 0666);

   if (zipfd < 0)
      {
      U_INTERNAL_TRACE("Error opening %s for writing", zipfile)

      perror(zipfile);

      return 1;
      }

   init_headers();

   if (init_compression()) return 1;

   for (i = 0; files[i] != 0; ++i)
      {
      if (strcmp(files[i], zipfile) == 0)
         {
         U_INTERNAL_TRACE("skipping: %s", files[i])

         continue;  /* we don't want to add ourselves.. */
         }

      if (add_to_zip(zipfd, files[i]))
         {
         U_INTERNAL_TRACE("Error adding %s to zip archive", files[i])

         return 1;
         }
      }

   /* de-initialize the compression DS */

   end_compression();

   create_central_header(zipfd);

   zip_close();

   return 0;
}

static pb_file pbf;

static void consume(int amt)
{
   ub1 buff[RDSZ];

   U_INTERNAL_TRACE("consume(%d)", amt)

   if (amt <= (int)pbf.buff_amt)
      {
      U_INTERNAL_TRACE("Consuming %d bytes", amt)

      if (pbf.fd == -1)
         {
         /* update the buff_amt field */

         pbf.next     += amt;
         pbf.buff_amt -= amt;
         }
      else
         {
         pb_read(&pbf, buff, amt);
         }
      }
   else
      {
      U_INTERNAL_ASSERT(pbf.fd != -1)

      (void) lseek(pbf.fd, amt - pbf.buff_amt, SEEK_CUR);

      pb_read(&pbf, buff, pbf.buff_amt); /* clear pbf */
      }

   U_INTERNAL_TRACE("%d bytes consumed", amt)
}

/* open the zipfile */

static int zip_open(const char* zipfile)
{
   U_INTERNAL_TRACE("zip_open(%s)", zipfile)

   zipfd = open(zipfile, O_RDONLY | O_BINARY);

   if (zipfd < 0)
      {
      U_INTERNAL_TRACE("Error opening %s for reading", zipfile)

      perror(zipfile);

      return 1;
      }

   if (init_inflation()) return 1;

   pb_init(&pbf, zipfd, 0);

   return 0;
}

static int rdamt;
static zipentry ze;
static ub1 scratch[16];
static ub2 fnlen, eflen, flags, method;
static ub4 signature, usize, csize, crc;

static int zip_read_entry(void)
{
   U_INTERNAL_TRACE("zip_read_entry()")

   if ((rdamt = pb_read(&pbf, scratch, 4)) != 4)
      {
      perror("read");

      return 1;
      }

   signature = UNPACK_UB4(scratch, 0);

   U_INTERNAL_TRACE("signature is %x", signature)

   if (signature == 0x08074b50)
      {
      U_INTERNAL_TRACE("skipping data_descriptor")

      pb_read(&pbf, scratch, 12);

      return 0;
      }

   if (signature == 0x02014b50)
      {
      U_INTERNAL_TRACE("Central header reached... we're all done")

      return 1;
      }

   if (signature != 0x04034b50)
      {
      U_INTERNAL_TRACE("Ick! %#x", signature)

      return 1;
      }

   if ((rdamt = pb_read(&pbf, (file_header + 4), 26)) != 26)
      {
      perror("read");

      return 1;
      }

   csize = UNPACK_UB4(file_header, LOC_CSIZE);

   U_INTERNAL_TRACE("Compressed size is %u", csize)

   fnlen = UNPACK_UB2(file_header, LOC_FNLEN);

   U_INTERNAL_TRACE("Filename length is %hu", fnlen)

   eflen = UNPACK_UB2(file_header, LOC_EFLEN);

   U_INTERNAL_TRACE("Extra field length is %hu", eflen)

   flags = UNPACK_UB2(file_header, LOC_EXTRA);

   U_INTERNAL_TRACE("Flags are %#hx", flags)

   method = UNPACK_UB2(file_header, LOC_COMP);

   U_INTERNAL_TRACE("Compression method is %#hx", method)

   usize = UNPACK_UB4(file_header, LOC_USIZE);

   U_INTERNAL_TRACE("Uncompressed size is %u", usize)

   if ((flags & 0x0008) == 0) /* if there isn't a data descriptor */
      {
      crc = UNPACK_UB4(file_header, LOC_CRC);

      U_INTERNAL_TRACE("CRC is %x", crc)
      }

   if (filename_len < (fnlen + 1))
      {
      if (filename != 0) free(filename);

      filename = (ub1*) malloc(sizeof(ub1) * (fnlen + 1));

      filename_len = fnlen + 1;
      }

   pb_read(&pbf, filename, fnlen);

   filename[fnlen] = '\0';

   U_INTERNAL_TRACE("filename is %s", filename)

   return 0;
}

int zip_match(const char* zipfile, const char* files[])
{
   int i, j, match, file_num = 0;

   U_INTERNAL_TRACE("zip_match(%s,%p)", zipfile, files)

   if (zip_open(zipfile)) return 0;

   eflen = 0;
   csize = 0;

   while (files[file_num] != 0) ++file_num;

   for (i = 0; zip_read_entry() == 0; ++i)
      {
      if (filename[fnlen - 1] != '/') /* directory */
         {
         match = 0;

         for (j = 0; j < file_num; ++j)
            {
            if (strcmp(files[j], (const char*)filename) == 0)
               {
               match = 1;

               break;
               }
            }

         if (match == 0) break;
         }

      if (eflen) consume(eflen);
      if (csize) consume(csize);
      /**
       * the header is at the end. In a ZIP file, this means that the data
       * happens to be compressed. We have no choice but to inflate the data
      else
         {
         inflate_file(&pbf, -1, &ze);
         }
      */
      }

   zip_close();

   return (i == file_num);
}

/* extract named (or all) files from archive */

unsigned zip_extract(const char* zipfile, const char** files, char*** filenames, unsigned** filenames_len)
{
   unsigned n = 0;
   char* names[1024];
   int j, file_num = 0;
   unsigned names_len[1024];

   U_INTERNAL_TRACE("zip_extract(%s,%p,%p,%p)", zipfile, files, filenames, filenames_len)

   if (zip_open(zipfile)) return 0; /* open the zipfile */

   if (files) while (files[file_num] != 0) ++file_num;

   (void) memset(names,     '\0', sizeof(names));
   (void) memset(names_len, '\0', sizeof(names_len));

   for (;;)
      {
      int f_fd   = 0,
          handle = 1; /* by default we'll extract/create the file */

      U_INTERNAL_ASSERT(n < 1024)

         crc = 0;
      ze.crc = 0;

      if (zip_read_entry()) break;

      if (file_num)
         {
         handle = 0;

         for (j = 0; j < file_num; ++j)
            {
            if (strcmp(files[j], (const char*)filename) == 0)
               {
               handle = 1;

               break;
               }
            }
         }

      if (handle)
         {
         names[n]     = strdup((const char*)filename);
         names_len[n] = fnlen;
         }
      else
         {
         f_fd = -1;
         }

      /**
       * OK, there is some directory information in the file. Nothing to do
       * but ensure the directory(s) exist, and create them if they don't.
       * What a pain!
       */

      if (strchr((const char*)filename, '/') != 0 && handle)
         {
         /* Loop through all the directories in the path, (everything w/ a '/') */

         int len;
         const ub1* idx;
         struct stat sbuf;
         const ub1* start = filename;
         char* tmp_buff = (char*) malloc(fnlen);

         for (;;)
            {
            idx = (const unsigned char*)strchr((const char*)start, '/');

            if (idx == 0) break;

            if (idx == start)
               {
               start++;

               continue;
               }

            start = idx + 1;

            len = idx-filename;

            (void) u__strncpy(tmp_buff, (const char*)filename, len);

            tmp_buff[len] = '\0';

            U_INTERNAL_TRACE("Making directory...")

            if (mkdir(tmp_buff, 0755) < 0)
               {
               if (errno != EEXIST)
                  {
                  perror("mkdir");

                  free(tmp_buff);

                  return 0;
                  }

#           ifndef U_COVERITY_FALSE_POSITIVE
               /* coverity[toctou] */
               if (stat(tmp_buff, &sbuf) < 0)
                  {
                  if (errno != ENOENT)
                     {
                     perror("stat");

                     free(tmp_buff);

                     return 0;
                     }
                  }
               else if (S_ISDIR(sbuf.st_mode))
                  {
                  U_INTERNAL_TRACE("Directory exists")

                  continue;
                  }
               else
                  {
                  U_INTERNAL_TRACE("Hmmm.. %s exists but isn't a directory!", tmp_buff)

                  free(tmp_buff);

                  return 0;
                  }
#           endif
               }
            }

         /* only a directory */

         U_INTERNAL_TRACE("Leftovers are \"%s\" (%d)", start, u__strlen((const char*)start, __PRETTY_FUNCTION__))

         /* If the entry was just a directory, don't write to file, etc */

         if (u__strlen((const char*)start, __PRETTY_FUNCTION__) == 0) f_fd = -1;

         free(tmp_buff);
         }

      if (method != 8 &&
          (flags & 0x0008))
         {
         U_WARNING("Not compressed but data_descriptor - filename: %s", filename);

         return 0;
         }

      if (handle &&
          f_fd != -1)
         {
         f_fd = open((const char*)filename, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0644);

         if (f_fd < 0)
            {
            U_ERROR("Error extracting ZIP archive - filename: %s", filename);

         // return 0;
            }
         }

      if (method == 8 ||
          (flags & 0x0008))
         {
         if (eflen) consume(eflen);

         inflate_file(&pbf, f_fd, &ze);
         }
      else
         {
         int out_a, in_a;
         ub4 rd_buff[RDSZ];

         U_INTERNAL_TRACE("writing stored data... (%d bytes)", csize)

         out_a = 0;
          in_a = csize;

         ze.crc = crc32(ze.crc, 0, 0); /* initialize the crc */

         while (out_a < (int)csize)
            {
            rdamt = (in_a > RDSZ ? RDSZ : in_a);

            if (pb_read(&pbf, rd_buff, rdamt) != rdamt)
               {
               perror("read");

               (void) close(f_fd);

               return 0;
               }

            ze.crc = crc32(ze.crc, (Bytef*)rd_buff, rdamt);

            (void) write(f_fd, rd_buff, rdamt);

            out_a += rdamt;
            in_a  -= rdamt;

            U_INTERNAL_TRACE("%d bytes written", out_a)
            }

         if (eflen) consume(eflen);
         }

      (void) close(f_fd);

      /* if there is a data descriptor left, compare the CRC */

      if (flags & 0x0008)
         {
         if (pb_read(&pbf, scratch, 16) != 16)
            {
            perror("read");

            return 0;
            }

         signature = UNPACK_UB4(scratch, 0);

         if (signature != 0x08074b50)
            {
            U_WARNING("Missing data_descriptor - filename: %s", filename);

            return 0;
            }

         crc = UNPACK_UB4(scratch, 4);
         }

      if (crc != ze.crc)
         {
         U_WARNING("CRCs do not match, got %x, expected %x - filename: %s", ze.crc, crc, filename);

      // return 0;
         }

      ++n;
      }

   zip_close();

   if (n > 0)
      {
      char** vec1    = *filenames     = (char**)    malloc(n * sizeof(char*));
      unsigned* vec2 = *filenames_len = (unsigned*) malloc(n * sizeof(unsigned));

      for (j = 0; j < (int)n; ++j)
         {
         vec1[j] = names[j];
         vec2[j] = names_len[j];
         }
      }
   else
      {
      *filenames = 0;
      *filenames_len = 0;
      }

   return n;
}

/* get contents files from archive data */

unsigned zip_get_content(const char* zipdata, unsigned datalen, char*** filenames,    unsigned** filenames_len,
                                                                char*** filecontents, unsigned** filecontents_len)
{
   unsigned n = 0;
   char* names[1024];
   char* contents[1024];
   unsigned names_len[1024];
   unsigned contents_len[1024];

   U_INTERNAL_TRACE("zip_get_content(%p,%u,%p,%p,%p,%p)", zipdata, datalen, filenames, filenames_len, filecontents, filecontents_len)

   if (init_inflation()) return 0;

   zipfd = -1;

   pb_init(&pbf, datalen, (ub1*)zipdata);

   for (;;)
      {
      U_INTERNAL_ASSERT(n < 1024)

         crc = 0;
      ze.crc = 0;

      if (zip_read_entry()) break;

      names[n]        = strdup((const char*)filename);
      names_len[n]    = fnlen;
      contents[n]     = 0;
      contents_len[n] = 0;

      if (method != 8 &&
          (flags & 0x0008))
         {
         U_INTERNAL_TRACE("Error: not compressed but data_descriptor")

         return 0;
         }

      if (eflen) consume(eflen);

      if (method == 8 ||
          (flags & 0x0008))
         {
         contents[n] = (usize ? (char*)malloc(usize) : 0);

         (void) inflate_buffer(&pbf, &csize, &(contents[n]), &usize, &ze);

         U_INTERNAL_TRACE("done inflating - ze.usize = %d usize = %d", ze.usize, usize)

         U_INTERNAL_ASSERT(ze.usize == usize)

         contents_len[n] = usize;

         consume(csize);
         }
      else if (csize)
         {
         U_INTERNAL_TRACE("writing stored data... (%d bytes)", csize)

         contents[n]     = (char*) malloc(csize);
         contents_len[n] = csize;

         ze.crc = crc32(ze.crc, 0, 0); /* initialize the crc */
         ze.crc = crc32(ze.crc, (Bytef*)pbf.next, csize);

         u__memcpy(contents[n], pbf.next, csize, __PRETTY_FUNCTION__);

         consume(csize);
         }

      /* if there is a data descriptor left, compare the CRC */

      if (flags & 0x0008)
         {
         if (pb_read(&pbf, scratch, 16) != 16)
            {
            perror("read");

            return 0;
            }

         signature = UNPACK_UB4(scratch, 0);

         if (signature != 0x08074b50)
            {
            U_INTERNAL_TRACE("Error: missing data_descriptor")

            return 0;
            }

         crc = UNPACK_UB4(scratch, 4);
         }

      if (crc != ze.crc)
         {
         U_INTERNAL_TRACE("Error: CRCs do not match, got %x, expected %x", ze.crc, crc)

         return 0;
         }

      ++n;
      }

   zip_close();

   if (n > 0)
      {
      unsigned j;
      char**    vec1 = *filenames        = (char**)    malloc(n * sizeof(char*));
      char**    vec2 = *filecontents     = (char**)    malloc(n * sizeof(char*));
      unsigned* vec3 = *filenames_len    = (unsigned*) malloc(n * sizeof(unsigned));
      unsigned* vec4 = *filecontents_len = (unsigned*) malloc(n * sizeof(unsigned));

      for (j = 0; j < n; ++j)
         {
         vec1[j] =        names[j];
         vec2[j] =     contents[j];
         vec3[j] =    names_len[j];
         vec4[j] = contents_len[j];
         }
      }
   else
      {
      *filenames        = 0;
      *filecontents     = 0;
      *filenames_len    = 0;
      *filecontents_len = 0;
      }

   return n;
}
