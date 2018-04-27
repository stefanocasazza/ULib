// posting.cpp - class support for inverted index (data structure)

#include <ulib/db/rdb.h>
#include <ulib/base/utility.h>
#include <ulib/utility/string_ext.h>

#include "posting.h"

extern UCDB* cdb_names;
extern UCDB* cdb_words;

// public

bool               UPosting::change_dir;
bool               UPosting::ignore_case;
bool               UPosting::dir_content_as_doc;
UFile*             UPosting::file;
UString*           UPosting::word;
UString*           UPosting::content;
UString*           UPosting::posting;
UString*           UPosting::filename;
UString*           UPosting::str_cur_doc_id;
uint32_t           UPosting::word_freq;
uint32_t           UPosting::min_word_size;
uint32_t           UPosting::tbl_name_space;
uint32_t           UPosting::tbl_words_space;
uint32_t           UPosting::max_distance = 2;
UHashMap<UString>* UPosting::tbl_name;
UHashMap<UString>* UPosting::tbl_words;

// private

vPF                UPosting::pfunction;
char*              UPosting::ptr;
char*              UPosting::data;
char*              UPosting::ptr_cur_doc_id;
uint64_t           UPosting::cur_doc_id;
uint32_t           UPosting::pos;
uint32_t           UPosting::pos_start;
uint32_t           UPosting::space;
uint32_t           UPosting::distance;
uint32_t           UPosting::size_entry;
uint32_t           UPosting::sub_word_size;
uint32_t           UPosting::off_last_doc_id;
uint32_t           UPosting::vec_sub_word_size;
uint32_t           UPosting::sub_word_pos_prev;
uint32_t           UPosting::approximate_num_words;
UString*           UPosting::sub_word;
UVector<UString>*  UPosting::vec_word;
UVector<UString>*  UPosting::vec_entry;
UVector<UString>*  UPosting::vec_posting;
UVector<UString>*  UPosting::vec_sub_word;
UVector<UString>*  UPosting::vec_sub_word_posting;

typedef struct u_property {
   uint32_t is_meta;
   uint32_t is_quoted;
} u_property;

static u_property property[32];

/**
 * uint32_t off_last_doc_id;
 *
 * typedef struct u_posting {
 *    uint64_t doc_id;
 *    uint32_t word_freq;
 *    // -----> array of unsigned with word_freq elements
 * } u_posting;
 *
 * +------+--------------------+--------+-----------+-------+-----+-------+-----+--------+-----------+-------+-----+-------+
 * | WORD | offset last DOC id | DOC id | frequency | pos 1 | ... | pos n | ... | DOC id | frequency | pos 1 | ... | pos n |
 * +------+--------------------+--------+-----------+-------+-----+-------+-----+--------+-----------+-------+-----+-------+
 */

#define POSTING32(x,attr) u_get_unaligned32(((u_posting*)(x))->attr)
#define POSTING64(x,attr) u_get_unaligned64(((u_posting*)(x))->attr)

#define POSTING_SIZE(x) (sizeof(u_posting)+((POSTING32(x,word_freq))*sizeof(uint32_t)))

#define POSTING_POS(n)             u_get_unalignedp32(ptr+sizeof(u_posting)+(n*sizeof(uint32_t)))
#define POSTING_OFFSET_LAST_DOC_ID u_get_unalignedp32(data)

UPosting::UPosting(uint32_t dimension, bool parsing, bool index)
{
   U_TRACE_CTOR(5, UPosting, "%u,%b,%b", dimension, parsing, index)

   U_INTERNAL_ASSERT_EQUALS(word, U_NULLPTR)
   U_INTERNAL_ASSERT_EQUALS(posting, U_NULLPTR)
   U_INTERNAL_ASSERT_EQUALS(filename, U_NULLPTR)
   U_INTERNAL_ASSERT_EQUALS(str_cur_doc_id, U_NULLPTR)

   U_NEW_STRING(word, UString);
   U_NEW_STRING(posting, UString);
   U_NEW_STRING(filename, UString);
   U_NEW_STRING(str_cur_doc_id, UString(sizeof(cur_doc_id)));

   approximate_num_words = 2000 + (dimension * 8);

   if (index)
      {
      U_INTERNAL_ASSERT_EQUALS(tbl_name, U_NULLPTR)
      U_INTERNAL_ASSERT_EQUALS(tbl_words, U_NULLPTR)

      dimension += dimension / 4;

      U_NEW(UHashMap<UString>, tbl_name,  UHashMap<UString>(u_nextPowerOfTwo(dimension)));
      U_NEW(UHashMap<UString>, tbl_words, UHashMap<UString>(u_nextPowerOfTwo(approximate_num_words), ignore_case));
      }

   if (parsing)
      {
      U_INTERNAL_ASSERT_EQUALS(file, U_NULLPTR)
      U_INTERNAL_ASSERT_EQUALS(content, U_NULLPTR)

      U_NEW(UFile, file, UFile);
      U_NEW_STRING(content, UString);
      }
}

void UPosting::resetVectorCompositeWord()
{
   U_TRACE(5, "UPosting::resetVectorCompositeWord()")

   U_DELETE(sub_word)
   U_DELETE(vec_sub_word)
   U_DELETE(vec_sub_word_posting)

   sub_word             = U_NULLPTR;
   vec_sub_word         = U_NULLPTR;
   vec_sub_word_posting = U_NULLPTR;
   vec_sub_word_size    = 0;
}

void UPosting::reset()
{
   U_TRACE(5, "UPosting::reset()")

   max_distance      = 2;
   sub_word_pos_prev = 0;

   if (vec_sub_word) resetVectorCompositeWord();

   if (vec_word)
      {
      U_DELETE(vec_word)
      U_DELETE(vec_entry)
      U_DELETE(vec_posting)

      vec_word    = U_NULLPTR;
      vec_entry   = U_NULLPTR;
      vec_posting = U_NULLPTR;
      }
}

UPosting::~UPosting()
{
   U_TRACE_DTOR(5, UPosting)

   U_DELETE(posting)
   U_DELETE(filename)
   U_DELETE(str_cur_doc_id)

   if (tbl_name == U_NULLPTR) reset();
   else
      {
      U_DELETE(tbl_name)
      U_DELETE(tbl_words)
      }

   U_DELETE(word)

   if (file)
      {
      U_DELETE(file)
      U_DELETE(content)
      }
}

// #define U_OPTIMIZE
// #define U_COMPRESS_ENTRY

// MANAGE POSTING VALUE ON DATABASE

inline bool UPosting::decompress()
{
   U_TRACE(5, "UPosting::decompress()")

#ifdef U_COMPRESS_ENTRY
   if (posting->size() > (sizeof(uint32_t) * 4) &&
       UStringExt::isCompress(*posting))
      {
      posting->decompress();

      U_RETURN(true);
      }
#endif

   U_RETURN(false);
}

U_NO_EXPORT void UPosting::readPosting(UStringRep* word_rep, bool flag)
{
   U_TRACE(5, "UPosting::readPosting(%p,%b)", word_rep, flag)

   U_INTERNAL_ASSERT_POINTER(cdb_words)

   *posting = (word_rep ? (flag ? (*((URDB*)cdb_words))[word_rep]
                                : (*        cdb_words) [word_rep])
                        : ((URDB*)cdb_words)->UCDB::elem());

#ifdef U_COMPRESS_ENTRY
   if (decompress() == false)
#endif
      {
      // check if add operation...

      if (word_rep == U_NULLPTR) posting->duplicate(); // NB: need duplicate string because we need space on string constant..
      }

   U_INTERNAL_ASSERT_EQUALS(UStringExt::isCompress(posting->data()), false)
}

U_NO_EXPORT int UPosting::writePosting(int flag)
{
   U_TRACE(5, "UPosting::writePosting(%d)", flag)

   U_INTERNAL_ASSERT_POINTER(cdb_words)
   U_INTERNAL_ASSERT_EQUALS(UStringExt::isCompress(posting->data()), false)

#ifdef U_COMPRESS_ENTRY
   if (posting->size() > U_CAPACITY) posting->compress();
#endif

   int result = ((URDB*)cdb_words)->store(*word, *posting, flag);

   U_RETURN(result);
}

// FIND CURRENT DOC ID ON POSTING

inline bool UPosting::checkEntry(char* str, char* s, uint32_t n)
{
   U_TRACE(5, "UPosting::checkEntry(%p,%p,%u)", str, n)

   U_INTERNAL_ASSERT_POINTER(s)
   U_INTERNAL_ASSERT_POINTER(str)

   uint32_t offset = (str - s);

   U_INTERNAL_DUMP("offset     = %u", offset)
   U_INTERNAL_DUMP("size_entry = %u", POSTING_SIZE(str))

   if ((offset % sizeof(uint32_t)) == 0 &&
       POSTING_SIZE(str) <= (n - offset))
      {
      U_RETURN(true);
      }

   U_RETURN(false);
}

U_NO_EXPORT __pure char* UPosting::find(char* s, uint32_t n, bool boptmize)
{
   U_TRACE(5, "UPosting::find(%p,%u,%b)", s, n, boptmize)

   U_INTERNAL_DUMP("cur_doc_id = %llu", cur_doc_id)

   if (boptmize)
      {
      char* str;

      do {
         str = (char*) u_find(s, n, ptr_cur_doc_id, sizeof(cur_doc_id));

         if (str)
            {
            if (checkEntry(str, s, n)) U_RETURN(str);

            ++str;

            n = (s + n) - str;
            s =           str;
            }
         }
      while (str);
      }
   else
      {
      char* str = s;
      char* end = s + n;

      do {
         if (POSTING64(str,doc_id) == cur_doc_id)
            {
            U_INTERNAL_DUMP("offset     = %u",   (str - s))
            U_INTERNAL_DUMP("doc_id     = %llu", POSTING64(str,doc_id))
            U_INTERNAL_DUMP("word_freq  = %u",   POSTING32(str,word_freq))
            U_INTERNAL_DUMP("size_entry = %u",   POSTING_SIZE(str))

            U_RETURN(str);
            }

         str += POSTING_SIZE(str);
         }
      while (str < end);

      U_INTERNAL_ASSERT_EQUALS((str - s), (ptrdiff_t)n)
      }

   U_RETURN((char*)U_NULLPTR);
}

U_NO_EXPORT bool UPosting::setPosting(bool bcache)
{
   U_TRACE(5, "UPosting::setPosting(%b)", bcache)

   U_INTERNAL_ASSERT_POINTER(ptr_cur_doc_id)

   if (bcache)
      {
      static char* last_posting_ptr;
      static uint32_t last_posting_sz;
      static UStringRep* last_word_rep;

      if (word->rep != last_word_rep)
         {
         readPosting(last_word_rep = word->rep, false);

         if (posting->empty()) last_posting_sz  = 0;
         else
            {
            last_posting_sz  = posting->size() - sizeof(uint32_t); // offset last DOC id
            last_posting_ptr = posting->data() + sizeof(uint32_t);
            }
         }

      char* posting_ptr = (last_posting_sz ? find(last_posting_ptr, last_posting_sz, true) : U_NULLPTR);

      if (posting_ptr) U_RETURN(true);

      U_RETURN(false);
      }

   U_INTERNAL_ASSERT(*posting)

// if (posting->empty()) U_RETURN(false);

   ptr = find(posting->data() + sizeof(uint32_t), posting->size() - sizeof(uint32_t), false);

   U_INTERNAL_DUMP("ptr = %p", ptr)

   if (ptr) U_RETURN(true);

   U_RETURN(false);
}

U_NO_EXPORT bool UPosting::findCurrentDocIdOnPosting(UStringRep* value)
{
   U_TRACE(5, "UPosting::findCurrentDocIdOnPosting(%.*S)", U_STRING_TO_TRACE(*value))

   posting->_assign(value);

#ifdef U_COMPRESS_ENTRY
   (void)decompress();
#endif

   ptr = (char*) u_find(U_STRING_TO_PARAM(*posting), ptr_cur_doc_id, sizeof(cur_doc_id));

   // check for collision...

   if (ptr &&
       checkEntry(ptr, U_STRING_TO_PARAM(*posting)))
      {
      U_RETURN(true);
      }

   U_RETURN(false);
}

// DELETE WORD

inline bool UPosting::isOneEntry()
{
   U_TRACE(5, "UPosting::isOneEntry()")

   if (posting->size() == (sizeof(uint32_t) + POSTING_SIZE(ptr))) U_RETURN(true);

   U_RETURN(false);
}

U_NO_EXPORT void UPosting::del()
{
   U_TRACE(5, "UPosting::del()")

   U_INTERNAL_ASSERT(*word)
   U_INTERNAL_ASSERT(*posting)
   U_INTERNAL_ASSERT_POINTER(ptr)

   data = posting->data();

   U_INTERNAL_DUMP("word_freq       = %u",   POSTING32(ptr,word_freq))
   U_INTERNAL_DUMP("size_entry      = %u",   POSTING_SIZE(ptr))
   U_INTERNAL_DUMP("cur_doc_id      = %llu", cur_doc_id)
   U_INTERNAL_DUMP("posting->size() = %u",   posting->size())
   U_INTERNAL_DUMP("off_last_doc_id = %u",   POSTING_OFFSET_LAST_DOC_ID)

   U_INTERNAL_ASSERT_MAJOR(POSTING32(ptr,word_freq), 0)

   // check if other reference...

   int result;

   if (isOneEntry())
      {
      result = ((URDB*)cdb_words)->remove(*word);

      if (result != 0)
         {
         U_ERROR("del(): error<%d> on operation remove word<%.*s> from index database", result, U_STRING_TO_TRACE(*word));
         }

      return;
      }

#if defined(DEBUG) || defined(U_TEST)
   uint32_t last_offset = POSTING_OFFSET_LAST_DOC_ID;
#endif

   char* end = (char*) posting->pend();

   do {
      U_INTERNAL_DUMP("doc_id          = %lu", POSTING64(ptr,doc_id))

      size_entry = POSTING_SIZE(ptr);

      if (POSTING64(ptr,doc_id) == cur_doc_id) break;

      ptr += size_entry;
      }
   while (ptr < end);

   if (ptr >= end) U_ERROR("del(): cannot find DocID<%llu> reference for word<%.*s> on index database", cur_doc_id, U_STRING_TO_TRACE(*word));

   (void) posting->replace(ptr - data, size_entry, 0, '\0');

   // reassign value

   data = posting->data();
   ptr  = data + sizeof(uint32_t);
   end  = (char*) posting->pend();

   U_INTERNAL_DUMP("posting->size() = %u", posting->size())

   U_INTERNAL_ASSERT_EQUALS(POSTING_OFFSET_LAST_DOC_ID, last_offset)

   do {
      size_entry = POSTING_SIZE(ptr);

      ptr += size_entry;
      }
   while (ptr < end);

   U_INTERNAL_ASSERT_EQUALS(ptr - data, (ptrdiff_t)posting->size())

   u_put_unalignedp32(data, (ptr - data) - size_entry);

   U_INTERNAL_DUMP("off_last_doc_id = %u", POSTING_OFFSET_LAST_DOC_ID)

   U_INTERNAL_ASSERT_MINOR(POSTING_OFFSET_LAST_DOC_ID, last_offset)

   result = writePosting(RDB_REPLACE);

   if (result != 0) U_ERROR("del(): error<%d> on operation replace reference for word<%.*s> on index database", result, U_STRING_TO_TRACE(*word));
}

// ADD WORD

inline void UPosting::init()
{
   U_TRACE(5, "UPosting::init()")

   u_put_unalignedp64(&(((u_posting*)(ptr))->doc_id), cur_doc_id);
   u_put_unalignedp32(&(((u_posting*)(ptr))->word_freq), 0);
}

inline void UPosting::checkCapacity()
{
   U_TRACE(5, "UPosting::checkCapacity()")

   space = off_last_doc_id + size_entry + sizeof(off_last_doc_id);

   U_INTERNAL_DUMP("space = %u", space)

   if (space >= posting->capacity())
      {
      if (tbl_words) tbl_words_space -= posting->capacity();

   // if (posting->uniq() == false) posting->duplicate();

      posting->reserve(space * 2);

      if (tbl_words)
         {
         tbl_words_space += posting->capacity();

         tbl_words->replaceAfterFind(*posting);
         }
      }

   data = posting->data();
   ptr  = data + off_last_doc_id;
}

U_NO_EXPORT void UPosting::add()
{
   U_TRACE(5, "UPosting::add()")

   U_INTERNAL_ASSERT(*word)

   bool present = (tbl_words ?         tbl_words->find(*word)
                             : ((URDB*)cdb_words)->find(*word));

   if (present == false)
      {
      // first document for this word...

      *posting = UString(U_CAPACITY);
      data     = posting->data();
      ptr      = data + sizeof(off_last_doc_id);

      // setting off_last_doc_id for first document...

      size_entry      = sizeof(u_posting);
      off_last_doc_id = sizeof(off_last_doc_id);

      u_put_unalignedp32(data, sizeof(off_last_doc_id));

      init();

      if (tbl_words)
         {
         tbl_words_space += word->size() + posting->capacity();

         word->duplicate(); // NB: need duplicate string because depends on mmap()'s content of document...

         UHashMap<void*>::lkey = word->rep;

         tbl_words->insertAfterFind(*posting);
         }
      }
   else
      {
      if (tbl_words) posting->_assign(tbl_words->elem());
      else           readPosting(U_NULLPTR, false);

   // U_ASSERT_EQUALS(0,find(posting->data() + sizeof(uint32_t), posting->size() - sizeof(uint32_t)))

      data            = posting->data();
      off_last_doc_id = POSTING_OFFSET_LAST_DOC_ID;
      ptr             = data + off_last_doc_id;
      size_entry      = POSTING_SIZE(ptr);

      U_INTERNAL_DUMP("word_freq       = %u", POSTING32(ptr,word_freq))
      U_INTERNAL_DUMP("size_entry      = %u", size_entry)
      U_INTERNAL_DUMP("off_last_doc_id = %u", off_last_doc_id)

      U_INTERNAL_ASSERT_MAJOR(POSTING32(ptr,word_freq), 0)

      U_ASSERT((off_last_doc_id + size_entry) <= posting->capacity())

      // check for new document...

      if (POSTING64(ptr,doc_id) != cur_doc_id)
         {
         off_last_doc_id += size_entry;
               size_entry = sizeof(u_posting);

         u_put_unalignedp32(data, off_last_doc_id);

         // check string capacity...

         checkCapacity();

         init();
         }
      }

   U_INTERNAL_ASSERT_EQUALS(cur_doc_id,      POSTING64(ptr,doc_id))
   U_INTERNAL_ASSERT_EQUALS(off_last_doc_id, POSTING_OFFSET_LAST_DOC_ID)

   // check string capacity...

   checkCapacity();

   U_INTERNAL_ASSERT_EQUALS(cur_doc_id,      POSTING64(ptr,doc_id))
   U_INTERNAL_ASSERT_EQUALS(off_last_doc_id, POSTING_OFFSET_LAST_DOC_ID)

   // save position of word in document...

   word_freq = POSTING32(ptr,word_freq);

   u_put_unalignedp32(ptr+sizeof(u_posting)+(word_freq*sizeof(uint32_t)), pos);

   u_put_unalignedp32(&(((u_posting*)(ptr))->word_freq), word_freq+1);

   U_INTERNAL_DUMP("pos       = %u", pos)
   U_INTERNAL_DUMP("pos[0]    = %u", POSTING_POS(0))
   U_INTERNAL_DUMP("pos[1]    = %u", POSTING_POS(1))
   U_INTERNAL_DUMP("word_freq = %u", POSTING32(ptr,word_freq))

   posting->size_adjust_force(space);

   if (tbl_words == U_NULLPTR)
      {
      int result = writePosting(present ? RDB_REPLACE : RDB_INSERT);

      if (result != 0)
         {
         U_ERROR("add(): error<%d> on operation store reference for word<%.*s> on index database", result, U_STRING_TO_TRACE(*word));
         }
      }
}

// CHECK POSTING AND WORD

U_NO_EXPORT void UPosting::checkPosting()
{
   U_TRACE(5, "UPosting::checkPosting()")

   U_INTERNAL_ASSERT_POINTER(ptr)

   U_INTERNAL_DUMP("cur_doc_id      = %llu", cur_doc_id)

   U_INTERNAL_ASSERT_EQUALS(cur_doc_id, POSTING64(ptr,doc_id))

   word_freq = POSTING32(ptr,word_freq);

   U_INTERNAL_DUMP("word_freq       = %u", word_freq)
   U_INTERNAL_DUMP("size_entry      = %u", POSTING_SIZE(ptr))
   U_INTERNAL_DUMP("offset          = %u", posting->distance(ptr))
   U_INTERNAL_DUMP("posting->size() = %u", posting->size())

#if defined(U_OPTIMIZE) && defined(DEBUG)
   if (cdb_words->ignoreCase() == false)
      {
      U_INTERNAL_ASSERT_DIFFERS(u_find(content->data(), content->size(), word->data(), word->size()),0)
      }
#endif

   uint32_t end = content->size(),
             sz =    word->size();

   uint32_t* vpos = (uint32_t*)(ptr+sizeof(u_posting));

   char* ptr_word = word->data();
   char* ptr_data = content->data();

   for (uint32_t i = 0; i < word_freq; ++i)
      {
      uint32_t start = u_get_unalignedp32(vpos+i);

      U_INTERNAL_DUMP("start           = %u", start)

      if (start >= end ||
          u_equal(ptr_word, ptr_data+start, sz, cdb_words->ignoreCase()))
         {
         U_ERROR("checkPosting(): word<%v> reference at position<%u> for DocID<%llu %v> lost", word->rep, start, cur_doc_id, filename->rep);
         }
      }
}

// PROCESS WORD

void UPosting::processWord(int32_t op)
{
   U_TRACE(5, "UPosting::processWord(%d)", op)

   U_INTERNAL_ASSERT_POINTER(content)

   if (word->size() < min_word_size) return;

   pos = pos_start + (word->data() - content->data());

   U_INTERNAL_DUMP("word = %V pos = %u", word->rep, pos)

   if (op == 0 ||
       op == 1)
      {
      add(); // add/sub
      }
   else
      {
      readPosting(word->rep, (op == 2)); // del

      if (posting->empty() ||
          setPosting(false) == false)
         {
         U_ERROR("processWord(%d): word<%v> reference at position<%u> for DocID<%llu %v> lost", op, word->rep, pos, cur_doc_id, filename->rep);
         }

      if      (op == 2) del();          // del
      else if (op == 3) checkPosting(); // check
      }
}

// CHECK DOCUMENT

U_NO_EXPORT int UPosting::checkDocument(UStringRep* word_rep, UStringRep* value)
{
   U_TRACE(5, "UPosting::checkDocument(%.*S,%p)", U_STRING_TO_TRACE(*word_rep), value)

   U_INTERNAL_ASSERT(*content)

   if (findCurrentDocIdOnPosting(value))
      {
      word->_assign(word_rep);

      U_INTERNAL_DUMP("cdb_words->ignoreCase() = %b", cdb_words->ignoreCase())

#  ifndef U_OPTIMIZE
      if (cdb_words->ignoreCase() == false &&
          u_find(content->data(), content->size(), word->data(), word->size()) == U_NULLPTR)
         {
         U_RETURN(1);
         }
#  endif

      checkPosting();
      }

   U_RETURN(1);
}

U_NO_EXPORT int UPosting::checkAllEntry(UStringRep* word_rep, UStringRep* value)
{
   U_TRACE(5, "UPosting::checkAllEntry(%.*S,%p)", U_STRING_TO_TRACE(*word_rep), value)

   word->_assign(word_rep);

   callForPostingAndSetFilename(U_NULLPTR);

   (void) write(1, U_CONSTANT_TO_PARAM(".")); // CHECK_2

   U_RETURN(1);
}

void UPosting::checkAllEntry()
{
   U_TRACE(5, "UPosting::checkAllEntry()")

   cdb_words->callForAllEntryWithPattern(checkAllEntry, U_NULLPTR);
}

// SUBSTITUTE DOCUMENT

U_NO_EXPORT int UPosting::substitute(UStringRep* word_rep, UStringRep* value)
{
   U_TRACE(5, "UPosting::substitute(%.*S,%p)", U_STRING_TO_TRACE(*word_rep), value)

   posting->_assign(value);

#ifdef U_COMPRESS_ENTRY
   (void) decompress();
#endif

   word->_assign(word_rep);

   if (setPosting(false)) del();

   U_RETURN(1);
}

// PROCESS DOCUMENT

inline void UPosting::setDocID(bool from_inode)
{
   U_TRACE(5, "UPosting::setDocID(%b)", from_inode)

   if (from_inode) cur_doc_id = - file->inode();

   U_INTERNAL_DUMP("cur_doc_id = %llu", cur_doc_id)

   str_cur_doc_id->setFromInode(&cur_doc_id);

   ptr_cur_doc_id = str_cur_doc_id->data();
}

void UPosting::setDocID(int32_t op)
{
   U_TRACE(5, "UPosting::setDocID(%d)", op)

   U_DUMP("filename = %.*S", U_STRING_TO_TRACE(*filename))

   if (dir_content_as_doc)
      {
      if (op != 0) // add
         {
         U_ERROR("setDocID(%d): sorry, not implemented", op);
         }

      if (change_dir == false) return;

      change_dir = false;

      *filename = UStringExt::dirname(*filename);

      filename->duplicate(); // NB: need duplicate string because depends on volatile buffer of filename...

      U_INTERNAL_DUMP("dirname = %V", filename->rep)
      }

   // insert/fetch/remove into table of docs name

   setDocID(true);

   if (tbl_name)
      {
      tbl_name->insert(*str_cur_doc_id, *filename);

      tbl_name_space += str_cur_doc_id->size() + filename->size();
      }
   else
      {
      int result = 0;

      if (op == 0) result = ((URDB*)cdb_names)->store( *str_cur_doc_id, *filename, RDB_INSERT); // add
      if (op == 2) result = ((URDB*)cdb_names)->remove(*str_cur_doc_id);                        // del

      if (result != 0)
         {
         U_ERROR("setDocID(%d): error<%d> for operation<%d> on DocID database", op, result, op);
         }

      if (op == 1) // sub
         {
         if (((URDB*)cdb_names)->find(*str_cur_doc_id) == false)
            {
            U_ERROR("setDocID(%d): cannot find DocID<%llu> on names database", op, cur_doc_id);
            }

         // SUBSTITUTE: we find all reference for this document in the words database and erase it...

         U_INTERNAL_ASSERT_POINTER(cdb_words)

         ((URDB*)cdb_words)->callForAllEntryWithPattern(substitute, U_NULLPTR);
         }
      else if (op == 3) // check
         {
         // CHECK: we find all reference for this document in the words database and check it...

         U_INTERNAL_ASSERT_POINTER(cdb_words)

         cdb_words->callForAllEntryWithPattern(checkDocument, U_NULLPTR);
         }
      }
}

// PROCESSING MISC (QUERY)

void UPosting::callForPosting(vPF function)
{
   U_TRACE(5, "UPosting::callForPosting(%p)", function)

   U_INTERNAL_ASSERT(*posting)
   U_INTERNAL_ASSERT_POINTER(function)

   data      = posting->data();
   ptr       = data + sizeof(uint32_t);
   char* end = (char*) posting->pend();

   do {
      word_freq  = POSTING32(ptr,word_freq);
      size_entry = POSTING_SIZE(ptr);
      cur_doc_id = POSTING64(ptr,doc_id);

      U_INTERNAL_DUMP("word_freq  = %u",   word_freq)
      U_INTERNAL_DUMP("size_entry = %u",   size_entry)
      U_INTERNAL_DUMP("cur_doc_id = %llu", cur_doc_id)

      function();

      ptr += size_entry;
      }
   while (ptr < end);

   U_INTERNAL_ASSERT_EQUALS(ptr - data, (ptrdiff_t)posting->size())
}

U_NO_EXPORT void UPosting::setFilename()
{
   U_TRACE(5, "UPosting::setFilename()")

   U_INTERNAL_ASSERT(*str_cur_doc_id)
   U_INTERNAL_ASSERT_POINTER(cdb_names)

   *filename = (*cdb_names)[str_cur_doc_id->rep];

   if (filename->empty())
      {
      U_ERROR("setFilename(): cannot find document name from DocID<%llu> reference for word<%v> on index database", cur_doc_id, word->rep);
      }
}

U_NO_EXPORT void UPosting::callForPostingAndSetFilename()
{
   U_TRACE(5, "UPosting::callForPostingAndSetFilename()")

   str_cur_doc_id->setFromInode(&cur_doc_id);

   setFilename();

   if (pfunction) pfunction();
}

void UPosting::callForPostingAndSetFilename(vPF function)
{
   U_TRACE(5, "UPosting::callForPostingAndSetFilename(%p)", function)

   pfunction = function;

   callForPosting(callForPostingAndSetFilename);
}

U_NO_EXPORT bool UPosting::setVectorCompositeWord()
{
   U_TRACE(5, "UPosting::setVectorCompositeWord()")

   U_INTERNAL_ASSERT(*word)
   U_INTERNAL_ASSERT_POINTER(cdb_words)
   U_INTERNAL_ASSERT_EQUALS(vec_sub_word,U_NULLPTR)
   U_INTERNAL_ASSERT_EQUALS(vec_sub_word_posting,U_NULLPTR)

   U_NEW_STRING(sub_word, UString);
   U_NEW(UVector<UString>, vec_sub_word, UVector<UString>(*word));
   U_NEW(UVector<UString>, vec_sub_word_posting, UVector<UString>);

   vec_sub_word_size = vec_sub_word->size();

   for (uint32_t i = 0; i < vec_sub_word_size; ++i)
      {
      *sub_word = (*vec_sub_word)[i];

      U_INTERNAL_DUMP("sub_word = %V", sub_word->rep)

      if (sub_word->size() < min_word_size)
         {
         vec_sub_word_posting->push(UString::getStringNull());

         continue;
         }

      readPosting(sub_word->rep, false);

      if (posting->empty())
         {
         resetVectorCompositeWord();

         U_RETURN(false);
         }

      vec_sub_word_posting->push(*posting);
      }

   U_RETURN(true);
}

inline bool UPosting::setSubWord(uint32_t i)
{
   U_TRACE(5, "UPosting::setSubWord(%u)", i)

   U_INTERNAL_ASSERT_POINTER(sub_word)
   U_INTERNAL_ASSERT_POINTER(vec_sub_word)

   *sub_word      = (*vec_sub_word)[i];
    sub_word_size = sub_word->size();

   U_INTERNAL_DUMP("sub_word = %V", sub_word->rep)

   if (sub_word_size < min_word_size)
      {
      sub_word_pos_prev += 1 + sub_word_size;

      U_RETURN(false);
      }

   U_INTERNAL_DUMP("sub_word_pos_prev = %u", sub_word_pos_prev)

   *posting = (*vec_sub_word_posting)[i];

   U_INTERNAL_ASSERT(*posting)

   U_RETURN(true);
}

inline UString UPosting::extractDocID()
{
   U_TRACE(5, "UPosting::extractDocID()")

   /**
    * +------+--------------------+--------+-----------+-------+-----+-------+-----+--------+-----------+-------+-----+-------+
    * | WORD | offset last DOC id | DOC id | frequency | pos 1 | ... | pos n | ... | DOC id | frequency | pos 1 | ... | pos n |
    * +------+--------------------+--------+-----------+-------+-----+-------+-----+--------+-----------+-------+-----+-------+
    */

   UStringRep* r;
   UString s(size_entry);

   char* sdata = s.data();
   char* sptr  = sdata;

   for (uint32_t i = 0, n = vec_entry->size(); i < n; ++i)
      {
      r = vec_entry->UVector<UStringRep*>::at(i);

#  ifdef U_COMPRESS_ENTRY
      posting->_assign(r);

      if (decompress()) r = posting->rep;
#  endif

      data      = r->data();
      ptr       = data + sizeof(uint32_t);
      char* end = r->pend();

      do {
         size_entry = POSTING_SIZE(ptr);
         cur_doc_id = POSTING64(ptr,doc_id);

         U_INTERNAL_DUMP("word_freq  = %u",   POSTING32(ptr,word_freq))
         U_INTERNAL_DUMP("size_entry = %u",   size_entry)
         U_INTERNAL_DUMP("cur_doc_id = %llu", cur_doc_id)

         U_MEMCPY((void*)sptr, &cur_doc_id, sizeof(cur_doc_id));

          ptr += size_entry;
         sptr += sizeof(uint64_t);
         }
      while (ptr < end);

      U_INTERNAL_ASSERT_EQUALS(ptr - data, (ptrdiff_t)r->size())
      }

   s.size_adjust_force(sptr - sdata);

   U_RETURN_STRING(s);
}

/**
 * ------------------------------------------------
 * findDocID() is be called from:
 * ------------------------------------------------
 * UQueryParser::evaluate() (loop for all doc name)
 * ------------------------------------------------
 */

bool UPosting::findDocID(UStringRep* word_rep)
{
   U_TRACE(5, "UPosting::findDocID(%V)", word_rep)

   U_INTERNAL_ASSERT(*str_cur_doc_id)
   U_INTERNAL_ASSERT_POINTER(cdb_words)

   // manage meta expr, composite word, etc. for all doc name... (save info with property)

   uint32_t i;

   word->_assign(word_rep);

   if (vec_word) i = vec_word->find(*word);
   else
      {
      // allocation property...

      i = U_NOT_FOUND;

      U_NEW(UVector<UString>, vec_word, UVector<UString>(32));
      U_NEW(UVector<UString>, vec_entry, UVector<UString>(approximate_num_words));
      U_NEW(UVector<UString>, vec_posting, UVector<UString>(32));
      }

   // check if exist property for this word...

   if (i == U_NOT_FOUND)
      {
      // set property for this word...

      i = vec_word->size();

      vec_word->push(*word);

      if (word_rep->isQuoted('"'))
         {
         property[i].is_meta   = false;
         property[i].is_quoted = true;
         }
      else if (word->find_first_of("?*", 0, 2) != U_NOT_FOUND)
         {
         property[i].is_meta   = true;
         property[i].is_quoted = false;
         }
      else
         {
         property[i].is_meta = property[i].is_quoted = false;
         }
      }

   // find context with property...

   if (property[i].is_quoted)
      {
      // context composite word...

      static UStringRep* last_word_rep;

      if (word_rep != last_word_rep)
         {
         last_word_rep = word_rep;

         if (vec_sub_word) resetVectorCompositeWord();
         }

      if (callForCompositeWord(U_NULLPTR)) U_RETURN(true);

      U_RETURN(false);
      }

   if (property[i].is_meta)
      {
      // context meta word...

      UString entry;

      if (i < vec_posting->size()) entry = (*vec_posting)[i];
      else
         {
         vec_entry->clear();

         if (cdb_words->getValuesWithKeyNask(*vec_entry, *word, &size_entry)) entry = extractDocID();

         vec_posting->push(entry);
         }

      if (entry &&
          u_find(entry.data(), entry.size(), ptr_cur_doc_id, sizeof(cur_doc_id)))
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

   // context single word...

   if (setPosting(true)) U_RETURN(true);

   U_RETURN(false);
}

void UPosting::callForPosting(vPF function, bool is_space)
{
   U_TRACE(5, "UPosting::callForPosting(%p,%b)", function, is_space)

   U_INTERNAL_ASSERT(*word)
   U_INTERNAL_ASSERT_POINTER(cdb_words)

   if (is_space) // composite word
      {
      // check for NEAR operation (string not quoted)...

      bool is_quoted = word->isQuoted();

      U_INTERNAL_DUMP("is_quoted = %b", is_quoted)

      if (is_quoted == false) max_distance = ((uint32_t)-1);

      (void) callForCompositeWord(function);
      }
   else // single word
      {
      readPosting(word->rep, false);

      if (*posting) callForPostingAndSetFilename(function);
      }
}

/**
 * ------------------------------------------------
 * CONTEXT COMPOSITE WORD...
 * ------------------------------------------------
 * checkCompositeWord() may be called from:
 * ------------------------------------------------
 * UQueryParser::evaluate() (loop for all doc name)
 * callForPosting()         (for composite word)
 * ------------------------------------------------
 */

U_NO_EXPORT bool UPosting::callForCompositeWord(vPF function)
{
   U_TRACE(5, "UPosting::callForCompositeWord(%p)", function)

   U_INTERNAL_ASSERT_POINTER(cdb_names)

   if (vec_sub_word == U_NULLPTR &&
       setVectorCompositeWord() == false)
      {
      U_RETURN(false);
      }

   // find the first sub-word util

   uint32_t first_subword_index;

   for (first_subword_index = 0; first_subword_index < vec_sub_word_size; ++first_subword_index)
      {
      if (setSubWord(first_subword_index) == false) continue;

      break;
      }

   U_INTERNAL_DUMP("FIRST subword util  = %.*S", U_STRING_TO_TRACE(*sub_word))
   U_INTERNAL_DUMP("first_subword_index = %u", first_subword_index)

   U_INTERNAL_ASSERT(*posting)

#if defined(DEBUG) || defined(U_TEST)
   uint32_t posting_size = posting->size();
#endif

   bool match;
   char* ptr1;
   char* end1 = U_NULLPTR;
   int32_t i, first_subword_freq;
   uint32_t j, k, sz = sub_word_size;

   // loop for all posting entry of the first sub-word util
   // ---------------------------------------------------------------------------------------------------------------
   // immatricolazione (-261845,1,1148)(-261811,1,2024)(-261819,4,2037,2624,4075,4567)(-261832,4,2037,2624,4479,4971)
   // ---------------------------------------------------------------------------------------------------------------

   if (function == U_NULLPTR)
      {
      if (setPosting(false) == false) U_RETURN(false);

      ptr1 = ptr;

      goto start;
      }

   data = posting->data();
   ptr1 = data + sizeof(uint32_t);
   end1 = (char*) posting->pend();

   do {
      ptr        = ptr1;
      cur_doc_id = POSTING64(ptr,doc_id);
      size_entry = POSTING_SIZE(ptr);

      U_INTERNAL_DUMP("cur_doc_id          = %llu", cur_doc_id)
      U_INTERNAL_DUMP("size_entry          = %u",   size_entry)

      setDocID(false);

start:
      first_subword_freq = POSTING32(ptr,word_freq);

      U_INTERNAL_DUMP("first_subword_freq  = %u", first_subword_freq)

      U_INTERNAL_ASSERT_MAJOR(first_subword_freq, 0)

      i     = -1;
      match = false;

      // loop for all the position of first sub-word util for this document...

loop1:
      while (++i < first_subword_freq)
         {
         ptr               = ptr1;
         pos               = POSTING_POS(i);
         sub_word_pos_prev = pos + sz;

         U_INTERNAL_DUMP("pos[%3u]           = %u", i, pos)
         U_INTERNAL_DUMP("sub_word_pos_prev  = %u", sub_word_pos_prev)

         // check if the other sub-word match in the current doc...

         for (j = first_subword_index + 1; j < vec_sub_word_size; ++j)
            {
            if (setSubWord(j) == false) continue;

            if (setPosting(false) == false) goto loop1;

            U_INTERNAL_DUMP("subword            = %.*S", U_STRING_TO_TRACE(*sub_word))

            U_INTERNAL_ASSERT_EQUALS(cur_doc_id, POSTING64(ptr,doc_id))

            word_freq = POSTING32(ptr,word_freq);

            U_INTERNAL_DUMP("word_freq          = %u", word_freq)

            U_INTERNAL_ASSERT_MAJOR(word_freq, 0)

            // check for all position for this sub-word in the current doc...

            for (k = 0; k < word_freq; ++k)
               {
               pos      = POSTING_POS(k);
               distance = pos - sub_word_pos_prev;

               U_INTERNAL_DUMP("pos[%3u]           = %u", k, pos)
               U_INTERNAL_DUMP("sub_word_pos_prev  = %u", sub_word_pos_prev)
               U_INTERNAL_DUMP("distance           = %u", distance)
               U_INTERNAL_DUMP("max_distance       = %u", max_distance)

               if (distance <= max_distance) break;
               }

            if (k == word_freq) goto loop1; // NOT match, go to another position for the first sub-word util...

            sub_word_pos_prev = pos + sub_word_size;
            }

         match = true;
         }

      U_INTERNAL_ASSERT_EQUALS(i, first_subword_freq)

      if (function == U_NULLPTR) U_RETURN(match);

      if (match)
         {
         setFilename();

         word_freq = (max_distance == ((uint32_t)-1) // check for NEAR...
                              ? 0
                              : max_distance - distance);

         function();
         }

      ptr1 += size_entry;
      }
   while (ptr1 < end1);

   U_INTERNAL_ASSERT_EQUALS(ptr1 - data, (ptrdiff_t)posting_size)

   U_RETURN(true);
}

// PRINT DATABASE

static ostream* os;
static UString* buffer;

#  define SIZE_ENTRY ( 2 + \
                      10 + \
                      10 + \
                      10 * word_freq)

U_NO_EXPORT void UPosting::printPosting()
{
   U_TRACE(5, "UPosting::printPosting()")

   static UStringRep* last = word->rep;

start:
   if (last == word->rep)
      {
      if (buffer->empty())
         {
#     ifdef U_STDCPP_ENABLE
         *os << "-------------------------------------------------------------------------------------------\n";

         last->write(*os);
#     endif
         }

      uint32_t* vpos = (uint32_t*)(ptr+sizeof(u_posting));

      if (buffer->space() < SIZE_ENTRY) buffer->reserve(buffer->capacity() * 2);

                                               buffer->snprintf_add(U_CONSTANT_TO_PARAM("(%llX,%u,%u"), cur_doc_id, word_freq, u_get_unalignedp32(vpos));
      for (uint32_t i = 1; i < word_freq; ++i) buffer->snprintf_add(U_CONSTANT_TO_PARAM(",%u"), u_get_unalignedp32(vpos+i));
                                        (void) buffer->append(U_CONSTANT_TO_PARAM(")"));
      }
   else
      {
#  ifdef U_STDCPP_ENABLE
      *os << ' ' << *buffer << '\n';
#  endif

      last = word->rep;

      buffer->setEmpty();

      goto start;
      }
}

U_NO_EXPORT int UPosting::print(UStringRep* word_rep, UStringRep* value)
{
   U_TRACE(5, "UPosting::print(%.*S,%p)", U_STRING_TO_TRACE(*word_rep), value)

   posting->_assign(value);

#ifdef U_COMPRESS_ENTRY
   (void) decompress();
#endif

   word->_assign(word_rep);

   callForPosting(printPosting);

   U_RETURN(1);
}

U_NO_EXPORT int UPosting::printDocName(UStringRep* doc_id, UStringRep* doc_name)
{
   U_TRACE(5, "UPosting::printDocName(%.*S,%.*S)", U_STRING_TO_TRACE(*doc_id), U_STRING_TO_TRACE(*doc_name))

#ifdef U_STDCPP_ENABLE
   char _buffer[20];

   os->write(_buffer, u__snprintf(_buffer, sizeof(_buffer), U_CONSTANT_TO_PARAM("%llX "), *((uint64_t*)(doc_id->data()))));

   doc_name->write(*os);

   os->put('\n');
#endif

   U_RETURN(1);
}

void UPosting::printDB(ostream& s)
{
   U_TRACE(5, "UPosting::printDB(%p)", &s)

#ifdef U_STDCPP_ENABLE
   os = &s;

   U_NEW_STRING(buffer, UString(U_CAPACITY));

   if (tbl_words) tbl_words->callForAllEntry((bPFprpv)print);
   else
      {
      U_INTERNAL_ASSERT_POINTER(cdb_names)
      U_INTERNAL_ASSERT_POINTER(cdb_words)

      cdb_names->callForAllEntryWithPattern(printDocName, U_NULLPTR);
      cdb_words->callForAllEntryWithPattern(print, U_NULLPTR);
      }

   *os << ' ' << *buffer
       << "\n-------------------------------------------------------------------------------------------\n";

   os->flush();

   U_DELETE(buffer)
#endif
}

// DEBUG

#if defined(DEBUG) && defined(U_STDCPP_ENABLE)
const char* UPosting::dump(bool _reset) const
{
   *UObjectIO::os << "pos                                     " << pos                          << '\n'
                  << "ptr                                     " << (void*)ptr                   << '\n'
                  << "data                                    " << (void*)data                  << '\n'
                  << "space                                   " << space                        << '\n'
                  << "distance                                " << distance                     << '\n'
                  << "pfunction                               " << (void*)pfunction             << '\n'
                  << "pos_start                               " << pos_start                    << '\n'
                  << "size_entry                              " << size_entry                   << '\n'
                  << "word_freq                               " << word_freq                    << '\n'
                  << "cur_doc_id                              " << cur_doc_id                   << '\n'
                  << "max_distance                            " << max_distance                 << '\n'
                  << "min_word_size                           " << min_word_size                << '\n'
                  << "sub_word_size                           " << sub_word_size                << '\n'
                  << "tbl_name_space                          " << tbl_name_space               << '\n'
                  << "ptr_cur_doc_id                          " << (void*)ptr_cur_doc_id        << '\n'
                  << "tbl_words_space                         " << tbl_words_space              << '\n'
                  << "off_last_doc_id                         " << off_last_doc_id              << '\n'
                  << "vec_sub_word_size                       " << vec_sub_word_size            << '\n'
                  << "sub_word_pos_prev                       " << sub_word_pos_prev            << '\n'
                  << "file                 (UFile             " << (void*)file                  << ")\n"
                  << "word                 (UString           " << (void*)word                  << ")\n"
                  << "content              (UString           " << (void*)content               << ")\n"
                  << "posting              (UString           " << (void*)posting               << ")\n"
                  << "filename             (UString           " << (void*)filename              << ")\n"
                  << "sub_word             (UString           " << (void*)sub_word              << ")\n"
                  << "tbl_name             (UHashMap<UString> " << (void*)tbl_name              << ")\n"
                  << "tbl_words            (UHashMap<UString> " << (void*)tbl_words             << ")\n"
                  << "vec_sub_word         (UVector<UString>  " << (void*)vec_sub_word          << ")\n"
                  << "str_cur_doc_id       (UString           " << (void*)str_cur_doc_id        << ")\n"
                  << "vec_sub_word_posting (UVector<UString>  " << (void*)vec_sub_word_posting  << ')';

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return U_NULLPTR;
}
#endif
