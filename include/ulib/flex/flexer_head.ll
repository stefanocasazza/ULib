%{
/* #define TRACE_DEBUG */
#include <ulib/flex/flexer.h>
#include <%.h>
#undef  YY_DECL
#define YY_DECL int UFlexer::yylex(void* yyval)
#define YY_USER_ACTION ((YYSTYPE*)yyval)->ref.offset = parsed_chars; \
                       ((YYSTYPE*)yyval)->ref.length = yyleng; parsed_chars += yyleng; \
                       ((YYSTYPE*)yyval)->ref.ptr    = yytext;
%}

%option c++
%option yyclass="UFlexer" caseless
%option nounput
%option nostack
%option noreject
%option never-interactive
%option noyywrap
%option noyymore
