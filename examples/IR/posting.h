// posting.h - class support for inverted index (data structure)

#ifndef POSTING_H
#define POSTING_H 1

#include <ulib/string.h>

                   class UCDB;
                   class UFile;
template <class T> class UVector;
template <class T> class UHashMap;

/*
+------+--------------------+--------+-----------+-------+-----+-------+-----+--------+-----------+-------+-----+-------+
| WORD | offset last DOC id | DOC id | frequency | pos 1 | ... | pos n | ... | DOC id | frequency | pos 1 | ... | pos n |
+------+--------------------+--------+-----------+-------+-----+-------+-----+--------+-----------+-------+-----+-------+
*/

class U_EXPORT UPosting {
public:

   // uint32_t off_last_doc_id;
   typedef struct u_posting {
      uint64_t doc_id;
      uint32_t word_freq;
      // -----> array of unsigned with word_freq elements
   } u_posting;

   // Check Memory
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   static UFile* file;
   static UString* word;
   static UString* content;
   static UString* posting;
   static UString* filename;
   static UString* str_cur_doc_id;
   static UHashMap<UString>* tbl_name;
   static UHashMap<UString>* tbl_words;
   static bool ignore_case, dir_content_as_doc, change_dir;
   static uint32_t word_freq, tbl_words_space, min_word_size, max_distance, pos_start;

   // COSTRUTTORE

    UPosting(uint32_t dimension, bool parsing, bool index);
   ~UPosting();

   // SERVICES

   static void reset();

   static void setDocID(int32_t op);

   static void setDocID(UStringRep* str_inode)
      {
      U_TRACE(5, "UPosting::setDocID(%#.*S)", U_STRING_TO_TRACE(*str_inode))

      str_cur_doc_id->_assign(str_inode);

      ptr_cur_doc_id = str_cur_doc_id->data();

      cur_doc_id = *((uint64_t*)(str_inode->data()));

      U_INTERNAL_DUMP("cur_doc_id = %llu", cur_doc_id)

      U_INTERNAL_ASSERT_DIFFERS(cur_doc_id,0)
      }

   static void processWord(int32_t op);

   // Call function for all/one entry

   static void printDB(ostream& os);
   static bool findDocID(UStringRep* word_rep);

   static void checkAllEntry();
   static void callForPosting(vPF function);
   static void callForPostingAndSetFilename(vPF function);
   static void callForPosting(vPF function, bool is_space);

   // DEBUG

#ifdef DEBUG
   const char* dump(bool reset) const;
#endif

protected:
   static vPF pfunction;
   static const char* ptr;
   static const char* data;
   static UString* sub_word;
   static uint64_t cur_doc_id;
   static const char* ptr_cur_doc_id;
   static UVector<UString>* vec_word;
   static UVector<UString>* vec_entry;
   static UVector<UString>* vec_posting;
   static UVector<UString>* vec_sub_word;
   static UVector<UString>* vec_sub_word_posting;
   static uint32_t pos, size_entry, space, off_last_doc_id, distance,
                   vec_sub_word_size, sub_word_size, sub_word_pos_prev, approximate_num_words;

private:
   static inline void    init() U_NO_EXPORT;
   static inline bool    decompress() U_NO_EXPORT;
   static inline bool    isOneEntry() U_NO_EXPORT;
   static inline UString extractDocID() U_NO_EXPORT;
   static inline bool    setSubWord(uint32_t i) U_NO_EXPORT;
   static inline void    setDocID(bool from_inode) U_NO_EXPORT;
   static inline bool    checkEntry(const char* str, const char* s, uint32_t n) U_NO_EXPORT;
   static const char*    find(const char* s, uint32_t n, bool boptmize) U_NO_EXPORT __pure;
   static       void     add() U_NO_EXPORT; // op 0
   static       void     del() U_NO_EXPORT; // op 2
   static       void     checkWord() U_NO_EXPORT;
   static       void     setFilename() U_NO_EXPORT;
   static       void     printPosting() U_NO_EXPORT;
   static       void     checkPosting() U_NO_EXPORT;
   static       void     checkCapacity() U_NO_EXPORT;
   static       int      writePosting(int flag) U_NO_EXPORT;
   static       bool     setPosting(bool bcache) U_NO_EXPORT;
   static       bool     setVectorCompositeWord() U_NO_EXPORT;
   static       void     resetVectorCompositeWord() U_NO_EXPORT;
   static       void     callForPostingAndSetFilename() U_NO_EXPORT;
   static       bool     callForCompositeWord(vPF function) U_NO_EXPORT;
   static       void     readPosting(UStringRep* word_rep, bool flag) U_NO_EXPORT;
   static       bool     findCurrentDocIdOnPosting(UStringRep* value) U_NO_EXPORT;
   static       int      print(UStringRep* word_rep, UStringRep* value) U_NO_EXPORT;
   static       int      substitute(UStringRep* word_rep, UStringRep* value) U_NO_EXPORT;
   static       int      checkAllEntry(UStringRep* word_rep, UStringRep* value) U_NO_EXPORT;
   static       int      checkDocument(UStringRep* word_rep, UStringRep* value) U_NO_EXPORT;
   static       int      printDocName(UStringRep* doc_id, UStringRep* doc_name) U_NO_EXPORT;

   // Forbidden operations

   UPosting(const UPosting&)            {}
   UPosting& operator=(const UPosting&) { return *this; }
};

#endif
