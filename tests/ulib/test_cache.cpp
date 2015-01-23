// test_cache.cpp

#include <ulib/file.h>
#include <ulib/cache.h>

/*
// #define ORIGINAL
#define USIZE 10

static unsigned v[USIZE+1];
static unsigned writer = 1;
static unsigned oldest = USIZE;
static unsigned unused = USIZE;

void print()
{
   cout << "v[] = [ ";

   for (unsigned i = 0; i <= USIZE; ++i)
      {
      cout << v[i] << " ";
      }

   cout << "] " << endl;
}

unsigned get(unsigned pos)                 { return *(unsigned*)(v + pos); }
void     set(unsigned pos, unsigned value) {        *(unsigned*)(v + pos) = value; }

typedef struct cache_entry {
   unsigned link;
   char* str;
} cache_entry;

void read(unsigned hash, char* s)
{
   unsigned pos = get(hash);

   struct cache_entry* e;

#ifdef ORIGINAL
   unsigned prevpos = hash, nextpos;

   while (pos)
#else
   while (pos >= 1)
#endif
      {
#ifdef ORIGINAL
      cout << "prevpos = " << prevpos << endl;
#endif
      cout << "pos     = " <<     pos << endl;

      e = (struct cache_entry*)(v + pos);

      cout << "str = " << e->str << endl;

      if (e->str == s)
         {
         break;
         }

#ifdef ORIGINAL
      nextpos = prevpos ^ get(pos);
      prevpos = pos;
      pos     = nextpos;
#else
      pos     = get(pos);
#endif
      }
}

void write(unsigned hash, char* s)
{
   unsigned pos;

   while ((writer + 2) > oldest)
      {
      fprintf(stdout, "writer = %u, oldest = %u, unused = %u\n", writer, oldest, unused);

      if (oldest == unused)
         {
         if (writer == 1)
            {
            return;
            }

         unused = writer;
         oldest = writer = 1;
         }

      pos = get(oldest);

      cout << "v[oldest] = " << pos << endl;

#ifdef ORIGINAL
      set(pos, get(pos) ^ oldest);
#else
      set(pos, (pos >= 1 ? get(pos) : 0));
#endif

      oldest += 2;

      if (oldest == unused)
         {
         unused = oldest = USIZE;
         }
      }

   fprintf(stdout, "writer = %u, oldest = %u, unused = %u\n", writer, oldest, unused);

   pos = get(hash);

   cout << "pos = " << pos << endl;

   if (pos)
      {
      cout << "v[pos] = " << v[pos] << endl;
#ifdef ORIGINAL
      set(pos, get(pos) ^ hash ^ writer);
      cout << "v[pos] = " << v[pos] << endl;
#else
#endif
      }

   set(hash, writer);

   struct cache_entry* e = (struct cache_entry*)(v + writer);

#ifdef ORIGINAL
   e->link = pos ^ hash;
#else
   e->link = (pos ? pos : hash);
#endif
   e->str  = s;

   writer += 2;
}

void example()
{
   write(0, "primo");
   print();
   write(0, "secondo");
   print();
   write(0, "terzo");
   print();
   write(0, "quarto");
   print();
   write(0, "quinto");
   print();

   read(0, "secondo");

// writer = 1, oldest = 10, unused = 10
// pos    = 0
// link   = 0      (xor 0 0)
// v[]    = [ 1 0 134515803 0         0 0         0 0         0 0 0 ]

// writer = 3, oldest = 10, unused = 10
// pos    = 1
// v[pos] = 0 -> 3 (xor 0 0 3)
// link   = 1      (xor 1 0)
// v[]    = [ 3 3 134515803 1 134515809 0         0 0         0 0 0 ]

// writer = 5, oldest = 10, unused = 10
// pos    = 3
// v[pos] = 1 -> 4 (xor 1 0 5)
// link   = 3      (xor 3 0)
// v[]    = [ 5 3 134515803 4 134515809 3 134515817 0         0 0 0 ]

// writer = 7, oldest = 10, unused = 10
// pos    = 5
// v[pos] = 3 -> 4 (xor 3 0 7)
// link   = 5      (xor 5 0)
// v[]    = [ 7 3 134516012 4 134516018 4 134516026 5 134516032 0 0 ]

// writer = 9, oldest = 10, unused = 10
// v[oldest == 1] = 3
// v[3] = 4 -> 5 (xor 4 1)
// writer = 1, oldest = 3, unused = 9
// pos    = 7
// v[pos] = 5 -> 4 (xor 5 0 1)
// link   = 7      (xor 7 0)
// v[]    = [ 1 7 134516675 5 134516654 4 134516662 4 134516668 0 0 ]

// prevpos = 0
// pos     = 7
// str     = quarto

// prevpos = 7
// pos     = 5 (xor 0 v[pos] = 5)
// str     = terzo

// prevpos = 5
// pos     = 3 (xor 7 v[pos] = 4)
// str     = secondo

// prevpos = 3
// pos     = 1 (xor 5 v[pos] = 4)
// str     = primo
}
*/

int
U_EXPORT main (int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   /*
   example();
   return 0;
   */

   UCache c;

   /*
   char* y;
   char x[1024];
   unsigned i, ttl;
   char buffer[1024 * 10];

   c.init(buffer, 1024 * 10);
   c.reset();

   while (cin >> x)
      {
      y = strchr(x, ':');

      if (y)
         {
         i = y - x;

         c.set(x, i, x + i + 1, strlen(x) - i - 1, 86400);
         }
      else
         {
         y = c.get(x, strlen(x), i, ttl);

         if (y)
            {
            cout.write(y, i);

            cout << endl;
            }
         }
      }
   */

   (void) c.open(U_STRING_FROM_CONSTANT("./cache.file"), atoi(argv[1]));

   cin >> c;

   // Network services, Internet style
   // --------------------------------
   // +6,4:@7/tcp->echo
   // +8,1:echo/tcp->7
   // +6,4:@7/udp->echo
   // +8,1:echo/udp->7
   // +6,7:@9/tcp->discard
   // +11,1:discard/tcp->9
   // +8,1:sink/tcp->9
   // +8,1:null/tcp->9
   // +6,7:@9/udp->discard
   // +11,1:discard/udp->9
   // +8,1:sink/udp->9
   // +8,1:null/udp->9
   // +7,6:@11/tcp->systat
   // +10,2:systat/tcp->11
   // +9,2:users/tcp->11
   // +7,6:@11/udp->systat
   // +10,2:systat/udp->11
   // +9,2:users/udp->11

   bool ok;
   char buffer1[128];
   char buffer2[128];
   const char* tbl[9]  = { "@7",    "echo",    "@9",        "discard",
                           "sink",  "null",    "@11",       "systat",   "users" };
   const char* data[9] = { "echo",   "7",       "discard",  "9",
                           "9",      "9",       "systat",   "11",       "11" };

   for (int i = 0; i < 9; ++i)
      {
      strcat(strcpy(buffer1, tbl[i]), "/tcp");
      strcat(strcpy(buffer2, tbl[i]), "/udp");

      ok = c[UString(buffer1)] == UString(data[i]);

      U_INTERNAL_ASSERT( ok )

      ok = c[UString(buffer2)] == UString(data[i]);

      U_INTERNAL_ASSERT( ok )
      }

   // handles repeated keys
   // ---------------------
   // +3,5:one->Hello
   // +3,7:one->Goodbye
   // +3,7:one->Another
   // +3,5:two->Hello
   // +3,7:two->Goodbye
   // +3,7:two->Another

   ok = c[U_STRING_FROM_CONSTANT("one")] == U_STRING_FROM_CONSTANT("Another");

   U_INTERNAL_ASSERT( ok )

   ok = c[U_STRING_FROM_CONSTANT("two")] == U_STRING_FROM_CONSTANT("Another");

   U_INTERNAL_ASSERT( ok )

   // handles long keys and data
   // --------------------------
   // +320,320:ba483b3442e75cace82def4b5df25bfca887b41687537.....

#     define LKEY "ba483b3442e75cace82def4b5df25bfca887b41687537c21dc4b82cb4c36315e2f6a0661d1af2e05e686c4c595c16561d8c1b3fbee8a6b99c54b3d10d61948445298e97e971f85a600c88164d6b0b09\nb5169a54910232db0a56938de61256721667bddc1c0a2b14f5d063ab586a87a957e87f704acb7246c5e8c25becef713a365efef79bb1f406fecee88f3261f68e239c5903e3145961eb0fbc538ff506a\n"
#     define LDATA "152e113d5deec3638ead782b93e1b9666d265feb5aebc840e79aa69e2cfc1a2ce4b3254b79fa73c338d22a75e67cfed4cd17b92c405e204a48f21c31cdcf7da46312dc80debfbdaf6dc39d74694a711\n6d170c5fde1a81806847cf71732c7f3217a38c6234235951af7b7c1d32e62d480d7c82a63a9d94291d92767ed97dd6a6809d1eb856ce23eda20268cb53fda31c016a19fc20e80aec3bd594a3eb82a5a\n"

   ok = c[U_STRING_FROM_CONSTANT(LKEY)] == U_STRING_FROM_CONSTANT(LDATA);

   U_INTERNAL_ASSERT( ok )

   cout << c;
}
