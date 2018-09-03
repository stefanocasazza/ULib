// product1.cpp

#include "product.h"

class U_EXPORT Product1 : public Product {
public:
            Product1() {}
   virtual ~Product1() { (void) write(1, U_CONSTANT_TO_PARAM("distruttore Product1\n")); }

   virtual void print(ostream& os) const
      {
#  if defined(U_STDCPP_ENABLE)
      os.write(U_CONSTANT_TO_PARAM("I am Product1\n"));
#  else
      (void) write(1, U_CONSTANT_TO_PARAM("I am Product1\n"));
#  endif
      }
};

U_CREAT_FUNC(product1, Product1)
