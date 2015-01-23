/* Infix notation calculator-calc */

%{
/*
*/
#define YYDEBUG 1
#define TRACE_DEBUG
#include <ulib/flex/bison.h>
#include <test_bison.h>
#include <math.h>
#define YYPARSE_PARAM obj
#define YYPARSE_RETURN_TYPE int
YYPARSE_RETURN_TYPE yyparse(void*);
int yyerror(char*);
#define yylex ((UBison*)obj)->yylex
#define YYLEX_PARAM &yylval
%}

/* BISON Declarations */

%union {
   int number;
   UFlexerReference ref;
}

%token <number> NUM
%left '-' '+'
%left '*' '/'
%left NEG     /* negation--unary minus */
%right '^'    /* exponentiation        */

%type <number> exp

/* Grammar follows */

%%

input:    /* empty string */
  | input line
;

line:  '\n'
  | exp '\n'
{
	printf("\t%d\n", $1);

   U_INTERNAL_TRACE("UBison::yyparse() <line> %d", $1);
}
;

exp: NUM
{
	$$ = $1;

	U_INTERNAL_TRACE("UBison::yyparse() <exp NUM> %d", $1);
}
  | exp '+' exp
{
	$$ = $1 + $3;

	U_INTERNAL_TRACE("UBison::yyparse() <exp +> %d + %d", $1, $3);
}
  | exp '-' exp
{
	$$ = $1 - $3;

	U_INTERNAL_TRACE("UBison::yyparse() <exp -> %d - %d", $1, $3);
}
  | exp '*' exp
{
	$$ = $1 * $3;

	U_INTERNAL_TRACE("UBison::yyparse() <exp *> %d * %d", $1, $3);
}
  | exp '/' exp
{
	$$ = $1 / $3;

	U_INTERNAL_TRACE("UBison::yyparse() <exp /> %d / %d", $1, $3);
}
  | '-' exp %prec NEG
{
	$$ = -$2;

	U_INTERNAL_TRACE("UBison::yyparse() <exp NEG> %d", $2);
}
  | exp '^' exp
{
	$$ = (int) ::pow((float)$1, (float)$3);

	U_INTERNAL_TRACE("UBison::yyparse() <exp ^> %d ^ %d", $1, $3);
}
  | '(' exp ')'
{
	$$ = $2;

	U_INTERNAL_TRACE("UBison::yyparse() <exp ()> %d", $2);
}
;

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

#include <ulib/file.h>

int main(int argc, char* argv[])
{
	U_ULIB_INIT(argv);

	U_TRACE(5, "main(%d)", argc)

	UBison parser(UFile::contentOf(argv[1]));

	/*
	*/
	extern int yydebug;
	yydebug = 1;

	parser.parse();
}
