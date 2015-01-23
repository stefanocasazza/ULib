%{
#define TRACE_DEBUG
#undef  YY_DECL
#include <ulib/flex/flexer.h>
#include <test_bison.h>
#define YY_DECL int UFlexer::yylex(void* yyval)
#define YY_USER_ACTION ((YYSTYPE*)yyval)->ref.offset = parsed_chars; \
                       ((YYSTYPE*)yyval)->ref.length = yyleng; parsed_chars += yyleng; \
                       ((YYSTYPE*)yyval)->ref.ptr    = yytext;

int line_num = 1;
%}

%option c++
%option yyclass="UFlexer" caseless
%option nounput
%option nostack
%option noreject
%option never-interactive
%option noyywrap
%option noyymore

dig     [0-9]
num1    [-+]?{dig}+\.?([eE][-+]?{dig}+)?
num2    [-+]?{dig}*\.{dig}+([eE][-+]?{dig}+)?
number  {num1}|{num2}
ws 	  [ \f\t\r]+

%%

{ws} /* skip blanks and tabs */

{number} {
   U_INTERNAL_TRACE("UFlexer::yylex() line_num = %d number <%s>", line_num, yytext);

	yylval.number = strtol(yytext, 0, 0);

	return NUM;
}

\n {
   U_INTERNAL_TRACE("UFlexer::yylex() line_num = %d new line", line_num);

	++line_num;

	return '\n';
}

. return yytext[0];

%%
