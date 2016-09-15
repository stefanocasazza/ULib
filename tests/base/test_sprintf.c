/* test_sprintf.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ulib/base/base.h>

#ifndef __MINGW32__
#  include <sys/uio.h>
#endif

static char ubuffer[4096];
static struct iovec iov[] = { { 0, 0 }, { (caddr_t)"\n", 1 } };

#define  puts(s)   (iov[0].iov_base = (caddr_t)s, iov[0].iov_len = strlen(s), writev(STDOUT_FILENO,iov,2))
#define fputs(s,o)          write(STDOUT_FILENO,s,strlen(s))
#define printf(fmt,args...) write(STDOUT_FILENO,ubuffer,u__snprintf(ubuffer,sizeof(ubuffer),fmt,strlen(fmt) , ##args))

#include <float.h>
#include <limits.h>

static int result = 0;

static void rfg1(void);
static void rfg2(void);

static void fmtchk(const char* fmt)
{
  (void) fputs(fmt, stdout);
  (void) printf(":\t`",0);
  (void) printf(fmt, 0x12);
  (void) printf("'\n",0);
}

static void fmtst1chk(const char* fmt)
{
  (void) fputs(fmt, stdout);
  (void) printf(":\t`",0);
  (void) printf(fmt, 4, 0x12);
  (void) printf("'\n",0);
}

static void fmtst2chk(const char* fmt)
{
  (void) fputs(fmt, stdout);
  (void) printf(":\t`",0);
  (void) printf(fmt, 4, 4, 0x12);
  (void) printf("'\n",0);
}

/*
 * Extracted from exercise.c for glibc-1.05 bug report by Bruce Evans.
 */

#define DEC -123
#define INT 255
#define UNS (~0)

/* Formatted Output Test
 *
 * This exercises the output formatting code.
 */

static void fp_test(void)
{
   int i, j, k, l;
   char buf[7];
   char *prefix = buf;
   char tp[20];

   puts("\nFormatted output test");
   printf("prefix  6d      6o      6x      6X      6u\n",0);

   strcpy(prefix, "%");
   for (i = 0; i < 2; i++)
      {
      for (j = 0; j < 2; j++)
         {
         for (k = 0; k < 2; k++)
            {
            for (l = 0; l < 2; l++)
               {
               strcpy(prefix, "%");
               if (i == 0) strcat(prefix, "-");
               if (j == 0) strcat(prefix, "+");
               if (k == 0) strcat(prefix, "#");
               if (l == 0) strcat(prefix, "0");
               printf("%5s |", prefix);
               strcpy(tp, prefix);
               strcat(tp, "6d |");
               printf(tp, DEC);
               strcpy(tp, prefix);
               strcat(tp, "6o |");
               printf(tp, INT);
               strcpy(tp, prefix);
               strcat(tp, "6x |");
               printf(tp, INT);
               strcpy(tp, prefix);
               strcat(tp, "6X |");
               printf(tp, INT);
               strcpy(tp, prefix);
               strcat(tp, "6u |");
               printf(tp, UNS);
               printf("\n",0);
               }
            }
         }
      }

   printf("%10s\n", (char*) NULL);
   printf("%-10s\n", (char*) NULL);
}

static void test1(void)
{
   double d = FLT_MIN;
   int niter = 17;

   while (niter-- != 0) printf("%.17e\n", d / 2);
}

static void test2(void)
{
   char buf[50];
   char buf2[1024];
   size_t n = u__snprintf(buf, sizeof (buf), U_CONSTANT_TO_PARAM("%30s"), "foo");

   printf("snprintf (\"%%30s\", \"foo\") == %d, \"%.*s\"\n", n, n, buf);

   n = u__snprintf(buf2, sizeof(buf2) - 10, U_CONSTANT_TO_PARAM("%.999u"), 10);

   printf("snprintf (\"%%.999u\", 10) = %d\n", n);
}

static void test3(void)
{
   char buf[200];
   const char* tmp;

   u__snprintf(buf,sizeof(buf),U_CONSTANT_TO_PARAM("%*s%*s%*s"),-1,"one",-20,"two",-30,"three");
   result |= strcmp(buf, "onetwo                 three                         ");
   tmp = (result != 0 ? "Test failed!" : "Test ok.");
   puts (tmp);

   u__snprintf(buf,sizeof(buf), U_CONSTANT_TO_PARAM("%07llo"), 040000000000ll);
   printf("sprintf (buf, \"%%07llo\", 040000000000ll) = %s", buf);

   if (strcmp(buf, "40000000000") != 0)
      {
      result = 1;
      fputs("\tFAILED", stdout);
      }

   puts("");
}

static void test4(void)
{
/*
#if !defined(SOLARIS) && !defined(MACOSX)
   char bytes[7];
   char buf[20];

   memset (bytes, '\xff', sizeof bytes);
   u__snprintf(buf,sizeof(buf), U_CONSTANT_TO_PARAM("foo%hn\n"), &bytes[3]);

   if (bytes[0] != '\xff' || bytes[1] != '\xff' || bytes[2] != '\xff' ||
       bytes[5] != '\xff' || bytes[6] != '\xff')
      {
      puts ("%hn overwrite more bytes");
      result = 1;
      }

   if (bytes[3] != 3)
      {
      puts ("%hn wrote incorrect value");
      result = 1;
      }
#endif
*/
}

int main(int argc, char* argv[])
{
   static char shortstr[] = "Hi, Z.";
   static char longstr[] = "Good morning, Doctor Chandra.  This is Hal.  \
   I am ready for my first lesson today.";

   u_init_ulib(argv);

   U_INTERNAL_TRACE("main(%d,%p)", argc, argv)

   /*
   char buffer[11];
   uint32_t written = u__snprintf(buffer, 10, U_CONSTANT_TO_PARAM("%ld"), 3821975508); // u_now->tv_sec

   U_INTERNAL_ASSERT_EQUALS(written, 10)
   U_INTERNAL_ASSERT_EQUALS(strncmp(buffer, "3821975508", 10), 0)
   */

   fmtchk("%.4x");
   fmtchk("%04x");
   fmtchk("%4.4x");
   fmtchk("%04.4x");
   fmtchk("%4.3x");
   fmtchk("%04.3x");

   fmtst1chk("%.*x");
   fmtst1chk("%0*x");
   fmtst2chk("%*.*x");
   fmtst2chk("%0*.*x");

   printf("bad format:\t\"%K\"\n",0);
   printf("nil pointer (padded):\t\"%10p\"\n", (void *) NULL);

   printf("decimal negative:\t\"%d\"\n", -2345);
   printf("octal negative:\t\"%o\"\n", -2345);
   printf("hex negative:\t\"%x\"\n", -2345);
   printf("long decimal number:\t\"%ld\"\n", -123456L);
// printf("long octal negative:\t\"%lo\"\n", -2345L);
// printf("long unsigned decimal number:\t\"%lu\"\n", -123456L);
   printf("zero-padded LDN:\t\"%010ld\"\n", -123456L);
// printf("left-adjusted ZLDN:\t\"%-010ld\"\n", -123456);
   printf("space-padded LDN:\t\"%10ld\"\n", -123456L);
   printf("left-adjusted SLDN:\t\"%-10ld\"\n", -123456L);

   printf("zero-padded string:\t\"%010s\"\n", shortstr);
   printf("left-adjusted Z string:\t\"%-010s\"\n", shortstr);
   printf("space-padded string:\t\"%10s\"\n", shortstr);
   printf("left-adjusted S string:\t\"%-10s\"\n", shortstr);
   printf("null string:\t\"%s\"\n", (char *)NULL);
   printf("limited string:\t\"%.22s\"\n", longstr);

   printf("e-style >= 1:\t\"%e\"\n", 12.34);
   printf("e-style >= .1:\t\"%e\"\n", 0.1234);
   printf("e-style < .1:\t\"%e\"\n", 0.001234);
   printf("e-style big:\t\"%.60e\"\n", 1e20);
   printf("e-style == .1:\t\"%e\"\n", 0.1);
   printf("f-style >= 1:\t\"%f\"\n", 12.34);
   printf("f-style >= .1:\t\"%f\"\n", 0.1234);
   printf("f-style < .1:\t\"%f\"\n", 0.001234);
   printf("g-style >= 1:\t\"%g\"\n", 12.34);
   printf("g-style >= .1:\t\"%g\"\n", 0.1234);
   printf("g-style < .1:\t\"%g\"\n", 0.001234);
   printf("g-style big:\t\"%.60g\"\n", 1e20);

   printf(" %6.5f\n", .099999999860301614);
   printf(" %6.5f\n", .1);
   printf("x%5.4fx\n", .5);

   printf("%#03x\n", 1);

   printf("something really insane: %.100f\n", 1.0);

   test1();

   printf("%15.5e\n", 4.9406564584124654e-324);

#define FORMAT "| %12.4f | %12.4e | %12.4g |\n"
   printf(FORMAT, 0.0, 0.0, 0.0);
   printf(FORMAT, 1.0, 1.0, 1.0);
   printf(FORMAT, -1.0, -1.0, -1.0);
   printf(FORMAT, 100.0, 100.0, 100.0);
   printf(FORMAT, 1000.0, 1000.0, 1000.0);
   printf(FORMAT, 10000.0, 10000.0, 10000.0);
   printf(FORMAT, 12345.0, 12345.0, 12345.0);
   printf(FORMAT, 100000.0, 100000.0, 100000.0);
   printf(FORMAT, 123456.0, 123456.0, 123456.0);
#undef FORMAT

   test2();

   fp_test();

// printf("%e should be 1.234568e+06\n", 1234567.8);
   printf("%f should be 1234567.800000\n", 1234567.8);
// printf("%g should be 1.23457e+06\n", 1234567.8);
   printf("%g should be 123.456\n", 123.456);
// printf("%g should be 1e+06\n", 1000000.0);
   printf("%g should be 10\n", 10.0);
   printf("%g should be 0.02\n", 0.02);

   test3();

   printf("printf (\"%%c\", %u) = %c\n", UCHAR_MAX + 2, UCHAR_MAX + 2);
   printf("printf (\"%%hu\", %u) = %hu\n", USHRT_MAX + 2, USHRT_MAX + 2);

   puts("--- Should be no further output. ---");

   rfg1();
   rfg2();

   test4();

   return (result != 0);
}

static void rfg1(void)
{
   char buf[100];

   u__snprintf(buf,sizeof(buf), U_CONSTANT_TO_PARAM("%5.s"), "xyz");
   if (strcmp (buf, "     ") != 0) printf ("got: '%s', expected: '%s'\n", buf, "     ");
   u__snprintf(buf,sizeof(buf), U_CONSTANT_TO_PARAM("%5.f"), 33.3);
   if (strcmp (buf, "   33") != 0) printf ("got: '%s', expected: '%s'\n", buf, "   33");
// u__snprintf(buf,sizeof(buf), U_CONSTANT_TO_PARAM("%8.e"), 33.3e7);
// if (strcmp (buf, "   3e+08") != 0) printf ("got: '%s', expected: '%s'\n", buf, "   3e+08");
// u__snprintf(buf,sizeof(buf), U_CONSTANT_TO_PARAM("%8.E"), 33.3e7);
// if (strcmp (buf, "   3E+08") != 0) printf ("got: '%s', expected: '%s'\n", buf, "   3E+08");
// u__snprintf(buf,sizeof(buf), U_CONSTANT_TO_PARAM("%.g"), 33.3);
// if (strcmp (buf, "3e+01") != 0) printf ("got: '%s', expected: '%s'\n", buf, "3e+01");
// u__snprintf(buf,sizeof(buf), U_CONSTANT_TO_PARAM("%.G"), 33.3);
// if (strcmp (buf, "3E+01") != 0) printf ("got: '%s', expected: '%s'\n", buf, "3E+01");
}

static void rfg2(void)
{
   int prec;
   char buf[100];

   prec = 0;
   u__snprintf(buf,sizeof(buf), U_CONSTANT_TO_PARAM("%.*g"), prec, 3.3);
   if (strcmp (buf, "3") != 0)
      printf ("got: '%s', expected: '%s'\n", buf, "3");
   prec = 0;
   u__snprintf(buf,sizeof(buf), U_CONSTANT_TO_PARAM("%.*G"), prec, 3.3);
   if (strcmp (buf, "3") != 0)
      printf ("got: '%s', expected: '%s'\n", buf, "3");
   prec = 0;
   u__snprintf(buf,sizeof(buf), U_CONSTANT_TO_PARAM("%7.*G"), prec, 3.33);
   if (strcmp (buf, "      3") != 0)
      printf ("got: '%s', expected: '%s'\n", buf, "      3");
   prec = 3;
   u__snprintf(buf,sizeof(buf), U_CONSTANT_TO_PARAM("%04.*o"), prec, 33);
   if (strcmp (buf, " 041") != 0)
      printf ("got: '%s', expected: '%s'\n", buf, " 041");
   prec = 7;
   u__snprintf(buf,sizeof(buf), U_CONSTANT_TO_PARAM("%09.*u"), prec, 33);
   if (strcmp (buf, "  0000033") != 0)
      printf ("got: '%s', expected: '%s'\n", buf, "  0000033");
   prec = 3;
   u__snprintf(buf,sizeof(buf), U_CONSTANT_TO_PARAM("%04.*x"), prec, 33);
   if (strcmp (buf, " 021") != 0)
      printf ("got: '%s', expected: '%s'\n", buf, " 021");
   prec = 3;
   u__snprintf(buf,sizeof(buf), U_CONSTANT_TO_PARAM("%04.*X"), prec, 33);
   if (strcmp (buf, " 021") != 0)
      printf ("got: '%s', expected: '%s'\n", buf, " 021");
}
