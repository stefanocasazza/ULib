// test_twilio.cpp

#include <ulib/net/client/twilio.h>

int main(int argc, char *argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   UTwilioClient tc(U_STRING_FROM_CONSTANT("SID"), U_STRING_FROM_CONSTANT("TOKEN"));

   bool ok = tc.getCompletedCalls();

   U_INTERNAL_ASSERT(ok)

   ok = tc.makeCall(U_CONSTANT_TO_PARAM("xxx-xxx-xxxx"), U_CONSTANT_TO_PARAM("xxx-xxx-xxxx"), U_CONSTANT_TO_PARAM("http://xxxx"));

   U_INTERNAL_ASSERT(ok)

   ok = tc.sendSMS(U_CONSTANT_TO_PARAM("xxx-xxx-xxxx"), U_CONSTANT_TO_PARAM("xxx-xxx-xxxx"), U_CONSTANT_TO_PARAM("\"Hello, how are you?\""));

   U_INTERNAL_ASSERT(ok)
}
