%{
/*
#define YYDEBUG 1
#define TRACE_DEBUG
*/
#include <ulib/flex/bison.h>
#include <%.h>
#define YYPARSE_PARAM obj
#define YYPARSE_RETURN_TYPE int
YYPARSE_RETURN_TYPE yyparse(void*);
int yyerror(char*);
#define yylex ((UBison*)obj)->yylex
#define YYLEX_PARAM &yylval
%}

/* BISON Declarations */

/* va commentato YYLEX_PARAM se abilitato parser rientrante...
%pure_parser
*/

%union {
   int number;
   UFlexerReference ref;
}

/*
%token ...

%type ...
*/

/* Grammar follows */

%%

%%

int yyerror(char* s)
{
   U_INTERNAL_TRACE("yyerror(\"%s\")", s);

	/*
	extern int yylineno;
	U_INTERNAL_TRACE("yylineno=%d", yylineno)
	*/

	return -1;
}
