// test_soap_server.cpp

#include <ulib/file_config.h>
#include <ulib/xml/soap/soap_fault.h>
#include <ulib/xml/soap/soap_parser.h>
#include <ulib/xml/soap/soap_object.h>
#include <ulib/xml/soap/soap_encoder.h>

class USOAPExample : public USOAPObject { // USOAPObject acts as a container for a set of methods
public:

   // OBJECT SERVICE

   // URPCMethod provides an interface for the things that methods most know how to do

   class AddMethod : public URPCMethod {
   public:
      AddMethod() : buffer(U_CAPACITY)
         {
         U_TRACE_REGISTER_OBJECT(5, AddMethod, "", 0)
         }

      virtual ~AddMethod()
         {
         U_TRACE_UNREGISTER_OBJECT(5, AddMethod)
         }

      virtual UString getMethodName() const { return U_STRING_FROM_CONSTANT("add"); }

      // Transforms the method into something that SOAP servers and clients can send.
      // The encoder holds the actual data while the client hands data to be entered in

      virtual void encode()
         {
         U_TRACE(5, "AddMethod::encode()")

         if (hasFailed()) U_SOAP_ENCODE_RES(buffer);
         else             U_SOAP_ENCODE_ARG(returnValue);
         }

      // Only to be called on the server by the dispatcher.
      // This method executes the call and returns true if the call succeeded, false if it failed.
      // URPCMethods should keep any return data in a member variable. The information will be returned via a call to encode.

      virtual bool execute(URPCEnvelope& theCall)
         {
         U_TRACE(5, "AddMethod::execute(%p)", &theCall)

         U_ASSERT(theCall.getMethodName() == getMethodName())

         unsigned num_arguments = theCall.getNumArgument();

         if (num_arguments != 2)
            {
            USOAPObject::dispatcher->setFailed();

            pFault->getFaultReason() = U_STRING_FROM_CONSTANT("Invalid number of arugments");

            pFault->setDetail(U_CONSTANT_TO_PARAM("The fault occurred because the add method expected 2 arguments "
                              "but received %d arguments"), num_arguments);

            pFault->encode(buffer);

            U_RETURN(false);
            }

         long a = theCall.getArgument(0).strtol(10),
              b = theCall.getArgument(1).strtol(10);

         returnValue = a + b;

         U_RETURN(true);
         }

   private:
      UString buffer;
      long returnValue;
   };

   // OBJECT SERVICE

   class ReverseMethod : public URPCMethod {
   public:
      ReverseMethod() : returnValue(U_CAPACITY)
         {
         U_TRACE_REGISTER_OBJECT(5, ReverseMethod, "", 0)
         }

      virtual ~ReverseMethod()
         {
         U_TRACE_UNREGISTER_OBJECT(5, ReverseMethod)
         }

      virtual UString getMethodName() const { return U_STRING_FROM_CONSTANT("reverse"); }

      virtual void encode()
         {
         U_TRACE(5, "ReverseMethod::encode()")

         if (hasFailed()) U_SOAP_ENCODE_RES(returnValue);
         else             U_SOAP_ENCODE_ARG(returnValue);
         }

      virtual bool execute(URPCEnvelope& theCall)
         {
         U_TRACE(5, "ReverseMethod::execute(%p)", &theCall)

         U_ASSERT(theCall.getMethodName() == getMethodName())

         unsigned num_arguments = theCall.getNumArgument();

         if (num_arguments != 1)
            {
            USOAPObject::dispatcher->setFailed();

            pFault->getFaultReason() = U_STRING_FROM_CONSTANT("Invalid number of arugments");

            pFault->setDetail(U_CONSTANT_TO_PARAM("The fault occurred because the reverse method expected 1 arguments "
                              "but received %d arguments"), num_arguments);

            pFault->encode(returnValue);

            U_RETURN(false);
            }

         UString str = theCall.getArgument(0);
         unsigned n  = str.size();

         returnValue.size_adjust(n);

         char* ptr_src =         str.data() + n - 1;
         char* ptr_dst = returnValue.data();

         for (unsigned i = 0; i < n; ++i) *ptr_dst++ = *ptr_src--;

         U_RETURN(true);
         }

   private:
      UString returnValue;
   };

   // COSTRUTTORE

   USOAPExample(UFileConfig& file_method)
      {
      U_TRACE_REGISTER_OBJECT(5, USOAPExample, "%p", &file_method)

      U_NEW(USOAPObject, dispatcher, USOAPObject);
      U_NEW(USOAPEncoder, URPCMethod::encoder, USOAPEncoder);

      URPCMethod* pmethod;

      U_NEW(AddMethod, pmethod, AddMethod);

      USOAPObject::insertMethod(pmethod);

      U_NEW(ReverseMethod, pmethod, ReverseMethod);

      USOAPObject::insertMethod(pmethod);

      // Load generic method

      readFileMethod(file_method);
      }

   virtual ~USOAPExample()
      {
      U_TRACE_UNREGISTER_OBJECT(5, USOAPExample)

      delete dispatcher;
      delete URPCMethod::encoder;
      }
};

int
U_EXPORT main (int argc, char* argv[], char* env[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5, "main(%d)",argc)

   UString::str_allocate(STR_ALLOCATE_SOAP);

   UFileConfig method_file;

   method_file.load(UString(argv[1]));

   // Perform registration

   USOAPExample dispatcher(method_file);

   // Process the SOAP message -- should be the contents of the message from "<SOAP:" to the end of the string

   bool bSendingFault;
   UString filename, msg;
   UVector<UString>* pvec;

   U_NEW(UVector<UString>, pvec, UVector<UString>);

   USOAPParser parser(pvec);

   while (cin >> filename)
      {
      msg = UFile::contentOf(filename);

      cout << parser.processMessage(msg, dispatcher, bSendingFault) << "\n";

      parser.clearData();
      }
}
