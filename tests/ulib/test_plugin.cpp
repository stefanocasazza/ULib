// test_plugin.cpp

#include <ulib/string.h>

#include "plugin/product.h"

/*
static char ap_ref[100];

static void setAccessPointReference(const char* s, uint32_t n)
{
   U_TRACE(5, "::setAccessPointReference(%.*S,%u)", n, s, n)

   // Ex: ap@10.8.1.2 => X0256Rap

   uint32_t count = 0, certid = 0;

   char* ptr;
   unsigned char c = *s;

   while (n--)
      {
      U_INTERNAL_DUMP("c = %C n = %u count = %u certid = %u", c, n, count, certid)

      if (c == '.') ++count;
      else
         {
         U_INTERNAL_ASSERT(u__isdigit(c))

         if (count == 2)
            {
            certid = 254 * strtol(s, &ptr, 10);

            c = *(s = ptr);

            continue;
            }

         if (count == 3)
            {
            certid += strtol(s, 0, 10);

            break;
            }
         }

      c = *(++s);
      }

   (void) u__snprintf(ap_ref, sizeof(ap_ref), U_CONSTANT_TO_PARAM("X%04dRap"), certid);

   U_INTERNAL_DUMP("ap_ref = %S", ap_ref)
}
*/

int
U_EXPORT main (int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   UDynamic::setPluginDirectory(UString(argv[1]));

   Product* b1 = UPlugIn<Product*>::create(argv[2], strlen(argv[2]));

   if (b1 == 0) cout << "object with key " << argv[2] << " cannot be created" << endl;
   else         cout << *b1 << endl;

   Product* b2 = UPlugIn<Product*>::create(argv[3], strlen(argv[3]));

   if (b2 == 0) cout << "object with key " << argv[3] << " cannot be created" << endl;
   else         cout << *b2 << endl;

   U_ASSERT(UPlugIn<Product*>::empty() == false)

   delete b2;
   delete b1;

   // UPlugIn<void*>::clear();
}
