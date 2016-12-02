// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    tokenizer.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_TOKENIZER_H
#define ULIB_TOKENIZER_H 1

#include <ulib/string.h>

class UValue;
class UQueryParser;

template <class T> class UVector;

class U_EXPORT UTokenizer {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   UTokenizer(const char* d = 0)
      {
      U_TRACE_REGISTER_OBJECT(0, UTokenizer, "%S", d)

          s =
        end = 0;
      delim = d;
      }

   UTokenizer(const UString& data, const char* d = 0) : str(data)
      {
      U_TRACE_REGISTER_OBJECT(0, UTokenizer, "%V,%S", data.rep, d)

      s     = data.data();
      end   = data.pend();
      delim = d;
      }

   ~UTokenizer()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UTokenizer)
      }

   bool atEnd()
      {
      U_TRACE_NO_PARAM(0, "UTokenizer::atEnd()")

      if (s < end) U_RETURN(false);

      U_RETURN(true);
      }

   void setDelimiter(const char* sep)
      {
      U_TRACE(0, "UTokenizer::setDelimiter(%S)", sep)

      delim = sep;
      }

   void setData(const UString& data)
      {
      U_TRACE(0, "UTokenizer::setData(%V)", data.rep)

      str = data;
      end = (s = data.data()) + data.size();
      }

   void skipSpaces()
      {
      U_TRACE_NO_PARAM(0, "UTokenizer::skipSpaces()")

      if (u__isspace(*s))
         {
         while (u__isspace(*++s)) {} 

         if (s > end) s = end;
         }
      }

   void setPointer(const char* ptr)
      {
      U_TRACE(0, "UTokenizer::setPointer(%S)", ptr)

      U_INTERNAL_ASSERT(ptr <= end)

      s = ptr;
      }

   const char* getPointer() const
      {
      U_TRACE_NO_PARAM(0, "UTokenizer::getPointer()")

      U_RETURN(s);
      }

   const char* getEnd() const
      {
      U_TRACE_NO_PARAM(0, "UTokenizer::getEnd()")

      return end;
      }

   void setDistance(uint32_t pos)
      {
      U_TRACE(0, "UTokenizer::setDistance(%u)", pos)

      s = str.c_pointer(pos);
      }

   uint32_t getSize() const __pure
      {
      U_TRACE(0, "UTokenizer::getSize()")

      uint32_t pos = str.size();

      U_RETURN(pos);
      }

   uint32_t getDistance() const __pure
      {
      U_TRACE(0, "UTokenizer::getDistance()")

      uint32_t pos = (s < end ? str.distance(s) : str.size());

      U_RETURN(pos);
      }

   // get prev char

   char prev()
      {
      U_TRACE_NO_PARAM(0, "UTokenizer::prev()")

      U_RETURN(*(s-1));
      }

   // go to prev position

   void back()
      {
      U_TRACE_NO_PARAM(0, "UTokenizer::back()")

      U_INTERNAL_ASSERT_MAJOR(s, str.data())

      --s;
      }

   // get current char

   char current()
      {
      U_TRACE_NO_PARAM(0, "UTokenizer::current()")

      U_INTERNAL_ASSERT(s <= end)

      U_RETURN(*s);
      }

   // get current char and advance one position

   char next()
      {
      U_TRACE_NO_PARAM(0, "UTokenizer::next()")
 
      U_INTERNAL_ASSERT(s < end)

      U_RETURN(*s++);
      }

   void advance()
      {
      U_TRACE_NO_PARAM(0, "UTokenizer::advance()")
 
      U_INTERNAL_ASSERT(s <= end)

      ++s;
      }

   UTokenizer& operator++() { advance(); return *this; } // ++tok

   // get next token

   bool   next(UString& tok, char c);
   bool extend(UString& tok, char c); // extend the actual token to the next char 'c'... (see PEC_report.cpp)

   bool next(UString& tok, bPFi func);
   bool next(UString& tok, bool* bgroup);

   // EXT

   bool skipToken();
   bool skipToken(const char* token, uint32_t sz)
      {
      U_TRACE(0, "UTokenizer::skipToken(%.*S,%u)", sz, token, sz)

      if ((uint32_t)str.remain(s) >= sz &&
          memcmp(s, token, sz) == 0)
         {
         s += sz;

         U_RETURN(true);
         }

      U_RETURN(false);
      }

   void skipNumber()
      {
      U_TRACE_NO_PARAM(0, "UTokenizer::skipNumber()")

      for (; s < end; ++s)
         {
         char c = *s;

         if (u__isnumberchar(c) == false &&
             u__toupper(c) != 'E') // scientific notation (Ex: 1.45e-10)
            {
            break;
            }
         }
      }

   UString substr() const
      {
      U_TRACE_NO_PARAM(0, "UTokenizer::substr()")

      UString result;

      if (s < end) result = str.substr(str.distance(s));

      U_RETURN_STRING(result);
      }

   UString substr(const char* start) const
      {
      U_TRACE(0, "UTokenizer::substr(%p)", start)

      UString result;

      if (start < end) result = str.substr(start, s - start);

      U_RETURN_STRING(result);
      }

   UString getTokenQueryParser();

   int  getTokenId(UString* ptoken);
   bool tokenSeen(const UString* x);

   static const char* group;
   static bool group_skip, avoid_punctuation;
   static uint32_t group_len, group_len_div_2;

   static void setGroup(const char* grp, uint32_t grp_len = 0, bool bskip = false)
      {
      U_TRACE(0, "UTokenizer::setGroup(%S,%u,%b)", grp, grp_len, bskip)

      group           = grp;
      group_skip      = bskip;
      group_len       = grp_len;
      group_len_div_2 = grp_len / 2;
      }

   static void setSkipTagXML(bool flag)
      {
      U_TRACE(0, "UTokenizer::setSkipTagXML(%b)", flag)

      if (flag) setGroup(U_CONSTANT_TO_PARAM("<>"), true);
      else      setGroup(0,                      0, false);
      }

   static void setAvoidPunctuation(bool flag)
      {
      U_TRACE(0, "UTokenizer::setAvoidPunctuation(%b)", flag)

      avoid_punctuation = flag;
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   const char* s;
   const char* end;
   const char* delim;
   UString str;

private:
   U_DISALLOW_COPY_AND_ASSIGN(UTokenizer)

   friend class UQueryParser;
};

#endif
