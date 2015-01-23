b4_copyright(Skeleton parser for Yacc-like parsing with Bison,
             1984, 1989, 1990, 2000, 2001, 2002)

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

b4_token_defines([[tTOKEN_FIELD]], 258,
[[tBCC]], 259,
[[tBOUNDARY]], 260,
[[tCC]], 261,
[[tCOMMENTS]], 262,
[[tCONTENT_DISPOSITION]], 263,
[[tCONTENT_TYPE]], 264,
[[tCONTENT_TRANSFER_ENCODING]], 265,
[[tCRLF]], 266,
[[tDATE]], 267,
[[tENCRYPTED]], 268,
[[tEND_HEADER]], 269,
[[tFIELD_NAME]], 270,
[[tFILENAME]], 271,
[[tFROM]], 272,
[[tIN_REPLY_TO]], 273,
[[tKEYWORDS]], 274,
[[tMESSAGE_ID]], 275,
[[tQUOTED_STRING]], 276,
[[tRECEIVED]], 277,
[[tREFERENCES]], 278,
[[tREPLY_TO]], 279,
[[tRESENT_BCC]], 280,
[[tRESENT_CC]], 281,
[[tRESENT_DATE]], 282,
[[tRESENT_FROM]], 283,
[[tRESENT_MESSAGE_ID]], 284,
[[tRESENT_REPLY_TO]], 285,
[[tRESENT_SENDER]], 286,
[[tRESENT_TO]], 287,
[[tRETURN]], 288,
[[tSENDER]], 289,
[[tSTART_BODY]], 290,
[[tSTART_MULTIPART]], 291,
[[tSUBJECT]], 292,
[[tTEXT]], 293,
[[tMIMEVERSION]], 294,
[[tTO]], 295,
[[tEND_BODY]], 296)

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
b4_syncline(b4_stype_line, b4_filename)
typedef union YYSTYPE b4_syncline(22, ["Parser.yy"])
{
   int number;
   UFlexerReference ref;
} YYSTYPE;
/* Line 1248 of yacc.c.  */
b4_syncline(58, "y.tab.h")
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

b4_pure_if(,
extern YYSTYPE b4_prefix[]lval;)

b4_location_if(
#if ! defined (YYLTYPE) && ! defined (YYLTYPE_IS_DECLARED)
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif

m4_if(b4_pure, [0],
[extern YYLTYPE b4_prefix[]lloc;])
)

