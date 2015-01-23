// test_soap_client.cpp

#include <ulib/notifier.h>
#include <ulib/utility/services.h>
#include <ulib/xml/soap/soap_client.h>

template <class T> class UTestSOAPClient : public USOAPClient<T> {
public:

   // COSTRUTTORE

    UTestSOAPClient() : USOAPClient<T>(0) {}
   ~UTestSOAPClient()                     {}

   // redefine method VIRTUAL class USOAPClient

   virtual bool sendRequest()
      {
      U_TRACE(0, "UTestSOAPClient::sendRequest()")

      uint32_t size = this->request.size();

      bool result = (UNotifier::write(STDOUT_FILENO, this->request.data(), size) == size);

      U_RETURN(result);
      }

   virtual bool readResponse()
      {
      U_TRACE(0, "UTestSOAPClient::readResponse()")

      bool result = (UServices::read(STDIN_FILENO, this->response, U_SINGLE_READ, 1 * 1000) > 0);

      U_RETURN(result);
      }

   // OBJECT FOR METHOD REQUEST

   class AddMethod : public URPCMethod { // URPCMethod provides an interface for the things that methods most know how to do
   public:
      long a, b;

      AddMethod()
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

         U_SOAP_ENCODE_ARG(a);
         U_SOAP_ENCODE_ARG(b);
         }
   };

   // OBJECT FOR METHOD REQUEST

   class ReverseMethod : public URPCMethod {
   public:
      UString str;

      ReverseMethod()
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

         U_SOAP_ENCODE_ARG(str);
         }
   };

   AddMethod     m_add;
   ReverseMethod m_reverse;

   // SERVICES

   long add(long a, long b)
      {
      U_TRACE(5, "UTestSOAPClient::add(%ld,%ld)", a, b)

      m_add.a = a;
      m_add.b = b;

      if (USOAPClient<T>::processRequest(m_add))
         {
         long result = USOAPClient<T>::getResponse().strtol(); // Get the value of the element inside the response

         U_RETURN(result);
         }

      U_RETURN(-1);
      }

   UString reverse(const UString& str)
      {
      U_TRACE(5, "UTestSOAPClient::reverse(%.*S)", U_STRING_TO_TRACE(str))

      m_reverse.str = str;

      UString result;

      if (USOAPClient<T>::processRequest(m_reverse))
         {
         result = USOAPClient<T>::getResponse(); // Get the value of the element inside the response
         }

      U_RETURN_STRING(result);
      }
   };

int
U_EXPORT main (int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5, "main(%d)",argc)

   UTestSOAPClient<USocket> testSOAP;

   testSOAP.setHostPort(U_STRING_FROM_CONSTANT("localhost"), 80);

   if (strcmp(argv[1], "add") == 0)
      {
      long result = testSOAP.add(10, 20);

      U_INTERNAL_ASSERT(result == 30)
      }
   else
      {
      UString reverse = testSOAP.reverse(U_STRING_FROM_CONSTANT("0123456789"));

      U_INTERNAL_ASSERT(reverse == U_STRING_FROM_CONSTANT("9876543210"))
      }
}
