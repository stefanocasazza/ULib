// ============================================================================
// C servlet sample
// ----------------------------------------------------------------------------
// hellox.c: how to get URL parameters and use them in a reply
// ============================================================================

#include <ulib/internal/csp_interface.h>
#include <ulib/base/coder/xml.h>

int main(int argc, char* argv[])
{
   unsigned char encoded[1024];

   char* reply = get_reply();

   (void) u_xml_encode(argv[1], strlen(argv[1]), encoded);

   (void) u__snprintf(reply, get_reply_capacity(), U_CONSTANT_TO_PARAM("<h1>Hello %s</h1>"), encoded);

   return 200;
}
// ============================================================================
// End of Source Code
// ============================================================================
