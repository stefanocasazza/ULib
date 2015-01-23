%{
/*
#define YYDEBUG 1
*/
#define TRACE_DEBUG
#include <ParserInterface.h>
#include <Parser.h>
#define YYPARSE_PARAM obj
#define YYPARSE_RETURN_TYPE int
YYPARSE_RETURN_TYPE yyparse(void*);
int yyerror(char*);
#define yylex ((ParserInterface*)obj)->yylex
/*
#define YYLEX_PARAM &yylval
*/
%}

/* BISON Declarations */

%pure_parser

%union {
   int number;
   UFlexerReference ref;
}

%token <ref> tTOKEN_FIELD
%token <ref> tBCC
%token <ref> tBOUNDARY
%token <ref> tCC
%token <ref> tCOMMENTS
%token <ref> tCONTENT_DISPOSITION
%token <ref> tCONTENT_TYPE
%token <ref> tCONTENT_TRANSFER_ENCODING
%token <ref> tCRLF
%token <ref> tDATE
%token <ref> tENCRYPTED
%token <ref> tEND_HEADER
%token <ref> tFIELD_NAME
%token <ref> tFILENAME
%token <ref> tFROM
%token <ref> tIN_REPLY_TO
%token <ref> tKEYWORDS
%token <ref> tMESSAGE_ID
%token <ref> tQUOTED_STRING
%token <ref> tRECEIVED
%token <ref> tREFERENCES
%token <ref> tREPLY_TO
%token <ref> tRESENT_BCC
%token <ref> tRESENT_CC
%token <ref> tRESENT_DATE
%token <ref> tRESENT_FROM
%token <ref> tRESENT_MESSAGE_ID
%token <ref> tRESENT_REPLY_TO
%token <ref> tRESENT_SENDER
%token <ref> tRESENT_TO
%token <ref> tRETURN
%token <ref> tSENDER
%token <ref> tSTART_BODY
%token <ref> tSTART_MULTIPART
%token <ref> tSUBJECT
%token <ref> tTEXT tMIMEVERSION
%token <ref> tTO
%token <ref> tEND_BODY

%type  <ref> fields message
%type  <ref> multipart_message_list multipart_message_item texts
%type  <ref> message_body message_body_item_list mandatory_message_body_item_list
%type  <ref> field fields_item field_body token_field content_item_list content_item user_defined_field field_name

/* Grammar follows */

%%

message: fields tEND_HEADER
{
	U_INTERNAL_TRACE("yyparse() <message 1>");

	((ParserInterface*)obj)->setHeaderBoundary($1.offset, $1.offset + $1.length);
	((ParserInterface*)obj)->setBodyBoundary($2.offset + $2.length);

	/* Return immediately from yyparse(), indicating success. */
	YYACCEPT;
}
	| fields tSTART_BODY
		{
		U_INTERNAL_TRACE("yyparse() <message 2a>");

		((ParserInterface*)obj)->buildMultipart();
		}
		message_body tEND_BODY
			{
			U_INTERNAL_TRACE("yyparse() <message 2b>");

			((ParserInterface*)obj)->setHeaderBoundary($1.offset, $1.offset + $1.length);
			((ParserInterface*)obj)->setBodyBoundary($4.offset, $4.offset + $4.length);

			/* Return immediately from yyparse(), indicating success. */
			YYACCEPT;
			}
;

fields: fields_item
{
	U_INTERNAL_TRACE("yyparse() <fields 1>");

	$$ = $1;
}
	| fields tCRLF fields_item
{
	U_INTERNAL_TRACE("yyparse() <fields 2>");

	$$.offset = $1.offset;
	$$.length = $3.offset - $1.offset + $3.length;
}
;

fields_item: field ':' field_body
{
	U_INTERNAL_TRACE("yyparse() <fields_item> %.*s", $1.length, $1.ptr);

	$$.offset = $1.offset;
	$$.length = $3.offset - $1.offset + $3.length;
}
	| tDATE ':' field_body
{
	U_INTERNAL_TRACE("yyparse() <fields_item DATE> %.*s", $1.length, $1.ptr);

	((ParserInterface*)obj)->setStartDate($3.offset, $3.length);

	$$.offset = $1.offset;
	$$.length = $3.offset - $1.offset + $3.length;
}
	| tFROM ':' field_body
{
	U_INTERNAL_TRACE("yyparse() <fields_item FROM> %.*s", $1.length, $1.ptr);

	((ParserInterface*)obj)->setFrom($3.offset, $3.length);

	$$.offset = $1.offset;
	$$.length = $3.offset - $1.offset + $3.length;
}
	| tSUBJECT ':' field_body
{
	U_INTERNAL_TRACE("yyparse() <fields_item SUBJECT> %.*s", $1.length, $1.ptr);

	((ParserInterface*)obj)->setSubject($3.offset, $3.length);

	$$.offset = $1.offset;
	$$.length = $3.offset - $1.offset + $3.length;
}
	| tMIMEVERSION ':' field_body
{
	U_INTERNAL_TRACE("yyparse() <fields_item MIMEVERSION> %.*s", $1.length, $1.ptr);

	((ParserInterface*)obj)->setMimeVersion($3.offset, $3.length);

	$$.offset = $1.offset;
	$$.length = $3.offset - $1.offset + $3.length;
}
	| tCONTENT_TRANSFER_ENCODING ':' field_body
{
	U_INTERNAL_TRACE("yyparse() <fields_item CONTENT_TRANSFER_ENCODING> %.*s", $1.length, $1.ptr);

	((ParserInterface*)obj)->setContentTransferEncoding($3.offset, $3.length);

	$$.offset = $1.offset;
	$$.length = $3.offset - $1.offset + $3.length;
}
	| tCONTENT_DISPOSITION ':' content_item_list
{
	U_INTERNAL_TRACE("yyparse() <fields_item CONTENT_DISPOSITION> %.*s", $1.length, $1.ptr);

	$$.offset = $1.offset;
	$$.length = $3.offset - $1.offset + $3.length;
}
	| tCONTENT_TYPE ':' content_item_list
{
	U_INTERNAL_TRACE("yyparse() <fields_item CONTENT_TYPE> %.*s", $1.length, $1.ptr);

	$$.offset = $1.offset;
	$$.length = $3.offset - $1.offset + $3.length;
}
;

field: tRETURN
{
  $$ = $1;
}
	| tRECEIVED
{
  $$ = $1;
}
	| tSENDER
{
  $$ = $1;
}
	| tREPLY_TO
{
  $$ = $1;
}
	| tRESENT_REPLY_TO
{
  $$ = $1;
}
	| tRESENT_FROM
{
  $$ = $1;
}
	| tRESENT_SENDER
{
  $$ = $1;
}
	| tRESENT_DATE
{
  $$ = $1;
}
	| tTO
{
  $$ = $1;
}
	| tRESENT_TO
{
  $$ = $1;
}
	| tCC
{
  $$ = $1;
}
	| tRESENT_CC
{
  $$ = $1;
}
	| tBCC
{
  $$ = $1;
}
	| tRESENT_BCC
{
  $$ = $1;
}
	| tCOMMENTS
{
  $$ = $1;
}
	| tMESSAGE_ID
{
  $$ = $1;
}
	| tRESENT_MESSAGE_ID
{
  $$ = $1;
}
	| tIN_REPLY_TO
{
  $$ = $1;
}
	| tREFERENCES
{
  $$ = $1;
}
	| tKEYWORDS
{
  $$ = $1;
}
	| tENCRYPTED
{
  $$ = $1;
}
	| user_defined_field
{
  $$ = $1;
}
;

field_body: tTEXT
{
	$$ = $1;
}
;

user_defined_field: field_name
{
  $$ = $1;
}
;

texts: tTEXT
{
  $$ = $1;
}
;

content_item_list: content_item
{
  $$ = $1;
}
	| content_item_list ';' content_item
{
  $$.offset = $1.offset;
  $$.length = $3.offset - $1.offset + $3.length;
}
;

content_item: token_field '/' token_field
{
	((ParserInterface*) obj)->setContentType($1.offset, $1.length, $3.offset, $3.length);

	$$.offset = $1.offset;
	$$.length = $3.offset - $1.offset + $3.length;
}
	| token_field '=' tQUOTED_STRING
{
	$$.offset = $1.offset;
	$$.length = $3.offset - $1.offset + $3.length;
}
	| token_field
{
	$$ = $1;
}
	| token_field '=' token_field
{
	$$.offset = $1.offset;
	$$.length = $3.offset - $1.offset + $3.length;
}
	| tFILENAME '=' tQUOTED_STRING
{
	((ParserInterface*) obj)->setName($3.offset, $3.length);

	$$.offset = $1.offset;
	$$.length = $3.offset - $1.offset + $3.length;
}
	| tBOUNDARY '=' tQUOTED_STRING
{
	((ParserInterface*) obj)->boundary($3.offset, $3.length);

	$$.offset = $1.offset;
	$$.length = $3.offset - $1.offset + $3.length;
}
;

token_field: tTOKEN_FIELD
{
  $$ = $1;
}
;

field_name: tFIELD_NAME
{
  $$ = $1;
}
;

message_body: message_body_item_list multipart_message_list
{
  $$.offset = $1.offset;
  $$.length = $2.offset - $1.offset + $2.length;
}
;

message_body_item_list:
{
  $$.offset = 0;
  $$.length = 0;
}
	| mandatory_message_body_item_list
{
  $$ = $1;
}
;

mandatory_message_body_item_list: texts
{
  $$ = $1;
}
	| mandatory_message_body_item_list texts
{
  $$.offset = $1.offset;
  $$.length = $2.offset - $1.offset + $2.length;
}
;

multipart_message_list: multipart_message_item
{
  $$ = $1;
}
	| multipart_message_list multipart_message_item
{
  $$.offset = $1.offset;
  $$.length = $2.offset - $1.offset + $2.length;
}
;

multipart_message_item: tSTART_MULTIPART message_body_item_list
{
  ((ParserInterface*) obj)->parse($2.offset, $2.length);

  $$.offset = $1.offset;
  $$.length = $2.offset - $1.offset + $2.length;
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
