// PEC_report_options.h

#ifndef PEC_report_options_H
#define PEC_report_options_H 1

#include <ulib/string.h>

#undef  PACKAGE
#undef  VERSION
#define VERSION "2.1"
#undef  ARGS
#define ARGS ""

#define U_OPTIONS_GEN1 \
"option c config           1 \"path of configuration file\" \"\"\n" \
"option d directory        1 \"directory path containing log files\" \"\"\n" \
"option e ends_with        1 \"proces only filenames that ends with this string\" \"\"\n" \
"option f from             1 \"processing log entry from this date (dd/mm/yyyy)\" \"\"\n" \
"option t to               1 \"processing log entry to this date (dd/mm/yyyy)\" \"\"\n" \
"option o optimization     0 \"processing files considering the date contained in the name: (PECx-yyyy-mm-dd)\" \"\"\n" \
"option F filter           1 \"filter records of log entry that match this string\" \"\"\n"
#define U_OPTIONS_GEN2 \
"option T title            1 \"title of the report\" \"\"\n" \
"option n domain           1 \"file containing list of own domain name\" \"\"\n" \
"option i installation_id  1 \"list of installation ID to be used as filter (separated by comma)\" \"\"\n"

#endif
