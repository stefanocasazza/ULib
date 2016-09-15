// gendoc.cpp

#include <ulib/file.h>

// 1000
static const char* studente = "<?xml version=\"1.0\"?>\n" \
   "<!DOCTYPE user-folder SYSTEM \"dtdUrl_556adfbc-0107-5000-ede4-d208\">\n" \
   "<user-folder>\n" \
   "<description>Cartella</description>\n" \
   "<uid>%02d_rossi_%d</uid>\n" \
   "<id>%s</id>\n" \
   "<date>02/12/2005 16:07:24</date>\n" \
   "<holder>Mario_%d Rossi_%d</holder>\n" \
   "<author>workflow</author>\n" \
   "</user-folder>";
// 19
static const char* studente_d_op_d = "<?xml version=\"1.0\"?>\n" \
   "<!DOCTYPE operation SYSTEM \"dtdUrl_556adfbc-0107-5000-ede4-d208\">\n" \
   "<operation>\n" \
   "<description>%s</description>\n" \
   "<uid>%02d_rossi_%d</uid>\n" \
   "<id>%s</id>\n" \
   "<date>03/11/2005 10:17:46</date>\n" \
   "<holder>Mario_%d Rossi_%d</holder>\n" \
   "<author>workflow</author>\n" \
   "</operation>";

static const char* operations[19] = {
   "Iscrizione alla prova",
   "Prova di valutazione",
   "Immatricolazione",
   "Conferma pagamento",
   "Ritiro credenziali",
   "Pagamento prima rata con carta di credito",
   "Pagamento prima rata con bonifico bancario",
   "Pagamento prima rata con bollettino",
   "Pagamento seconda rata",
   "Pagamento seconda rata con carta di credito",
   "Pagamento seconda rata con bollettino bancario",
   "Pagamento terza rata",
   "Esame di diritto costituzionale",
   "Esame di diritto civile",
   "Esame di diritto privato",
   "Borsa di studio",
   "Erasmus",
   "II Borsa di studio",
   "Tesi di laurea" };

int
U_EXPORT main(int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)", argc)

   int i, j, k,
#  ifdef U_COVERITY_FALSE_POSITIVE
   num = 1000;
#  else
   num = atoi(argv[1]);
#  endif

   UString directory(1000U), content(1000U), filename(100U), suffix = U_STRING_FROM_CONSTANT(".xml");

   for (k = 1; k <= 20; ++k)
      {
      directory.snprintf(U_CONSTANT_TO_PARAM("%s/D%02d/"), argv[2], k);

      UFile::_mkdir(directory.c_str(), PERM_DIRECTORY);

      // coverity[TAINTED_SCALAR]
      for (i = 1; i <= num; ++i)
         {
         filename.snprintf(U_CONSTANT_TO_PARAM("D%02d_studente_%d"), k, i);
          content.snprintf(studente, strlen(studente), k, i, filename.c_str(), i, i);

         UFile::writeTo(directory + filename + suffix, content);

         for (j = 0; j < 19; ++j)
            {
            filename.snprintf(U_CONSTANT_TO_PARAM("D%02d_studente_%d_op_%d"), k, i, j);
             content.snprintf(studente_d_op_d, strlen(studente_d_op_d), operations[j], k, i, filename.c_str(), i, i);

            UFile::writeTo(directory + filename + suffix, content);
            }

         UFile::writeTo(directory + filename + suffix, content);
         }
      }
}
