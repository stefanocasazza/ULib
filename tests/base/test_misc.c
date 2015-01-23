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

static int display_char(char* output, int what)
{
   switch (what)
      {
      case '\r':  return sprintf(output, "\\r");
      case '\n':  return sprintf(output, "\\n");
      case '\t':  return sprintf(output, "\\t");
      case '\b':  return sprintf(output, "\\b");
      case '\f':  return sprintf(output, "\\f");
      case '\\':  return sprintf(output, "\\\\");
      case '"':   return sprintf(output, "\\\"");
      default:
         if ((what<32) || (what>126)) return sprintf(output, "\\%03o", (unsigned char)what);
         else                         return sprintf(output,     "%c",                what);
      }
}

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
   u__snprintf(buffer, 4096, "%9D", 0);
   if (strncmp(buf, buffer, strlen(buf) - n_cmp)) goto failed;

   sprintf(buf, "%s %s", argv[1], argv[2]);
   u__snprintf(buffer, 4096, "%U %H", 0);
   if (strcasecmp(buf, buffer)) goto failed;

   sprintf(buf, "%s", u_basename(argv[0]));
   u__snprintf(buffer, 4096, "%N", 0);
   if (strcmp(buf, buffer)) goto failed;

   sprintf(buf, "%d", getpid());
   u__snprintf(buffer, 4096, "%P", 0);
   if (strcmp(buf, buffer)) goto failed;

   sprintf(buf, "%s", "true false");
   u__snprintf(buffer, 4096, "%b %b", true, false);
   if (strcmp(buf, buffer)) goto failed;

   sprintf(buf, "%s", "-2,000,000 -200 2,000 2,000,000,000 -2,147,483,648");
   u__snprintf(buffer, 4096, "%'d %'d %'d %'d %'d", -2000000, -200, 2000, 2000000000, INT_MIN);
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
   u__snprintf(buffer, 4096, "%M", bytes, sizeof(bytes));
   if (strcmp(buf, buffer)) goto failed;

#if __BYTE_ORDER == __LITTLE_ENDIAN
   sprintf(buf, "%s", "<11110001 10111010 00000000 00000000>");
#else
   sprintf(buf, "%s", "<10001111 01011101 00000000 00000000>");
#endif
   u__snprintf(buffer, 4096, "%B", 23951);
   if (strcmp(buf, buffer)) goto failed;
#endif

#ifndef HAVE_STRSIGNAL
   sprintf(buf, "SIGILL (%d, Illegal instruction)", SIGILL);
   u__snprintf(buffer, 4096, "%Y", SIGILL);
   if (strcmp(buf, buffer)) goto failed;

   sprintf(buf, "SIGSEGV (%d, Segmentation fault)", SIGSEGV);
   u__snprintf(buffer, 4096, "%Y", SIGSEGV);
   if (strcmp(buf, buffer)) goto failed;

   sprintf(buf, "SIGHUP (%d, Hangup)", SIGHUP);
   u__snprintf(buffer, 4096, "%Y", SIGHUP);
   if (strcmp(buf, buffer)) goto failed;
#endif

#ifndef HAVE_STRERROR
   sprintf(buf, "test - EFAULT (%d, Bad address)", EFAULT);
   errno = EFAULT;
   u__snprintf(buffer, 4096, "%R", "test");
   if (strcmp(buf, buffer)) goto failed;

   sprintf(buf, "test - E2BIG (%d, Argument list too long)", E2BIG);
   errno = E2BIG;
   u__snprintf(buffer, 4096, "%R", "test");
   if (strcmp(buf, buffer)) goto failed;
#endif

   sprintf(buf, "test - EX_PROTOCOL (%d, remote error in protocol)", EX_PROTOCOL);
   u__snprintf(buffer, 4096, "test - %r", EX_PROTOCOL);
   if (strcmp(buf, buffer)) goto failed;

   len = u_escape_encode((unsigned char*)U_CONSTANT_TO_PARAM("stringa che continua 01234567890"), buf, 25, false);
   buf[len] = '\0';
   strcpy(buffer, "\"stringa che continua...\"");
   if (strcmp(buf, buffer)) goto failed;

   {
   int c;
   char* ptr = buf;

   for (c = 0; c < 256; ++c)
      {
      display_char(ptr, c);

      ptr += strlen(ptr);
      }

   ptr = buffer;

   for (c = 0; c < 256; ++c) ptr += u_sprintc(ptr, c);

   /* u_sprintc() = \\000\\001\\002\\003\\004\\005\\006\\007\\b\\t\\n\\013\\f\\r\\016\\017\\020\\021\\022\\023\\024\\025\\026\\027\\030\\031\\032\\033\\034\\035\\036\\037 !\"#$%%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\\177\\200\\201\\202\\203\\204\\205\\206\\207\\210\\211\\212\\213\\214\\215\\216\\217\\220\\221\\222\\223\\224\\225\\226\\227\\230\\231\\232\\233\\234\\235\\236\\237\\240\\241\\242\\243\\244\\245\\246\\247\\250\\251\\252\\253\\254\\255\\256\\257\\260\\261\\262\\263\\264\\265\\266\\267\\270\\271\\272\\273\\274\\275\\276\\277\\300\\301\\302\\303\\304\\305\\306\\307\\310\\311\\312\\313\\314\\315\\316\\317\\320\\321\\322\\323\\324\\325\\326\\327\\330\\331\\332\\333\\334\\335\\336\\337\\340\\341\\342\\343\\344\\345\\346\\347\\350\\351\\352\\353\\354\\355\\356\\357\\360\\361\\362\\363\\364\\365\\366\\367\\370\\371\\372\\373\\374\\375\\376\\377; */

   if (strcmp(buf, buffer)) goto failed;
   }

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
