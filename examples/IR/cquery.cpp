// cquery.cpp

#include <ulib/query/parser.h>
#include <ulib/utility/string_ext.h>

#include "cquery.h"

bool                   WeightWord::check_for_duplicate;
UVector<WeightWord*>*  WeightWord::vec;
UHashMap<WeightWord*>* WeightWord::tbl;

void WeightWord::clear()
{
   U_TRACE(5, "WeightWord::clear()")

   if (tbl)
      {
      tbl->_length = 0;
      tbl->deallocate();

      delete tbl;
             tbl = 0;
      }

   if (vec)
      {
      delete vec;
             vec = 0;
      }
}

void WeightWord::push()
{
   U_TRACE(5, "WeightWord::push()")

   U_INTERNAL_DUMP("UPosting::word_freq = %d", UPosting::word_freq)

   U_INTERNAL_ASSERT(*UPosting::filename)

   WeightWord* item = U_NEW(WeightWord(*UPosting::filename, UPosting::word_freq));

   if (check_for_duplicate)
      {
      if (tbl == 0) tbl = U_NEW(UHashMap<WeightWord*>);

      if (tbl->find(*UPosting::filename))
         {
         U_INTERNAL_DUMP("DUPLICATE")

         delete item;

         return;
         }

      tbl->insertAfterFind(*UPosting::filename, item);
      }

   if (vec == 0) vec = U_NEW(UVector<WeightWord*>);

   vec->push_back(item);
}

__pure int WeightWord::compareObj(const void* obj1, const void* obj2)
{
   U_TRACE(5, "WeightWord::compareObj(%p,%p)", obj1, obj2)

   int cmp = ((*(const WeightWord**)obj1)->word_freq < (*(const WeightWord**)obj2)->word_freq ?  1 :
              (*(const WeightWord**)obj1)->word_freq > (*(const WeightWord**)obj2)->word_freq ? -1 :
              (*(const WeightWord**)obj1)->filename.compare((*(const WeightWord**)obj2)->filename));

   return cmp;
}

void WeightWord::sortObjects()
{
   U_TRACE(5+256, "WeightWord::sortObjects()")

   if (size() > 1) vec->sort(compareObj); 
}

void WeightWord::dumpObjects()
{
   U_TRACE(5, "WeightWord::dumpObjects()")

   if (vec)
      {
      sortObjects();

      for (uint32_t i = 0, n = vec->size(); i < n; ++i)
         {
         (void) write(1, U_STRING_TO_PARAM((*vec)[i]->filename));
         (void) write(1, U_CONSTANT_TO_PARAM("\n"));
         }
      }
}

UString*      Query::request;
UQueryParser* Query::parser;

Query::Query()
{
   U_TRACE(5, "Query::Query()")

   U_INTERNAL_ASSERT_EQUALS(parser,  0)
   U_INTERNAL_ASSERT_EQUALS(request, 0)

   parser  = U_NEW(UQueryParser);
   request = U_NEW(UString);
}

Query::~Query()
{
   U_TRACE(5, "Query::~Query()")

   clear();

   delete parser;
   delete request;
}

void Query::clear()
{
   U_TRACE(5, "Query::clear()")

     UPosting::reset();
   WeightWord::clear();

    parser->clear();
   request->clear();
}

int Query::query_meta(UStringRep* word_rep, UStringRep* value)
{
   U_TRACE(5, "Query::query_meta(%.*S,%p)", U_STRING_TO_TRACE(*word_rep), value)

   if (u_pfn_match(      word_rep->data(),       word_rep->size(),
               UPosting::word->data(), UPosting::word->size(), u_pfn_flags))
      {
      UPosting::posting->_assign(value);

      UPosting::callForPostingAndSetFilename(WeightWord::push);
      }

   U_RETURN(1);
}

int Query::push(UStringRep* str_inode, UStringRep* filename)
{
   U_TRACE(5, "Query::push(%#.*S,%.*S)", U_STRING_TO_TRACE(*str_inode), U_STRING_TO_TRACE(*filename))

   UPosting::filename->_assign(filename);

   UPosting::word_freq = 0;

   WeightWord::push();

   U_RETURN(1);
}

int Query::query_expr(UStringRep* str_inode, UStringRep* filename)
{
   U_TRACE(5, "Query::query_expr(%#.*S,%.*S)", U_STRING_TO_TRACE(*str_inode), U_STRING_TO_TRACE(*filename))

   UPosting::setDocID(str_inode);

   if (parser->evaluate()) push(str_inode, filename);

   U_RETURN(1);
}

// NB: may be there are difficult with quoting (MINGW)...

const char* Query::checkQuoting(char* argv[], uint32_t& len)
{
   U_TRACE(5, "Query::checkQuoting(%p,%u)", argv, len)

   U_INTERNAL_DUMP("optind = %d", optind)

   U_INTERNAL_ASSERT_RANGE(1,optind,3)

   U_DUMP_ATTRS(argv)

   // [0] -> path_prog
   // [1] -> "-c"
   // [2] -> "index.cfg"
   // [3] -> "query..."
   // [4] -> '\0'

   const char* ptr = argv[optind];

   if (argv[optind+1] == 0) len = u__strlen(ptr, __PRETTY_FUNCTION__);
   else
      {
      request->setBuffer(U_CAPACITY);

      do {
         U_INTERNAL_DUMP("ptr = %S", ptr)

         bool bquote = (*ptr != '"' && strchr(ptr, ' ') != 0);

                     request->push_back(' ');
         if (bquote) request->push_back('"');
              (void) request->append(ptr);
         if (bquote) request->push_back('"');
         }
      while ((ptr = argv[++optind]) && ptr[0]);

      len = request->size()-1;
      ptr = request->c_str()+1;

      U_WARNING("quoting issue detected, actual query to be executed is(%u) <%.*s>", len, ptr);
      }

   U_RETURN_POINTER(ptr,const char);
}

void Query::run(const char* ptr, uint32_t len, UVector<WeightWord*>* vec)
{
   U_TRACE(5, "Query::run(%.*S,%u,%p)", len, ptr, len, vec)

   if (vec)
      {
      vec->clear();

      WeightWord::vec = vec;
      }

   *UPosting::word = UStringExt::removeEscape(UStringExt::trim(ptr, len));

   U_INTERNAL_DUMP("UPosting::word = %.*S", U_STRING_TO_TRACE(*UPosting::word))

   if (UServices::dosMatchWithOR(*UPosting::word, U_CONSTANT_TO_PARAM("* or *|* and *|*not *"), FNM_IGNORECASE))
      {
      *UPosting::word = UStringExt::substitute(*UPosting::word, U_CONSTANT_TO_PARAM(" or "),
                                                                U_CONSTANT_TO_PARAM(" OR "));
      *UPosting::word = UStringExt::substitute(*UPosting::word, U_CONSTANT_TO_PARAM(" and "),
                                                                U_CONSTANT_TO_PARAM(" AND "));
      *UPosting::word = UStringExt::substitute(*UPosting::word, U_CONSTANT_TO_PARAM("not "),
                                                                U_CONSTANT_TO_PARAM("NOT "));

      if (parser->parse(*UPosting::word))
         {
         parser->startEvaluate(UPosting::findDocID);

         cdb_names->callForAllEntryWithPattern(query_expr, 0);
         }
      }
   else
      {
      bool is_space = (UPosting::word->findWhiteSpace() != U_NOT_FOUND);

      if (UPosting::word->find('?') == U_NOT_FOUND &&
          UPosting::word->find('*') == U_NOT_FOUND)
         {
         UPosting::callForPosting(WeightWord::push, is_space);
         }
      else
         {
         if (is_space) U_ERROR("syntax error on query");

         if (UPosting::word->equal(U_CONSTANT_TO_PARAM("*"))) cdb_names->callForAllEntryWithPattern(push, 0);
         else
            {
            WeightWord::check_for_duplicate = true;

            if (UPosting::ignore_case) u_pfn_flags |= FNM_CASEFOLD;

            cdb_words->callForAllEntryWithPattern(query_meta, 0);

            WeightWord::check_for_duplicate = false;
            }
         }
      }
}

// DEBUG

#if defined(DEBUG) && defined(U_STDCPP_ENABLE)
const char* WeightWord::dump(bool reset) const
{
   *UObjectIO::os << "word_freq          " << word_freq        << '\n'
                  << "filename  (UString " << (void*)&filename << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
