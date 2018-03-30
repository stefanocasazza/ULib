// serialiaze.cpp

#include <ulib/serialize/flatbuffers.h>

#undef  PACKAGE
#define PACKAGE "serialize"

#define U_OPTIONS \
"purpose 'simple serialization test'\n"

#include <ulib/application.h>

class Application : public UApplication {
public:

   Application()
      {
      U_TRACE(5, "Application::Application()")
      }

   ~Application()
      {
      U_TRACE(5, "Application::~Application()")
      }

   void run(int argc, char* argv[], char* env[])
      {
      U_TRACE(5, "Application::run(%d,%p,%p)", argc, argv, env)

      UApplication::run(argc, argv, env);

      UString result;
      UFlatBuffer fb, vec;
      uint32_t _ctime, ctraffic;

      fb.StartBuild();
      (void) fb.StartVector();
      fb.String(U_CONSTANT_TO_PARAM("121@10.8.0.130/uffGiulianiInterno-r29587_picoM2"));
      fb.String(U_CONSTANT_TO_PARAM("166a600c16a5"));
      fb.String(U_CONSTANT_TO_PARAM("172.16.18.24"));
      fb.String(U_CONSTANT_TO_PARAM("121"));
      fb.UInt(1024U);
      fb.UInt(1024U);
      fb.String(U_CONSTANT_TO_PARAM("166a600c16a5"));
      fb.String(U_CONSTANT_TO_PARAM("172.16.18.24"));
      fb.String(U_CONSTANT_TO_PARAM("121"));
      fb.UInt(1024U);
      fb.UInt(1024U);
      fb.EndVector(0, false);
      (void) fb.EndBuild();
      result = fb.getResult();

      fb.setRoot(result);
      fb.AsVector(vec);
      result = vec.AsVectorGet<UString>(0);

      U_ASSERT_EQUALS(result, "121@10.8.0.130/uffGiulianiInterno-r29587_picoM2");

      for (int32_t i = 1, n = (int32_t) vec.GetSize(); i < n; i += 5)
         {
         result = vec.AsVectorGet<UString>(i);

         U_ASSERT_EQUALS(result, "166a600c16a5");

         result = vec.AsVectorGet<UString>(i+1);

         U_ASSERT_EQUALS(result, "172.16.18.24");

         result = vec.AsVectorGet<UString>(i+2);

         U_ASSERT_EQUALS(result, "121");

         _ctime   = vec.AsVectorGet<uint32_t>(i+3);
         ctraffic = vec.AsVectorGet<uint32_t>(i+4);

         U_INTERNAL_ASSERT_EQUALS(_ctime,   1024);
         U_INTERNAL_ASSERT_EQUALS(ctraffic, 1024);
         }
      }
};

U_MAIN
