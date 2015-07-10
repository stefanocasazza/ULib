#ifndef YYTOKENTYPE
#define YYTOKENTYPE
enum yytokentype { NUM = 258, NEG = 259 };
#endif
#define NUM 258
#define NEG 259
#if !defined YYSTYPE && !defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE { int number; UFlexerReference ref; } YYSTYPE;
# define yystype YYSTYPE
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif
extern YYSTYPE yylval;
