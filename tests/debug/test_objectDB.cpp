// test_objectDB.cpp

#include <ulib/debug/trace.h>
#include <ulib/debug/common.h>
#include <ulib/debug/objectDB.h>
#include <ulib/debug/error_memory.h>

#include <stdlib.h>

class Genitore {
public:
   U_MEMORY_TEST

   Genitore(const char* n = "genitore")
      {
      U_TRACE_REGISTER_OBJECT(5,Genitore,"%S",n)

      nome = n;
      }

   ~Genitore()
      {
      U_TRACE_UNREGISTER_OBJECT(5,Genitore)
      }

   Genitore(const Genitore& g) { *this = g; }

   Genitore& operator=(const Genitore& g)
      {
      nome = g.nome;
      
      return *this;
      }

// friend ostream& operator<<(ostream& os, const Genitore& genitore) { return os << genitore.nome; }

   const char* dump(bool) const { return nome; }

protected:
   const char* nome;
};

class Figlio : public Genitore {
public:

   Figlio(const char* n = "figlio") : Genitore(n)
      {
      U_TRACE_REGISTER_OBJECT(5, Figlio, "%S", n)
      }

   ~Figlio()
      {
      U_TRACE_UNREGISTER_OBJECT(5, Figlio)
      }

   const char* dump(bool) const { return nome; }
};

static void one()
{
   U_TRACE(5, "one()")

   Figlio* vettore_figli = U_NEW(Figlio[3]);
}

static void two()
{
   U_TRACE(5, "two()")

   Figlio* vettore_figli = U_NEW(Figlio[3]);
}

int U_EXPORT main (int argc, char* argv[])
{
   u_init_ulib(argv);

   U_TRACE(5, "main(%d,%p)", argc, argv)

   int sz = (argc > 1 ? atoi(argv[1]) : 1000);

   U_SET_LOCATION_INFO;
   Figlio   figlio;
   U_SET_LOCATION_INFO;
   Genitore genitore;

   one();
   two();

   Figlio* vettore_figli = U_NEW(Figlio[sz]);

   delete[] vettore_figli;

   ::exit(0);
}
