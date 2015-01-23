%{
/*
*/
#define TRACE_DEBUG
#undef  YY_DECL
#define YY_DECL int ParserInterface::yylex(void* yyval)
#include <ParserInterface.h>
#include <Parser.h>
#define YY_USER_ACTION ((YYSTYPE*)yyval)->ref.offset = parsed_chars; \
                       ((YYSTYPE*)yyval)->ref.length = yyleng; parsed_chars += yyleng; \
                       ((YYSTYPE*)yyval)->ref.ptr    = yytext;

int commentCount   = 0;
int comment_caller = 0;
%}

%option c++
%option yyclass="ParserInterface" caseless
%option nounput
%option nostack
%option noreject
%option never-interactive
%option noyywrap
%option noyymore

BCC								"Bcc"
BOUNDARY							"boundary"
BY									"by"
CC									"Cc"
COMMENTS							"Comments"
CONTENT_DISPOSITION    		"Content-Disposition"
CONTENT_TYPE    				"Content-Type"
CONTENT_TRANSFER_ENCODING  "Content-Transfer-Encoding"
DATE								"Date"
FILENAME        				"filename"
FROM								"From"
ID      							"id"
INCLUDE							"Include"
IN_REPLY_TO						"In-Reply-To"
JAN								"Jan"
JUL								"Jul"
JUN								"Jun"
KEYWORDS							"Keywords"
MIMEVERSION     				"MIME-Version"
MAR								"Mar"
MAY								"May"
MESSAGE_ID						"Message-Id"
MON								"Mon"
NOV								"Nov"
OCT								"Oct"
POSTAL							"Postal"
RECEIVED        				"Received"
REFERENCES						"References"
REPLY_TO							"Reply_to"
RETURN  							"Return-Path"
SAT								"Sat"
SEP								"Sep"
SUBJECT							"Subject"
SUN								"Sun"
THU								"Thu"
TO									"To"
TUE								"Tue"
UP2DIGIT							"UP2DIGIT"
WED								"Wed"
WITH    							"with"

OCTET (.|\n)
CHAR [\0-\177]
UPALPHA [A-Z]
LOALPHA [a-z]
ALPHA ({UPALPHA}|{LOALPHA})
DIGIT [0-9]
CTL [\0-\37\177]
CR \15
LF \n
SP \40
HT \11

CRLF ({CR}{LF})
LWSP ({SP}|{HT})
/* LWS ({CRLF}?{LWSP}) */
LWS (({CRLF}|{LF})?{LWSP})

SPECIAL [()<>@,;:\\".//]

/* TEXT ([^\200-\377\15]|((\15)/[^\n])) */
TEXT (([^\200-\377\15\n]|(\15+[^\200-\377\n]))+)
/* BOUNDARY_TEXT (([^\200-\377\-\15\n]|(\15+[^\200-\377\n]))+) */
BOUNDARY_TEXT ((([^\200-\377\15\n]|(\15+[^\200-\377\n]))+)([^\200-\377\-\15\n]|(\15+[^\200-\377\n])))
ATOM ([^\0-\37\177()<>@,;:\\".// \200-\377]+)

QTEXT ([^\200-\377"\\\15]|{LWS})
QUOTED_PAIR ("\\"{CHAR})
QUOTED_STRING (\"({QTEXT}|{QUOTED_PAIR})*\")

FNATOM ([^\0-\37\177\40:\200-\377]+)
FIELD_NAME ({FNATOM}+)

TOKEN ([^\200-\377\0-\37\177\40()<>@,;:\\"//[\]?=]+)
CTEXT ([^\200-\377()\\\15]|{LWS})
DTEXT ([^\200-\377[]\\\15]|{LWS})

DOMAIN_LITERAL_BODY ({DTEXT}|{QUOTED_PAIR})

/* ho aggiunto LWS per accettare tutti i nuovi field */
FIELD_VALUE ({ATOM}|{QUOTED_STRING}|{SPECIAL}|{TEXT}|{LWS})

/* aggiunta per nomi opzioni content field */
TOKEN_FIELD ({ALPHA}(({ALPHA}|"-"|[0-9])+))

/* COMMENT (\(({CTEXT}|{COMMENT}|{QUOTED_PAIR})*\)) */
/* DELIMITER ({SPECIAL}|{COMMENT}|{LWS}) */

BASE64_CHAR [A-Za-z0-9+/]
BASE64 ({BASE64_CHAR}{4,4})*(({BASE64_CHAR}{2,2}"=""=")|({BASE64_CHAR}{3,3}"=")|({BASE64_CHAR}{4,4}))

%s BODY_FIELD BODY COMMENT_BODY CONTENT_BODY
%x FIELDS
 
%%

<INITIAL,FIELDS>^{CONTENT_TRANSFER_ENCODING} { /* parse mail message */
  return(tCONTENT_TRANSFER_ENCODING);
};
<INITIAL,FIELDS>^{CONTENT_DISPOSITION} { /* parse mail message */
  BEGIN(CONTENT_BODY);
  return(tCONTENT_DISPOSITION);
};
<INITIAL,FIELDS>^{CONTENT_TYPE} { /* parse mail message */
  BEGIN(CONTENT_BODY);
  return(tCONTENT_TYPE);
};
<CONTENT_BODY>{BOUNDARY} {
  return(tBOUNDARY);
};
<CONTENT_BODY>{FILENAME} {
  return(tFILENAME);
};
<CONTENT_BODY>{QUOTED_STRING} {
  return(tQUOTED_STRING);
};
<CONTENT_BODY>{TOKEN} {
  return(tTOKEN_FIELD);
};
<CONTENT_BODY>{LWS} {
};

<INITIAL,FIELDS>{BCC} {
  return(tBCC);
};
<INITIAL,FIELDS>{CC} {
  return(tCC);
};
<INITIAL,FIELDS>{DATE} {
  return(tDATE);
};
<INITIAL,FIELDS>{FROM} {
  return(tFROM);
};
<INITIAL,FIELDS>{IN_REPLY_TO} {
  return(tIN_REPLY_TO);
};
<INITIAL,FIELDS>{KEYWORDS} {
  return(tKEYWORDS);
};
<INITIAL,FIELDS>{MIMEVERSION} {
  return(tMIMEVERSION);
};
<INITIAL,FIELDS>{MESSAGE_ID} {
  return(tMESSAGE_ID);
};
<INITIAL,FIELDS>{REFERENCES} {
  return(tREFERENCES);
};
<INITIAL,FIELDS>{REPLY_TO} {
  return(tREPLY_TO);
};
<INITIAL,FIELDS>{SUBJECT} {
  return(tSUBJECT);
};
<INITIAL,FIELDS>{TO} {
  return(tTO);
};
<INITIAL,FIELDS>{RETURN} {
  return(tRETURN);
};
<INITIAL,FIELDS>{RECEIVED} {
  return(tRECEIVED);
};
<INITIAL,FIELDS>{COMMENTS} {
  return(tCOMMENTS);
};
<INITIAL,FIELDS>{FIELD_NAME} {
  return(tFIELD_NAME);
};

<INITIAL,FIELDS>: {
  BEGIN(BODY_FIELD);
  return yytext[0];
};

<BODY_FIELD>({TEXT}|{LWS})+ {
  return(tTEXT);
};

<BODY_FIELD,CONTENT_BODY>{CRLF}|{LF}|(({CRLF}|{LF}){CR}) {
// pare che su linux non ci sia il CR e che a volte ci sia un CR in piu`
  BEGIN(FIELDS);
  return(tCRLF);
};

<BODY_FIELD,CONTENT_BODY>({CRLF}|{LF})({CRLF}|{LF}) {
// pare che su linux non ci sia il CR
  if (isMultipart()) {
    BEGIN(BODY);
    return(tSTART_BODY);
  } else {
    return(tEND_HEADER);
  }
};

<BODY>^--{BOUNDARY_TEXT}({CRLF}|{LF}) {
  if (isMultipart() && isBoundary(yytext)) {
    return(tSTART_MULTIPART);
  } else {
    return(tTEXT);
  }
};
<BODY>^--{BOUNDARY_TEXT}--({CRLF}|{LF}) {
  if (isMultipart() && isEndBoundary(yytext)) {
    return(tEND_BODY);
  } else {
    return(tTEXT);
  }
};
<BODY>^{TEXT} {
  return(tTEXT);
};
<BODY><<EOF>> {
  return(tEND_BODY);
};
<COMMENT_BODY>{CTEXT}|{QUOTED_PAIR} {
};

"(" {
  if (commentCount == 0) {
    comment_caller = YY_START;
  }
  ++commentCount;
  BEGIN(COMMENT_BODY);
};
")" {
  --commentCount;
  if(commentCount <= 0) {
    BEGIN(comment_caller);
  }
};

<BODY>.|{CRLF}|{LF} {
  return(tTEXT);
}

. {
  return yytext[0];
}

%%
