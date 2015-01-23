// IR.cpp

#include <ulib/utility/dir_walk.h>
#include <ulib/utility/string_ext.h>

#include "IR.h"

/*
inverted index: (data structure)

Definition: An index into a set of texts of the words in the texts. The index is accessed by some search method.
Each index entry gives the word and a list of texts, possibly with locations within the text, where the word occurs.
See also full inverted index, inverted file index, block addressing index, index file, external index, forward index.

Note: Suppose we want to search the texts "i love you," "god is love," "love is blind," and "blind justice."
(The words of the text are all lower case for simplicity) If we index by (text, character within the text),
the index with location in text is:

 blind   (3,8);(4,0)
 god     (2,0)
 i       (1,0)
 is      (2,4);(3,5)
 justice (4,6)
 love    (1,2);(2,7);(3,0)
 you     (1,7)

The word "blind" is in document 3 ("love is blind") starting at character 8, so has an entry (3,8).
To find, for instance, documents with both "is" and "love," first look up the words in the index,
then find the intersection of the texts in each list. In this case, documents 2 and 3 have both words.
We can quickly find documents where the words appear close to each other by comparing the character within the text
*/

UCDB* cdb_names;
UCDB* cdb_words;

int32_t              IR::operation; // 0 -> add, 1 -> sub, 2 -> del, 3 -> check
UPosting*            IR::posting;
UTokenizer*          IR::t;
UString*             IR::bad_words;
UVector<UString>*    IR::filter_ext;
UVector<UString>*    IR::suffix_bad_words;
UVector<UString>*    IR::suffix_skip_tag_xml;
UVector<UCommand*>*  IR::filter_cmd;

IR::~IR()
{
   U_TRACE(5, "IR::~IR()")

   if (t) delete t;

   if (filter_ext)
      {
      delete filter_cmd;
      delete filter_ext;
      }

   if (posting)             delete posting;
   if (bad_words)           delete bad_words;
   if (suffix_bad_words)    delete suffix_bad_words;
   if (suffix_skip_tag_xml) delete suffix_skip_tag_xml;
}

void IR::setBadWords()
{
   U_TRACE(5, "IR::setBadWords()")

   if (t == 0) t = new UTokenizer;

   cfg_skip_tag_xml = cfg[U_STRING_FROM_CONSTANT("SKIP_TAG_XML")];

   if (cfg_skip_tag_xml) suffix_skip_tag_xml = new UVector<UString>(cfg_skip_tag_xml);

   cfg_bad_words = cfg[U_STRING_FROM_CONSTANT("BAD_WORDS")];

   if (cfg_bad_words)
      {
      bad_words = U_NEW(UString(cfg_bad_words)); 

      u_setPfnMatch(U_DOSMATCH_WITH_OR, UPosting::ignore_case);

      cfg_bad_words_ext = cfg[U_STRING_FROM_CONSTANT("BAD_WORDS_EXT")];

      if (cfg_bad_words_ext) suffix_bad_words = new UVector<UString>(cfg_bad_words_ext);
      }
}

void IR::parse()
{
   U_TRACE(5, "IR::parse()")

   U_INTERNAL_ASSERT_POINTER(t)

   UPosting::file->setPath(*UPosting::filename);

   uint32_t i;
   UString suffix = UPosting::file->getSuffix();

   if (filter_ext &&
       (i = filter_ext->find(suffix), i != U_NOT_FOUND))
      {
#  ifdef _MSWINDOWS_
      (void) UPosting::file->open();
      (void) UPosting::file->size(true);
      (void) UPosting::file->close();
#  else
      (void) UPosting::file->stat();
#  endif

      UPosting::content->clear();
      UPosting::content->reserve(UPosting::file->getSize());

      (void) ((*filter_cmd)[i])->executeWithFileArgument(UPosting::content, UPosting::file);
      }
   else
      {
      *UPosting::content = UPosting::file->getContent(true, true);
      }

   UPosting::setDocID(operation); // insert/fetch/remove into table of docs name

   // loop for all words in document

   UPosting::word->clear(); // depend on content...

   t->setData(*UPosting::content);

   t->setAvoidPunctuation(true);

   bool bad_words_active = bad_words &&
                           (suffix_bad_words == 0 ||
                            suffix_bad_words->find(suffix) != U_NOT_FOUND);

   if (suffix_skip_tag_xml) t->setSkipTagXML(suffix_skip_tag_xml->find(suffix) != U_NOT_FOUND);

   while (t->next(*UPosting::word, (bool*)0))
      {
      if (bad_words_active &&
          UServices::match(*UPosting::word, *bad_words))
         {
         continue;
         }

      UPosting::processWord(operation);
      }

   if (operation == 2) UPosting::file->_unlink(); // del
}

void IR::processFile()
{
   U_TRACE(5, "IR::processFile()")

   U_INTERNAL_ASSERT_EQUALS(UDirWalk::isDirectory(), false)

   UDirWalk::setFoundFile(*UPosting::filename);

   IR::parse();

   if (operation == 3) (void) write(1, U_CONSTANT_TO_PARAM(".")); // check

   // adjust virtual position if context 'directory as document'...

   if (UPosting::dir_content_as_doc) UPosting::pos_start += UPosting::content->size();
}

void IR::processDirectory()
{
   U_TRACE(5, "IR::processDirectory()")

   UPosting::pos_start  = 0;
   UPosting::change_dir = true;
}

void IR::loadFiles()
{
   U_TRACE(5, "IR::loadFiles()")

   UDirWalk dirwalk;

   UDirWalk::setSortingForInode();
   UDirWalk::setRecurseSubDirs(false);

                                     dirwalk.call_internal = IR::processFile;
   if (UPosting::dir_content_as_doc) dirwalk.call_if_up    = IR::processDirectory;

   if (operation == 3) (void) write(1, U_CONSTANT_TO_PARAM("CHECK_1")); // check

   dirwalk.walk();

   if (operation == 3) (void) write(1, U_CONSTANT_TO_PARAM("OK")); // check

   (void) UFile::chdir(0, true);
}

void IR::loadFilters()
{
   U_TRACE(5, "IR::loadFilters()")

   cfg_filter_ext = cfg[U_STRING_FROM_CONSTANT("FILTER_EXT")];

   if (cfg_filter_ext)
      {
      cfg_filter_cmd = cfg[U_STRING_FROM_CONSTANT("FILTER_CMD")];

      if (cfg_filter_cmd)
         {
         filter_cmd = new UVector<UCommand*>;
         filter_ext = new UVector<UString>(cfg_filter_ext);

         UVector<UString> filter_str(cfg_filter_cmd);

         U_ASSERT_EQUALS(filter_ext->size(),filter_str.size())

         for (uint32_t i = 0; i < filter_str.size(); ++i)
            {
            UCommand* cmd = new UCommand(filter_str[i]);

            cmd->setFileArgument();

            filter_cmd->push(cmd);
            }
         }
      }
}
