// cquery.h

#ifndef IR_CQUERY_H
#define IR_CQUERY_H 1

#include "IR.h"

class WeightWord {
public:

   // Check Memory
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   UString filename;
   uint32_t word_freq;

   static bool check_for_duplicate;
   static  UVector<WeightWord*>* vec;
   static UHashMap<WeightWord*>* tbl;

   // COSTRUTTORE

   WeightWord() : word_freq(0)
      {
      U_TRACE_CTOR(5, WeightWord, "")
      }

   WeightWord(const UString& x, uint32_t f) : filename(x), word_freq(f)
      {
      U_TRACE_CTOR(5, WeightWord, "%.*S,%u", U_STRING_TO_TRACE(filename), word_freq)
      }

   WeightWord(const WeightWord& w) : filename(w.filename)
      {
      U_TRACE_CTOR(5, WeightWord, "%p", &w)

      U_MEMORY_TEST_COPY(w)

      ((UString&)w.filename).clear();

      word_freq = w.word_freq;
      }

   ~WeightWord()
      {
      U_TRACE_DTOR(5, WeightWord)
      }

   // SERVICES

   void toBuffer(UString& data_buffer)
      {
      U_TRACE(5, "WeightWord::toBuffer(%.*S)", U_STRING_TO_TRACE(data_buffer))

      U_CHECK_MEMORY

      char buffer[U_PATH_MAX+1];

      uint32_t len = u__snprintf(buffer, U_PATH_MAX, U_CONSTANT_TO_PARAM(" %u \"%.*s\""), word_freq, U_STRING_TO_TRACE(filename));

      (void) data_buffer.append(buffer, len);
      }

   static void push();
   static void clear();
   static void sortObjects();
   static void dumpObjects();
   static int  compareObj(const void* obj1, const void* obj2) __pure;

   static uint32_t size() { return (vec ? vec->size() : 0); }

   // STREAM

#ifdef U_STDCPP_ENABLE
   void fromStream(istream& is)
      {
      U_TRACE(5, "WeightWord::fromStream(%p)", &is)

      U_CHECK_MEMORY

      is >> word_freq;

      is.get(); // skip ' '

      filename.get(is);
      }

   friend istream& operator>>(istream& is, WeightWord& w) { w.fromStream(is); return is; }

   // DEBUG

#  ifdef DEBUG
   const char* dump(bool reset) const;
#  endif
#endif
};

class UQueryParser;

class Query {
public:

   // Check Memory
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   // COSTRUTTORE

    Query();
   ~Query();

   // SERVICES

   void run(const char* ptr, uint32_t len, UVector<WeightWord*>* vec = U_NULLPTR);

   static void        clear();
   static const char* checkQuoting(char* argv[], uint32_t& len); // NB: may be there are some difficult with quoting (MINGW)...

#ifdef DEBUG
   const char* dump(bool reset) const { return ""; }
#endif

protected:
   static UString* request;
   static UQueryParser* parser;

   static int push(      UStringRep* str_inode, UStringRep* filename);
   static int query_expr(UStringRep* str_inode, UStringRep* filename);
   static int query_meta(UStringRep*  word_rep, UStringRep* value);
};

#endif
