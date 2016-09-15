/* test_misc.c */

#include <ulib/base/utility.h>
#include <ulib/base/coder/escape.h>
#include <ulib/base/replace/sysexits.h>

#include <errno.h>
#include <signal.h>

#ifdef __MINGW32__
#  include <process.h>
#else
#  include <sys/mman.h>
#  include <sysexits.h>
#endif

static char buf[4096], buffer[4096];

#ifdef DEBUG
static char bytes[] = { 'p', 'i', 'p', 'p', 'o', '\xff',
                        'p', 'i', 'p', 'p', 'o', '\xff',
                        'p', 'i', 'p', 'p', 'o', '\xff',
                        'p', 'i', 'p', 'p', 'o', '\xff' };
#endif

static int endiantest(void)
{
   unsigned char _bytes[] = { 0x01, 0x02, 0x03, 0x04 };

   return (*(unsigned int*)_bytes == 0x01020304 ?  0    // big    endian
                                                :  1);  // little endian
}

int main(int argc, char* argv[])
{
   int n_cmp;
   unsigned len;
   char l_err_buffer[256];

   u_init_ulib(argv);
   u_init_ulib_hostname();
   u_init_ulib_username();

   u_err_buffer = l_err_buffer;

#ifdef __MINGW32__
   n_cmp = 8;
#else
   n_cmp = 4;
#endif

   U_INTERNAL_ASSERT(argc >= 5)

   sprintf(buf, "%s %s", argv[3], argv[4]);
   u__snprintf(buffer, 4096, U_CONSTANT_TO_PARAM("%9D"), 0);
   if (strncmp(buf, buffer, strlen(buf) - n_cmp)) goto failed;

   sprintf(buf, "%s %s", argv[1], argv[2]);
   u__snprintf(buffer, 4096, U_CONSTANT_TO_PARAM("%U %H"), 0);
   if (strcasecmp(buf, buffer)) goto failed;

   sprintf(buf, "%s", u_basename(argv[0]));
   u__snprintf(buffer, 4096, U_CONSTANT_TO_PARAM("%N"), 0);
   if (strcmp(buf, buffer)) goto failed;

   sprintf(buf, "%d", getpid());
   u__snprintf(buffer, 4096, U_CONSTANT_TO_PARAM("%P"), 0);
   if (strcmp(buf, buffer)) goto failed;

   sprintf(buf, "%s", "true false");
   u__snprintf(buffer, 4096, U_CONSTANT_TO_PARAM("%b %b"), true, false);
   if (strcmp(buf, buffer)) goto failed;

   sprintf(buf, "%s", "-2,000,000 -200 2,000 2,000,000,000 -2,147,483,648");
   u__snprintf(buffer, 4096, U_CONSTANT_TO_PARAM("%'d %'d %'d %'d %'d"), -2000000, -200, 2000, 2000000000, INT_MIN);
   if (strcmp(buf, buffer)) goto failed;

#if __BYTE_ORDER == __LITTLE_ENDIAN
   sprintf(buf, "%s", "little endian");
#else
   sprintf(buf, "%s", "big endian");
#endif
   sprintf(buffer, "%s", (endiantest() ? "little endian" : "big endian"));
   if (strcmp(buf, buffer)) goto failed;

#ifdef DEBUG
   sprintf(buf, "%s", "0000000|70 69 70 70 6f ff 70 69:70 70 6f ff 70 69 70 70 |pippo.pippo.pipp\n"
                      "0000010|6f ff 70 69 70 70 6f ff:                        |o.pippo.        \n");
   u__snprintf(buffer, 4096, U_CONSTANT_TO_PARAM("%M"), bytes, sizeof(bytes));
   if (strcmp(buf, buffer)) goto failed;

#if __BYTE_ORDER == __LITTLE_ENDIAN
   sprintf(buf, "%s", "<11110001 10111010 00000000 00000000>");
#else
   sprintf(buf, "%s", "<10001111 01011101 00000000 00000000>");
#endif
   u__snprintf(buffer, 4096, U_CONSTANT_TO_PARAM("%B"), 23951);
   if (strcmp(buf, buffer)) goto failed;
#endif

#ifndef HAVE_STRSIGNAL
   sprintf(buf, "SIGILL (%d, Illegal instruction)", SIGILL);
   u__snprintf(buffer, 4096, U_CONSTANT_TO_PARAM("%Y"), SIGILL);
   if (strcmp(buf, buffer)) goto failed;

   sprintf(buf, "SIGSEGV (%d, Segmentation fault)", SIGSEGV);
   u__snprintf(buffer, 4096, U_CONSTANT_TO_PARAM("%Y"), SIGSEGV);
   if (strcmp(buf, buffer)) goto failed;

   sprintf(buf, "SIGHUP (%d, Hangup)", SIGHUP);
   u__snprintf(buffer, 4096, U_CONSTANT_TO_PARAM("%Y"), SIGHUP);
   if (strcmp(buf, buffer)) goto failed;
#endif

#ifndef HAVE_STRERROR
   sprintf(buf, "test - EFAULT (%d, Bad address)", EFAULT);
   errno = EFAULT;
   u__snprintf(buffer, 4096, U_CONSTANT_TO_PARAM("%R"), "test");
   if (strcmp(buf, buffer)) goto failed;

   sprintf(buf, "test - E2BIG (%d, Argument list too long)", E2BIG);
   errno = E2BIG;
   u__snprintf(buffer, 4096, U_CONSTANT_TO_PARAM("%R"), "test");
   if (strcmp(buf, buffer)) goto failed;
#endif

   sprintf(buf, "test - EX_PROTOCOL (%d, remote error in protocol)", EX_PROTOCOL);
   u__snprintf(buffer, 4096, U_CONSTANT_TO_PARAM("test - %r"), EX_PROTOCOL);
   if (strcmp(buf, buffer)) goto failed;

   len = u_escape_encode((unsigned char*)U_CONSTANT_TO_PARAM("stringa che continua 01234567890"), buf, 25);
   buf[len] = '\0';
   strcpy(buffer, "\"stringa che continua...\"");
   if (strcmp(buf, buffer)) goto failed;

   if (u_rmatch(U_CONSTANT_TO_PARAM("1234567890#Envelope"), U_CONSTANT_TO_PARAM("#Envelope")) == false) goto failed;

   if (u_runAsUser("mail", true))
      {
      strcpy(buf,    argv[5]); /* /var/mail, /var/spool/mail, /var/spool/clientmqueue, ... */
      strcpy(buffer, u_cwd);

      if (strcmp(buf, buffer) &&
          strcmp("/var/mail", buffer))
         {
         goto failed;
         }
      }

   errno = E2BIG;
   U_WARNING_SYSCALL("U_WARNING_SYSCALL() errno = E2BIG...");

   errno = EFAULT;
   U_ERROR_SYSCALL("U_ERROR_SYSCALL()   errno = EFAULT..");

   {
   int esito = 0;

   U_VAR_UNUSED(esito)

   U_INTERNAL_ASSERT(esito == 1)
   }

   errno = EACCES;
   U_ABORT_SYSCALL("U_ABORT_SYSCALL() errno = EACCES...");

   return 0;

failed:
   fprintf(stdout,"expect = %s\n", buf);
   fprintf(stdout,"found  = %s\n", buffer);

   return 1;
}
