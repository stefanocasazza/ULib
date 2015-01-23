%{
#define TRACE_DEBUG
#undef  YY_DECL
#include <ulib/flex/flexer.h>
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
alpha   [A-Za-z]
name    ({alpha}|{dig}|\$)({alpha}|{dig}|[_.\-/$])*
num1    [-+]?{dig}+\.?([eE][-+]?{dig}+)?
num2    [-+]?{dig}*\.{dig}+([eE][-+]?{dig}+)?
number  {num1}|{num2}
ws      [ \t]+
word	  [^ \t\n]+
string  \"[^\n"]+\"

%x comment

%%

{ws} /* skip blanks and tabs */

\n {
	++line_num;
}

{number} {
   U_INTERNAL_TRACE("UFlexer::yylex() line_num = %4d number <%s>", line_num, yytext);
}

{name} {
   U_INTERNAL_TRACE("UFlexer::yylex() line_num = %4d name   <%s>", line_num, yytext);
}

{string} {
   U_INTERNAL_TRACE("UFlexer::yylex() line_num = %4d string <%s>", line_num, yytext);
}

"/*" {
	BEGIN(comment);
}

<comment>{
	[^*\n]*
	[^*\n]*\n			++line_num;
	"*"+[^*/\n]*
	"*"+[^*/\n]*\n		++line_num;
	"*"+"/"        	BEGIN(INITIAL);
}

{word} {
   U_INTERNAL_TRACE("UFlexer::yylex() line_num = %4d word   <%s>", line_num, yytext);
}

%%

#include <ulib/file.h>

int main(int argc, char* argv[])
{
	U_ULIB_INIT(argv);

	U_TRACE(5, "main(%d)", argc)

	UFlexer flexer;

	flexer.setData(UFile::contentOf(argv[1]));

#ifdef DEBUG
	flexer.test();
#endif
}
