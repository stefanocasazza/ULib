// ==============================================================================================
// C servlet sample
// ----------------------------------------------------------------------------------------------
// captcha.c: how to use small portable CAPTCHA images generator library without dependencies
// ==============================================================================================

#include <ulib/internal/csp_interface.h>
#include <ulib/base/coder/base64.h>

#pragma link "libcaptcha.o"

extern void captcha(unsigned char* im, unsigned char* l);
extern void makegif(unsigned char* im, unsigned char* gif);

#define GIF_SIZE 17646

int main(int argc, char* argv[])
{
   unsigned char l[6];
   unsigned char im[70*200];
   unsigned char gif[GIF_SIZE];
   unsigned char encoded[GIF_SIZE * 3];

   captcha(im, l);
   makegif(im, gif);

   set_reply_capacity(1024 + GIF_SIZE * 3);

   (void) u__snprintf(get_reply(), get_reply_capacity(), U_CONSTANT_TO_PARAM("<img src=\"data:img/gif;base64,%.*s\">"), u_base64_encode(gif, GIF_SIZE, encoded), encoded);

   return 200;
}
// ============================================================================
// End of Source Code
// ============================================================================
