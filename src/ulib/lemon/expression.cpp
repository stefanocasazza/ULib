/*
** 2000-05-29
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** Driver template for the LEMON parser generator.
**
** The "lemon" program processes an LALR(1) input grammar file, then uses
** this template to construct a parser.  The "lemon" program inserts text
** at each "%%" line.  Also, any "P-a-r-s-e" identifer prefix (without the
** interstitial "-" characters) contained in this template is changed into
** the value of the %name directive from the grammar.  Otherwise, the content
** of this template is copied straight through into the generate parser
** source file.
**
** The following is the concatenation of all %include directives from the
** input grammar file:
*/
#include <stdio.h>
/************ Begin %include sections from the grammar ************************/
#line 1 "expression.y"

#include <ulib/tokenizer.h>
#include <ulib/dynamic/dynamic.h>
#include <ulib/utility/string_ext.h>

#include <assert.h>

typedef long (*lPFv)(void);
typedef long (*lPFll)(long, long);

typedef UVector<UString> Items;

extern void* expressionParserAlloc(void* (*mallocProc)(size_t));
extern void  expressionParserFree(void* p, void (*freeProc)(void*));
extern void  expressionParserTrace(FILE* stream, char* zPrefix);
extern void  expressionParser(void* yyp, int yymajor, UString* yyminor, UString* result);

extern void token_destructor(UString* token);

void token_destructor(UString* token) {
   U_TRACE(0, "::token_destructor(%V)", token->rep)
   delete token;
}

#line 53 "expression.c"
/**************** End of %include directives **********************************/
/* These constants specify the various numeric values for terminal symbols
** in a format understandable to "makeheaders".  This section is blank unless
** "lemon" is run with the "-m" command-line option.
***************** Begin makeheaders token definitions *************************/
/**************** End makeheaders token definitions ***************************/

/* The next sections is a series of control #defines.
** various aspects of the generated parser.
**    YYCODETYPE         is the data type used to store the integer codes
**                       that represent terminal and non-terminal symbols.
**                       "unsigned char" is used if there are fewer than
**                       256 symbols.  Larger types otherwise.
**    YYNOCODE           is a number of type YYCODETYPE that is not used for
**                       any terminal or nonterminal symbol.
**    YYFALLBACK         If defined, this indicates that one or more tokens
**                       (also known as: "terminal symbols") have fall-back
**                       values which should be used if the original symbol
**                       would not parse.  This permits keywords to sometimes
**                       be used as identifiers, for example.
**    YYACTIONTYPE       is the data type used for "action codes" - numbers
**                       that indicate what to do in response to the next
**                       token.
**    expressionParserTOKENTYPE     is the data type used for minor type for terminal
**                       symbols.  Background: A "minor type" is a semantic
**                       value associated with a terminal or non-terminal
**                       symbols.  For example, for an "ID" terminal symbol,
**                       the minor type might be the name of the identifier.
**                       Each non-terminal can have a different minor type.
**                       Terminal symbols all have the same minor type, though.
**                       This macros defines the minor type for terminal 
**                       symbols.
**    YYMINORTYPE        is the data type used for all minor types.
**                       This is typically a union of many types, one of
**                       which is expressionParserTOKENTYPE.  The entry in the union
**                       for terminal symbols is called "yy0".
**    YYSTACKDEPTH       is the maximum depth of the parser's stack.  If
**                       zero the stack is dynamically sized using realloc()
**    expressionParserARG_SDECL     A static variable declaration for the %extra_argument
**    expressionParserARG_PDECL     A parameter declaration for the %extra_argument
**    expressionParserARG_STORE     Code to store %extra_argument into yypParser
**    expressionParserARG_FETCH     Code to extract %extra_argument from yypParser
**    YYERRORSYMBOL      is the code number of the error symbol.  If not
**                       defined, then do no error processing.
**    YYNSTATE           the combined number of states.
**    YYNRULE            the number of rules in the grammar
**    YY_MAX_SHIFT       Maximum value for shift actions
**    YY_MIN_SHIFTREDUCE Minimum value for shift-reduce actions
**    YY_MAX_SHIFTREDUCE Maximum value for shift-reduce actions
**    YY_MIN_REDUCE      Maximum value for reduce actions
**    YY_ERROR_ACTION    The yy_action[] code for syntax error
**    YY_ACCEPT_ACTION   The yy_action[] code for accept
**    YY_NO_ACTION       The yy_action[] code for no-op
*/
#ifndef INTERFACE
# define INTERFACE 1
#endif
/************* Begin control #defines *****************************************/
#define YYCODETYPE unsigned char
#define YYNOCODE 41
#define YYACTIONTYPE unsigned char
#define expressionParserTOKENTYPE  UString* 
typedef union {
  int yyinit;
  expressionParserTOKENTYPE yy0;
  UString* yy3;
  int yy12;
  Items* yy49;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define expressionParserARG_SDECL  UString* result ;
#define expressionParserARG_PDECL , UString* result 
#define expressionParserARG_FETCH  UString* result  = yypParser->result 
#define expressionParserARG_STORE yypParser->result  = result 
#define YYNSTATE             23
#define YYNRULE              38
#define YY_MAX_SHIFT         22
#define YY_MIN_SHIFTREDUCE   51
#define YY_MAX_SHIFTREDUCE   88
#define YY_MIN_REDUCE        89
#define YY_MAX_REDUCE        126
#define YY_ERROR_ACTION      127
#define YY_ACCEPT_ACTION     128
#define YY_NO_ACTION         129
/************* End control #defines *******************************************/

/* The yyzerominor constant is used to initialize instances of
** YYMINORTYPE objects to zero. */
static const YYMINORTYPE yyzerominor = { 0 };

/* Define the yytestcase() macro to be a no-op if is not already defined
** otherwise.
**
** Applications can choose to define yytestcase() in the %include section
** to a macro that can assist in verifying code coverage.  For production
** code the yytestcase() macro should be turned off.  But it is useful
** for testing.
*/
#ifndef yytestcase
# define yytestcase(X)
#endif


/* Next are the tables used to determine what action to take based on the
** current state and lookahead token.  These tables are used to implement
** functions that take a state number and lookahead value and return an
** action integer.  
**
** Suppose the action integer is N.  Then the action is determined as
** follows
**
**   0 <= N <= YY_MAX_SHIFT             Shift N.  That is, push the lookahead
**                                      token onto the stack and goto state N.
**
**   N between YY_MIN_SHIFTREDUCE       Shift to an arbitrary state then
**     and YY_MAX_SHIFTREDUCE           reduce by rule N-YY_MIN_SHIFTREDUCE.
**
**   N between YY_MIN_REDUCE            Reduce by rule N-YY_MIN_REDUCE
**     and YY_MAX_REDUCE

**   N == YY_ERROR_ACTION               A syntax error has occurred.
**
**   N == YY_ACCEPT_ACTION              The parser accepts its input.
**
**   N == YY_NO_ACTION                  No such action.  Denotes unused
**                                      slots in the yy_action[] table.
**
** The action table is constructed as a single large table named yy_action[].
** Given state S and lookahead X, the action is computed as
**
**      yy_action[ yy_shift_ofst[S] + X ]
**
** If the index value yy_shift_ofst[S]+X is out of range or if the value
** yy_lookahead[yy_shift_ofst[S]+X] is not equal to X or if yy_shift_ofst[S]
** is equal to YY_SHIFT_USE_DFLT, it means that the action is not in the table
** and that yy_default[S] should be used instead.  
**
** The formula above is for computing the action when the lookahead is
** a terminal symbol.  If the lookahead is a non-terminal (as occurs after
** a reduce action) then the yy_reduce_ofst[] array is used in place of
** the yy_shift_ofst[] array and YY_REDUCE_USE_DFLT is used in place of
** YY_SHIFT_USE_DFLT.
**
** The following are the tables generated in this section:
**
**  yy_action[]        A single table containing all actions.
**  yy_lookahead[]     A table containing the lookahead for each entry in
**                     yy_action.  Used to detect hash collisions.
**  yy_shift_ofst[]    For each state, the offset into yy_action for
**                     shifting terminals.
**  yy_reduce_ofst[]   For each state, the offset into yy_action for
**                     shifting non-terminals after a reduce.
**  yy_default[]       Default action for each state.
**
*********** Begin parsing tables **********************************************/
#define YY_ACTTAB_COUNT (87)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */   128,   15,    8,   10,   17,   12,   61,   63,   22,   13,
 /*    10 */     8,   10,   17,   12,   61,   63,   22,    9,   10,   17,
 /*    20 */    12,   61,   63,   22,   11,   17,   12,   61,   63,   22,
 /*    30 */    70,   71,   14,   61,   63,   22,   72,   73,   74,   75,
 /*    40 */    18,   12,   61,   63,   22,    3,    7,   21,    1,    4,
 /*    50 */    66,    2,   76,   77,   78,   79,   60,   63,   22,   21,
 /*    60 */     1,    6,   66,   82,   83,   84,   69,   68,   89,   69,
 /*    70 */    68,   62,   22,    5,   85,   87,   80,   81,   86,   19,
 /*    80 */    20,   88,   16,   67,   91,   91,   64,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */    31,   32,   33,   34,   35,   36,   37,   38,   39,   32,
 /*    10 */    33,   34,   35,   36,   37,   38,   39,   33,   34,   35,
 /*    20 */    36,   37,   38,   39,   34,   35,   36,   37,   38,   39,
 /*    30 */     3,    4,   36,   37,   38,   39,    9,   10,   11,   12,
 /*    40 */    35,   36,   37,   38,   39,   27,   18,   19,   20,   28,
 /*    50 */    22,   26,    5,    6,    7,    8,   37,   38,   39,   19,
 /*    60 */    20,   30,   22,   15,   16,   17,    1,    2,    0,    1,
 /*    70 */     2,   38,   39,   29,   21,   22,   13,   14,   21,   25,
 /*    80 */    23,   22,   20,   22,   40,   40,   21,
};
#define YY_SHIFT_USE_DFLT (-1)
#define YY_SHIFT_COUNT (22)
#define YY_SHIFT_MIN   (0)
#define YY_SHIFT_MAX   (68)
static const signed char yy_shift_ofst[] = {
 /*     0 */    28,   28,   28,   28,   28,   28,   28,   40,   27,   27,
 /*    10 */    47,   47,   48,   65,   48,   68,   53,   63,   63,   57,
 /*    20 */    59,   62,   61,
};
#define YY_REDUCE_USE_DFLT (-32)
#define YY_REDUCE_COUNT (18)
#define YY_REDUCE_MIN   (-31)
#define YY_REDUCE_MAX   (54)
static const signed char yy_reduce_ofst[] = {
 /*     0 */   -31,  -23,  -16,  -10,    5,   -4,   19,   33,   18,   18,
 /*    10 */    21,   21,   31,   25,   31,   25,   54,   44,   44,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   127,  127,  127,  127,  127,  127,  127,  127,   91,   90,
 /*    10 */    93,   92,   97,  127,   96,  127,  127,   95,   94,  127,
 /*    20 */   127,  127,  103,
};
/********** End of lemon-generated parsing tables *****************************/

/* The next table maps tokens (terminal symbols) into fallback tokens.  
** If a construct like the following:
** 
**      %fallback ID X Y Z.
**
** appears in the grammar, then ID becomes a fallback token for X, Y,
** and Z.  Whenever one of the tokens X, Y, or Z is input to the parser
** but it does not parse, the type of the token is changed to ID and
** the parse is retried before an error is thrown.
**
** This feature can be used, for example, to cause some keywords in a language
** to revert to identifiers if they keyword does not apply in the context where
** it appears.
*/
#ifdef YYFALLBACK
static const YYCODETYPE yyFallback[] = {
};
#endif /* YYFALLBACK */

/* The following structure represents a single element of the
** parser's stack.  Information stored includes:
**
**   +  The state number for the parser at this level of the stack.
**
**   +  The value of the token stored at this level of the stack.
**      (In other words, the "major" token.)
**
**   +  The semantic value stored at this level of the stack.  This is
**      the information used by the action routines in the grammar.
**      It is sometimes called the "minor" token.
**
** After the "shift" half of a SHIFTREDUCE action, the stateno field
** actually contains the reduce action for the second half of the
** SHIFTREDUCE.
*/
struct yyStackEntry {
  YYACTIONTYPE stateno;  /* The state-number, or reduce action in SHIFTREDUCE */
  YYCODETYPE major;      /* The major token value.  This is the code
                         ** number for the token at this stack level */
  YYMINORTYPE minor;     /* The user-supplied minor token value.  This
                         ** is the value of the token  */
};
typedef struct yyStackEntry yyStackEntry;

/* The state of the parser is completely contained in an instance of
** the following structure */
struct yyParser {
  int yyidx;                    /* Index of top element in stack */
#ifdef YYTRACKMAXSTACKDEPTH
  int yyidxMax;                 /* Maximum value of yyidx */
#endif
  int yyerrcnt;                 /* Shifts left before out of the error */
  expressionParserARG_SDECL                /* A place to hold %extra_argument */
#if YYSTACKDEPTH<=0
  int yystksz;                  /* Current side of the stack */
  yyStackEntry *yystack;        /* The parser's stack */
#else
  yyStackEntry yystack[YYSTACKDEPTH];  /* The parser's stack */
#endif
};
typedef struct yyParser yyParser;

#ifndef NDEBUG
#include <stdio.h>
static FILE *yyTraceFILE = 0;
static char *yyTracePrompt = 0;
#endif /* NDEBUG */

#ifndef NDEBUG
/* 
** Turn parser tracing on by giving a stream to which to write the trace
** and a prompt to preface each trace message.  Tracing is turned off
** by making either argument NULL 
**
** Inputs:
** <ul>
** <li> A FILE* to which trace output should be written.
**      If NULL, then tracing is turned off.
** <li> A prefix string written at the beginning of every
**      line of trace output.  If NULL, then tracing is
**      turned off.
** </ul>
**
** Outputs:
** None.
*/
void expressionParserTrace(FILE *TraceFILE, char *zTracePrompt){
  yyTraceFILE = TraceFILE;
  yyTracePrompt = zTracePrompt;
  if( yyTraceFILE==0 ) yyTracePrompt = 0;
  else if( yyTracePrompt==0 ) yyTraceFILE = 0;
}
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing shifts, the names of all terminals and nonterminals
** are required.  The following table supplies these names */
static const char *const yyTokenName[] = { 
  "$",             "AND",           "OR",            "EQ",          
  "NE",            "GT",            "GE",            "LT",          
  "LE",            "STARTS_WITH",   "ENDS_WITH",     "IS_PRESENT",  
  "CONTAINS",      "PLUS",          "MINUS",         "MULT",        
  "DIV",           "MOD",           "NOT",           "FN_CALL",     
  "LPAREN",        "RPAREN",        "VALUE",         "COMMA",       
  "error",         "params",        "booleanCond",   "equalityCond",
  "relationalCond",  "additiveCond",  "multiplicativeCond",  "input",       
  "booleanExpression",  "equalityExpression",  "relationalExpression",  "additiveExpression",
  "multiplicativeExpression",  "unaryExpression",  "primaryExpression",  "value",       
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
 /*   0 */ "input ::= booleanExpression",
 /*   1 */ "booleanExpression ::= booleanExpression booleanCond equalityExpression",
 /*   2 */ "booleanExpression ::= equalityExpression",
 /*   3 */ "equalityExpression ::= equalityExpression equalityCond relationalExpression",
 /*   4 */ "equalityExpression ::= relationalExpression",
 /*   5 */ "relationalExpression ::= relationalExpression relationalCond additiveExpression",
 /*   6 */ "relationalExpression ::= additiveExpression",
 /*   7 */ "additiveExpression ::= additiveExpression additiveCond multiplicativeExpression",
 /*   8 */ "additiveExpression ::= multiplicativeExpression",
 /*   9 */ "multiplicativeExpression ::= multiplicativeExpression multiplicativeCond unaryExpression",
 /*  10 */ "multiplicativeExpression ::= unaryExpression",
 /*  11 */ "unaryExpression ::= NOT primaryExpression",
 /*  12 */ "unaryExpression ::= primaryExpression",
 /*  13 */ "primaryExpression ::= LPAREN booleanExpression RPAREN",
 /*  14 */ "primaryExpression ::= value",
 /*  15 */ "value ::= VALUE",
 /*  16 */ "value ::= value VALUE",
 /*  17 */ "booleanCond ::= OR",
 /*  18 */ "booleanCond ::= AND",
 /*  19 */ "equalityCond ::= EQ",
 /*  20 */ "equalityCond ::= NE",
 /*  21 */ "equalityCond ::= STARTS_WITH",
 /*  22 */ "equalityCond ::= ENDS_WITH",
 /*  23 */ "equalityCond ::= IS_PRESENT",
 /*  24 */ "equalityCond ::= CONTAINS",
 /*  25 */ "relationalCond ::= GT",
 /*  26 */ "relationalCond ::= GE",
 /*  27 */ "relationalCond ::= LT",
 /*  28 */ "relationalCond ::= LE",
 /*  29 */ "additiveCond ::= PLUS",
 /*  30 */ "additiveCond ::= MINUS",
 /*  31 */ "multiplicativeCond ::= MULT",
 /*  32 */ "multiplicativeCond ::= DIV",
 /*  33 */ "multiplicativeCond ::= MOD",
 /*  34 */ "primaryExpression ::= FN_CALL LPAREN RPAREN",
 /*  35 */ "primaryExpression ::= FN_CALL LPAREN params RPAREN",
 /*  36 */ "params ::= VALUE",
 /*  37 */ "params ::= params COMMA VALUE",
};
#endif /* NDEBUG */


#if YYSTACKDEPTH<=0
/*
** Try to increase the size of the parser stack.
*/
static void yyGrowStack(yyParser *p){
  int newSize;
  yyStackEntry *pNew;

  newSize = p->yystksz*2 + 100;
  pNew = realloc(p->yystack, newSize*sizeof(pNew[0]));
  if( pNew ){
    p->yystack = pNew;
    p->yystksz = newSize;
#ifndef NDEBUG
    if( yyTraceFILE ){
      fprintf(yyTraceFILE,"%sStack grows to %d entries!\n",
              yyTracePrompt, p->yystksz);
    }
#endif
  }
}
#endif

/* Datatype of the argument to the memory allocated passed as the
** second argument to expressionParserAlloc() below.  This can be changed by
** putting an appropriate #define in the %include section of the input
** grammar.
*/
#ifndef YYMALLOCARGTYPE
# define YYMALLOCARGTYPE size_t
#endif

/* 
** This function allocates a new parser.
** The only argument is a pointer to a function which works like
** malloc.
**
** Inputs:
** A pointer to the function used to allocate memory.
**
** Outputs:
** A pointer to a parser.  This pointer is used in subsequent calls
** to expressionParser and expressionParserFree.
*/
void *expressionParserAlloc(void *(*mallocProc)(YYMALLOCARGTYPE)){
  yyParser *pParser;
  pParser = (yyParser*)(*mallocProc)( (YYMALLOCARGTYPE)sizeof(yyParser) );
  if( pParser ){
    pParser->yyidx = -1;
#ifdef YYTRACKMAXSTACKDEPTH
    pParser->yyidxMax = 0;
#endif
#if YYSTACKDEPTH<=0
    pParser->yystack = NULL;
    pParser->yystksz = 0;
    yyGrowStack(pParser);
#endif
  }
  return pParser;
}

/* The following function deletes the "minor type" or semantic value
** associated with a symbol.  The symbol can be either a terminal
** or nonterminal. "yymajor" is the symbol code, and "yypminor" is
** a pointer to the value to be deleted.  The code used to do the 
** deletions is derived from the %destructor and/or %token_destructor
** directives of the input grammar.
*/
static void yy_destructor(
  yyParser *yypParser,    /* The parser */
  YYCODETYPE yymajor,     /* Type code for object to destroy */
  YYMINORTYPE *yypminor   /* The object to be destroyed */
){
  switch( yymajor ){
    /* Here is inserted the actions which take place when a
    ** terminal or non-terminal is destroyed.  This can happen
    ** when the symbol is popped from the stack during a
    ** reduce or during error processing or when a parser is 
    ** being destroyed before it is finished parsing.
    **
    ** Note: during a reduce, the only symbols destroyed are those
    ** which appear on the RHS of the rule, but which are *not* used
    ** inside the C code.
    */
/********* Begin destructor definitions ***************************************/
      /* TERMINAL Destructor */
    case 1: /* AND */
    case 2: /* OR */
    case 3: /* EQ */
    case 4: /* NE */
    case 5: /* GT */
    case 6: /* GE */
    case 7: /* LT */
    case 8: /* LE */
    case 9: /* STARTS_WITH */
    case 10: /* ENDS_WITH */
    case 11: /* IS_PRESENT */
    case 12: /* CONTAINS */
    case 13: /* PLUS */
    case 14: /* MINUS */
    case 15: /* MULT */
    case 16: /* DIV */
    case 17: /* MOD */
    case 18: /* NOT */
    case 19: /* FN_CALL */
    case 20: /* LPAREN */
    case 21: /* RPAREN */
    case 22: /* VALUE */
    case 23: /* COMMA */
{
#line 32 "expression.y"
 token_destructor((yypminor->yy0)); 
#line 528 "expression.c"
}
      break;
    case 25: /* params */
{
#line 34 "expression.y"

   U_TRACE(0, "::params_destructor(%p)", (yypminor->yy49))
   delete (yypminor->yy49);

#line 538 "expression.c"
}
      break;
/********* End destructor definitions *****************************************/
    default:  break;   /* If no destructor action specified: do nothing */
  }
}

/*
** Pop the parser's stack once.
**
** If there is a destructor routine associated with the token which
** is popped from the stack, then call it.
*/
static void yy_pop_parser_stack(yyParser *pParser){
  yyStackEntry *yytos;
  assert( pParser->yyidx>=0 );
  yytos = &pParser->yystack[pParser->yyidx--];
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sPopping %s\n",
      yyTracePrompt,
      yyTokenName[yytos->major]);
  }
#endif
  yy_destructor(pParser, yytos->major, &yytos->minor);
}

/* 
** Deallocate and destroy a parser.  Destructors are called for
** all stack elements before shutting the parser down.
**
** If the YYPARSEFREENEVERNULL macro exists (for example because it
** is defined in a %include section of the input grammar) then it is
** assumed that the input pointer is never NULL.
*/
void expressionParserFree(
  void *p,                    /* The parser to be deleted */
  void (*freeProc)(void*)     /* Function used to reclaim memory */
){
  yyParser *pParser = (yyParser*)p;
#ifndef YYPARSEFREENEVERNULL
  if( pParser==0 ) return;
#endif
  while( pParser->yyidx>=0 ) yy_pop_parser_stack(pParser);
#if YYSTACKDEPTH<=0
  free(pParser->yystack);
#endif
  (*freeProc)((void*)pParser);
}

/*
** Return the peak depth of the stack for a parser.
*/
#ifdef YYTRACKMAXSTACKDEPTH
int expressionParserStackPeak(void *p){
  yyParser *pParser = (yyParser*)p;
  return pParser->yyidxMax;
}
#endif

/*
** Find the appropriate action for a parser given the terminal
** look-ahead token iLookAhead.
*/
__pure static int yy_find_shift_action(
  yyParser *pParser,        /* The parser */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
  int stateno = pParser->yystack[pParser->yyidx].stateno;
 
  if( stateno>=YY_MIN_REDUCE ) return stateno;
  assert( stateno <= YY_SHIFT_COUNT );
  do{
    i = yy_shift_ofst[stateno];
    if( i==YY_SHIFT_USE_DFLT ) return yy_default[stateno];
    assert( iLookAhead!=YYNOCODE );
    i += iLookAhead;
    if( i<0 || i>=YY_ACTTAB_COUNT || yy_lookahead[i]!=iLookAhead ){
      if( iLookAhead>0 ){
#ifdef YYFALLBACK
        YYCODETYPE iFallback;            /* Fallback token */
        if( iLookAhead<sizeof(yyFallback)/sizeof(yyFallback[0])
               && (iFallback = yyFallback[iLookAhead])!=0 ){
#ifndef NDEBUG
          if( yyTraceFILE ){
            fprintf(yyTraceFILE, "%sFALLBACK %s => %s\n",
               yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[iFallback]);
          }
#endif
          assert( yyFallback[iFallback]==0 ); /* Fallback loop must terminate */
          iLookAhead = iFallback;
          continue;
        }
#endif
#ifdef YYWILDCARD
        {
          int j = i - iLookAhead + YYWILDCARD;
          if( 
#if YY_SHIFT_MIN+YYWILDCARD<0
            j>=0 &&
#endif
#if YY_SHIFT_MAX+YYWILDCARD>=YY_ACTTAB_COUNT
            j<YY_ACTTAB_COUNT &&
#endif
            yy_lookahead[j]==YYWILDCARD
          ){
#ifndef NDEBUG
            if( yyTraceFILE ){
              fprintf(yyTraceFILE, "%sWILDCARD %s => %s\n",
                 yyTracePrompt, yyTokenName[iLookAhead],
                 yyTokenName[YYWILDCARD]);
            }
#endif /* NDEBUG */
            return yy_action[j];
          }
        }
#endif /* YYWILDCARD */
      }
      return yy_default[stateno];
    }else{
      return yy_action[i];
    }
  }while(1);
}

/*
** Find the appropriate action for a parser given the non-terminal
** look-ahead token iLookAhead.
*/
__pure static int yy_find_reduce_action(
  int stateno,              /* Current state number */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
#ifdef YYERRORSYMBOL
  if( stateno>YY_REDUCE_COUNT ){
    return yy_default[stateno];
  }
#else
  assert( stateno<=YY_REDUCE_COUNT );
#endif
  i = yy_reduce_ofst[stateno];
  assert( i!=YY_REDUCE_USE_DFLT );
  assert( iLookAhead!=YYNOCODE );
  i += iLookAhead;
#ifdef YYERRORSYMBOL
  if( i<0 || i>=YY_ACTTAB_COUNT || yy_lookahead[i]!=iLookAhead ){
    return yy_default[stateno];
  }
#else
  assert( i>=0 && i<YY_ACTTAB_COUNT );
  assert( yy_lookahead[i]==iLookAhead );
#endif
  return yy_action[i];
}

/*
** The following routine is called if the stack overflows.
*/
static void yyStackOverflow(yyParser *yypParser, YYMINORTYPE *yypMinor){
   expressionParserARG_FETCH;
   yypParser->yyidx--;
#ifndef NDEBUG
   if( yyTraceFILE ){
     fprintf(yyTraceFILE,"%sStack Overflow!\n",yyTracePrompt);
   }
#endif
   while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
   /* Here code is inserted which will execute if the parser
   ** stack every overflows */
/******** Begin %stack_overflow code ******************************************/
#line 56 "expression.y"

   U_TRACE_NO_PARAM(0, "::stack_overflow()")
   U_WARNING("Parse stack overflow");
#line 715 "expression.c"
/******** End %stack_overflow code ********************************************/
   expressionParserARG_STORE; /* Suppress warning about unused %extra_argument var */
}

/*
** Print tracing information for a SHIFT action
*/
#ifndef NDEBUG
static void yyTraceShift(yyParser *yypParser, int yyNewState){
  if( yyTraceFILE ){
    if( yyNewState<YYNSTATE ){
      fprintf(yyTraceFILE,"%sShift '%s', go to state %d\n",
         yyTracePrompt,yyTokenName[yypParser->yystack[yypParser->yyidx].major],
         yyNewState);
    }else{
      fprintf(yyTraceFILE,"%sShift '%s'\n",
         yyTracePrompt,yyTokenName[yypParser->yystack[yypParser->yyidx].major]);
    }
  }
}
#else
# define yyTraceShift(X,Y)
#endif

/*
** Perform a shift action.
*/
static void yy_shift(
  yyParser *yypParser,          /* The parser to be shifted */
  int yyNewState,               /* The new state to shift in */
  int yyMajor,                  /* The major token to shift in */
  YYMINORTYPE *yypMinor         /* Pointer to the minor token to shift in */
){
  yyStackEntry *yytos;
  yypParser->yyidx++;
#ifdef YYTRACKMAXSTACKDEPTH
  if( yypParser->yyidx>yypParser->yyidxMax ){
    yypParser->yyidxMax = yypParser->yyidx;
  }
#endif
#if YYSTACKDEPTH>0 
  if( yypParser->yyidx>=YYSTACKDEPTH ){
    yyStackOverflow(yypParser, yypMinor);
    return;
  }
#else
  if( yypParser->yyidx>=yypParser->yystksz ){
    yyGrowStack(yypParser);
    if( yypParser->yyidx>=yypParser->yystksz ){
      yyStackOverflow(yypParser, yypMinor);
      return;
    }
  }
#endif
  yytos = &yypParser->yystack[yypParser->yyidx];
  yytos->stateno = (YYACTIONTYPE)yyNewState;
  yytos->major = (YYCODETYPE)yyMajor;
  yytos->minor = *yypMinor;
  yyTraceShift(yypParser, yyNewState);
}

/* The following table contains information about every rule that
** is used during the reduce.
*/
static const struct {
  YYCODETYPE lhs;         /* Symbol on the left-hand side of the rule */
  unsigned char nrhs;     /* Number of right-hand side symbols in the rule */
} yyRuleInfo[] = {
  { 31, 1 },
  { 32, 3 },
  { 32, 1 },
  { 33, 3 },
  { 33, 1 },
  { 34, 3 },
  { 34, 1 },
  { 35, 3 },
  { 35, 1 },
  { 36, 3 },
  { 36, 1 },
  { 37, 2 },
  { 37, 1 },
  { 38, 3 },
  { 38, 1 },
  { 39, 1 },
  { 39, 2 },
  { 26, 1 },
  { 26, 1 },
  { 27, 1 },
  { 27, 1 },
  { 27, 1 },
  { 27, 1 },
  { 27, 1 },
  { 27, 1 },
  { 28, 1 },
  { 28, 1 },
  { 28, 1 },
  { 28, 1 },
  { 29, 1 },
  { 29, 1 },
  { 30, 1 },
  { 30, 1 },
  { 30, 1 },
  { 38, 3 },
  { 38, 4 },
  { 25, 1 },
  { 25, 3 },
};

static void yy_accept(yyParser*);  /* Forward Declaration */

/*
** Perform a reduce action and the shift that must immediately
** follow the reduce.
*/
static void yy_reduce(
  yyParser *yypParser,         /* The parser */
  int yyruleno                 /* Number of the rule by which to reduce */
){
  int yygoto;                     /* The next state */
  int yyact;                      /* The next action */
  YYMINORTYPE yygotominor;        /* The LHS of the rule reduced */
  yyStackEntry *yymsp;            /* The top of the parser's stack */
  int yysize;                     /* Amount to pop the stack */
  expressionParserARG_FETCH;
  yymsp = &yypParser->yystack[yypParser->yyidx];
#ifndef NDEBUG
  if( yyTraceFILE && yyruleno>=0 
        && yyruleno<(int)(sizeof(yyRuleName)/sizeof(yyRuleName[0])) ){
    yysize = yyRuleInfo[yyruleno].nrhs;
    fprintf(yyTraceFILE, "%sReduce [%s], go to state %d.\n", yyTracePrompt,
      yyRuleName[yyruleno], yymsp[-yysize].stateno);
  }
#endif /* NDEBUG */
  yygotominor = yyzerominor;

  switch( yyruleno ){
  /* Beginning here are the reduction cases.  A typical example
  ** follows:
  **   case 0:
  **  #line <lineno> <grammarfile>
  **     { ... }           // User supplied code
  **  #line <lineno> <thisfile>
  **     break;
  */
/********** Begin reduce actions **********************************************/
      case 0: /* input ::= booleanExpression */
#line 79 "expression.y"
{
   U_TRACE_NO_PARAM(0, "input ::= booleanExpression(yymsp[0].minor.yy3)")

   U_INTERNAL_ASSERT_POINTER(yymsp[0].minor.yy3)
   U_INTERNAL_ASSERT_POINTER(result)

   U_INTERNAL_DUMP("yymsp[0].minor.yy3 = %V result = %V", yymsp[0].minor.yy3->rep, result->rep)

   *result = *yymsp[0].minor.yy3;

   delete yymsp[0].minor.yy3;
}
#line 875 "expression.c"
        break;
      case 1: /* booleanExpression ::= booleanExpression booleanCond equalityExpression */
#line 92 "expression.y"
{
   U_TRACE_NO_PARAM(0, "booleanExpression(yygotominor.yy3) ::= booleanExpression(yymsp[-2].minor.yy3) booleanCond(yymsp[-1].minor.yy12) equalityExpression(yymsp[0].minor.yy3)")

   U_INTERNAL_ASSERT_POINTER(yymsp[-2].minor.yy3)
   U_INTERNAL_ASSERT_POINTER(yymsp[0].minor.yy3)

   U_INTERNAL_DUMP("yymsp[-2].minor.yy3 = %V yymsp[-1].minor.yy12 = %d yymsp[0].minor.yy3 = %V", yymsp[-2].minor.yy3->rep, yymsp[-1].minor.yy12, yymsp[0].minor.yy3->rep)

   bool Bbo = (yymsp[-2].minor.yy3->empty() == false),
        Dbo = (yymsp[0].minor.yy3->empty() == false),
         bo = (yymsp[-1].minor.yy12 == U_TK_AND ? Bbo && Dbo
                             : Bbo || Dbo);

   if (bo) U_NEW(UString, yygotominor.yy3, UString(*UString::str_true));
   else    U_NEW(UString, yygotominor.yy3, UString);

   delete yymsp[-2].minor.yy3;
   delete yymsp[0].minor.yy3;

   U_INTERNAL_DUMP("yygotominor.yy3 = %V", yygotominor.yy3->rep)
}
#line 900 "expression.c"
        break;
      case 2: /* booleanExpression ::= equalityExpression */
#line 114 "expression.y"
{
   U_TRACE_NO_PARAM(0, "booleanExpression(yygotominor.yy3) ::= equalityExpression(yymsp[0].minor.yy3)")

   U_INTERNAL_DUMP("yygotominor.yy3 = %p yymsp[0].minor.yy3 = %p", yygotominor.yy3, yymsp[0].minor.yy3)

   U_INTERNAL_ASSERT_POINTER(yymsp[0].minor.yy3)

   yygotominor.yy3 = yymsp[0].minor.yy3;

   U_INTERNAL_DUMP("yygotominor.yy3 = %V", yygotominor.yy3->rep)
}
#line 915 "expression.c"
        break;
      case 3: /* equalityExpression ::= equalityExpression equalityCond relationalExpression */
#line 126 "expression.y"
{
   U_TRACE_NO_PARAM(0, "equalityExpression(yygotominor.yy3) ::= equalityExpression(yymsp[-2].minor.yy3) equalityCond(yymsp[-1].minor.yy12) relationalExpression(yymsp[0].minor.yy3)")

   U_INTERNAL_ASSERT_POINTER(yymsp[-2].minor.yy3)
   U_INTERNAL_ASSERT_POINTER(yymsp[0].minor.yy3)

   U_INTERNAL_DUMP("yymsp[-2].minor.yy3 = %V yymsp[-1].minor.yy12 = %d yymsp[0].minor.yy3 = %V", yymsp[-2].minor.yy3->rep, yymsp[-1].minor.yy12, yymsp[0].minor.yy3->rep)

   bool bo = false,
       Bbo = (yymsp[-2].minor.yy3->empty() == false),
       Dbo = (yymsp[0].minor.yy3->empty() == false);

   int cmp = (Bbo && Dbo ? yymsp[-2].minor.yy3->compare(yymsp[0].minor.yy3->rep) : Bbo - Dbo);

   switch (yymsp[-1].minor.yy12)
      {
      case U_TK_EQ: bo = (cmp == 0); break;
      case U_TK_NE: bo = (cmp != 0); break;

      case U_TK_ENDS_WITH:    if (Bbo && Dbo) bo = UStringExt::endsWith(  *yymsp[-2].minor.yy3, *yymsp[0].minor.yy3); break;
      case U_TK_STARTS_WITH:  if (Bbo && Dbo) bo = UStringExt::startsWith(*yymsp[-2].minor.yy3, *yymsp[0].minor.yy3); break;
      case U_TK_CONTAINS:     if (Bbo && Dbo) bo = (yymsp[-2].minor.yy3->find(*yymsp[0].minor.yy3) != U_NOT_FOUND);    break;
      case U_TK_IS_PRESENT:   if (Bbo && Dbo) bo = Items(*yymsp[-2].minor.yy3).isContained(*yymsp[0].minor.yy3);       break;
      }

   U_INTERNAL_DUMP("bo = %b cmp = %d", bo, cmp)

   if (bo) U_NEW(UString, yygotominor.yy3, UString(*UString::str_true));
   else    U_NEW(UString, yygotominor.yy3, UString);

   delete yymsp[-2].minor.yy3;
   delete yymsp[0].minor.yy3;

   U_INTERNAL_DUMP("yygotominor.yy3 = %V", yygotominor.yy3->rep)
}
#line 954 "expression.c"
        break;
      case 4: /* equalityExpression ::= relationalExpression */
#line 162 "expression.y"
{
   U_TRACE_NO_PARAM(0, "equalityExpression(yygotominor.yy3) ::= relationalExpression(yymsp[0].minor.yy3)")

   U_INTERNAL_DUMP("yygotominor.yy3 = %p yymsp[0].minor.yy3 = %p", yygotominor.yy3, yymsp[0].minor.yy3)

   U_INTERNAL_ASSERT_POINTER(yymsp[0].minor.yy3)

   yygotominor.yy3 = yymsp[0].minor.yy3;

   U_INTERNAL_DUMP("yygotominor.yy3 = %V", yygotominor.yy3->rep)
}
#line 969 "expression.c"
        break;
      case 5: /* relationalExpression ::= relationalExpression relationalCond additiveExpression */
#line 174 "expression.y"
{
   U_TRACE_NO_PARAM(0, "relationalExpression(yygotominor.yy3) ::= relationalExpression(yymsp[-2].minor.yy3) relationalCond(yymsp[-1].minor.yy12) additiveExpression(yymsp[0].minor.yy3)")

   U_INTERNAL_ASSERT_POINTER(yymsp[-2].minor.yy3)
   U_INTERNAL_ASSERT_POINTER(yymsp[0].minor.yy3)

   U_INTERNAL_DUMP("yymsp[-2].minor.yy3 = %V yymsp[-1].minor.yy12 = %d yymsp[0].minor.yy3 = %V", yymsp[-2].minor.yy3->rep, yymsp[-1].minor.yy12, yymsp[0].minor.yy3->rep)

   bool bo = false,
       Bbo = (yymsp[-2].minor.yy3->empty() == false),
       Dbo = (yymsp[0].minor.yy3->empty() == false);

   int cmp = (Bbo && Dbo ? yymsp[-2].minor.yy3->compare(yymsp[0].minor.yy3->rep) : Bbo - Dbo);

   switch (yymsp[-1].minor.yy12)
      {
      case U_TK_LT: bo = (cmp <  0); break;
      case U_TK_LE: bo = (cmp <= 0); break;
      case U_TK_GT: bo = (cmp >  0); break;
      case U_TK_GE: bo = (cmp >= 0); break;
      }

   U_INTERNAL_DUMP("bo = %b cmp = %d", bo, cmp)

   if (bo) U_NEW(UString, yygotominor.yy3, UString(*UString::str_true));
   else    U_NEW(UString, yygotominor.yy3, UString);

   delete yymsp[-2].minor.yy3;
   delete yymsp[0].minor.yy3;

   U_INTERNAL_DUMP("yygotominor.yy3 = %V", yygotominor.yy3->rep)
}
#line 1005 "expression.c"
        break;
      case 6: /* relationalExpression ::= additiveExpression */
#line 207 "expression.y"
{
   U_TRACE_NO_PARAM(0, "relationalExpression(yygotominor.yy3) ::= additiveExpression(yymsp[0].minor.yy3)")

   U_INTERNAL_DUMP("yygotominor.yy3 = %p yymsp[0].minor.yy3 = %p", yygotominor.yy3, yymsp[0].minor.yy3)

   U_INTERNAL_ASSERT_POINTER(yymsp[0].minor.yy3)

   yygotominor.yy3 = yymsp[0].minor.yy3;

   U_INTERNAL_DUMP("yygotominor.yy3 = %V", yygotominor.yy3->rep)
}
#line 1020 "expression.c"
        break;
      case 7: /* additiveExpression ::= additiveExpression additiveCond multiplicativeExpression */
#line 219 "expression.y"
{
   U_TRACE_NO_PARAM(0, "additiveExpression(yygotominor.yy3) ::= additiveExpression(yymsp[-2].minor.yy3) additiveCond(yymsp[-1].minor.yy12) multiplicativeExpression(yymsp[0].minor.yy3)")

   U_INTERNAL_ASSERT_POINTER(yymsp[-2].minor.yy3)
   U_INTERNAL_ASSERT_POINTER(yymsp[0].minor.yy3)

   U_INTERNAL_DUMP("yymsp[-2].minor.yy3 = %V yymsp[-1].minor.yy12 = %d yymsp[0].minor.yy3 = %V", yymsp[-2].minor.yy3->rep, yymsp[-1].minor.yy12, yymsp[0].minor.yy3->rep)

   long Blo = yymsp[-2].minor.yy3->strtol(),
        Dlo = yymsp[0].minor.yy3->strtol(),
         lo = (yymsp[-1].minor.yy12 == U_TK_PLUS ? Blo + Dlo
                              : Blo - Dlo);

   U_NEW(UString, yygotominor.yy3, UString(UStringExt::stringFromNumber(lo)));

   delete yymsp[-2].minor.yy3;
   delete yymsp[0].minor.yy3;

   U_INTERNAL_DUMP("yygotominor.yy3 = %V", yygotominor.yy3->rep)
}
#line 1044 "expression.c"
        break;
      case 8: /* additiveExpression ::= multiplicativeExpression */
#line 240 "expression.y"
{
   U_TRACE_NO_PARAM(0, "additiveExpression(yygotominor.yy3) ::= multiplicativeExpression(yymsp[0].minor.yy3)")

   U_INTERNAL_DUMP("yygotominor.yy3 = %p yymsp[0].minor.yy3 = %p", yygotominor.yy3, yymsp[0].minor.yy3)

   U_INTERNAL_ASSERT_POINTER(yymsp[0].minor.yy3)

   yygotominor.yy3 = yymsp[0].minor.yy3;

   U_INTERNAL_DUMP("yygotominor.yy3 = %V", yygotominor.yy3->rep)
}
#line 1059 "expression.c"
        break;
      case 9: /* multiplicativeExpression ::= multiplicativeExpression multiplicativeCond unaryExpression */
#line 252 "expression.y"
{
   U_TRACE_NO_PARAM(0, "multiplicativeExpression(yygotominor.yy3) ::= multiplicativeExpression(yymsp[-2].minor.yy3) multiplicativeCond(yymsp[-1].minor.yy12) unaryExpression(yymsp[0].minor.yy3)")

   U_INTERNAL_ASSERT_POINTER(yymsp[-2].minor.yy3)
   U_INTERNAL_ASSERT_POINTER(yymsp[0].minor.yy3)

   U_INTERNAL_DUMP("yymsp[-2].minor.yy3 = %V yymsp[-1].minor.yy12 = %d yymsp[0].minor.yy3 = %V", yymsp[-2].minor.yy3->rep, yymsp[-1].minor.yy12, yymsp[0].minor.yy3->rep)

   long lo,
        Blo = yymsp[-2].minor.yy3->strtol(),
        Dlo = yymsp[0].minor.yy3->strtol();

   if (Dlo == 0) U_NEW(UString, yygotominor.yy3, UString(*UString::str_zero));
   else
      {
      lo = (yymsp[-1].minor.yy12 == U_TK_MULT ? Blo * Dlo :
            yymsp[-1].minor.yy12 == U_TK_DIV  ? Blo / Dlo :
                             Blo % Dlo);

      U_NEW(UString, yygotominor.yy3, UString(UStringExt::stringFromNumber(lo)));
      }

   delete yymsp[-2].minor.yy3;
   delete yymsp[0].minor.yy3;

   U_INTERNAL_DUMP("yygotominor.yy3 = %V", yygotominor.yy3->rep)
}
#line 1090 "expression.c"
        break;
      case 10: /* multiplicativeExpression ::= unaryExpression */
#line 280 "expression.y"
{
   U_TRACE_NO_PARAM(0, "multiplicativeExpression(yygotominor.yy3) ::= unaryExpression(yymsp[0].minor.yy3)")

   U_INTERNAL_DUMP("yygotominor.yy3 = %p yymsp[0].minor.yy3 = %p", yygotominor.yy3, yymsp[0].minor.yy3)

   U_INTERNAL_ASSERT_POINTER(yymsp[0].minor.yy3)

   yygotominor.yy3 = yymsp[0].minor.yy3;

   U_INTERNAL_DUMP("yygotominor.yy3 = %V", yygotominor.yy3->rep)
}
#line 1105 "expression.c"
        break;
      case 11: /* unaryExpression ::= NOT primaryExpression */
#line 292 "expression.y"
{
   U_TRACE_NO_PARAM(0, "unaryExpression(yygotominor.yy3) ::= NOT primaryExpression(yymsp[0].minor.yy3)")

   U_INTERNAL_DUMP("yygotominor.yy3 = %p yymsp[0].minor.yy3 = %p", yygotominor.yy3, yymsp[0].minor.yy3)

   U_INTERNAL_ASSERT_POINTER(yymsp[0].minor.yy3)

   if (yymsp[0].minor.yy3->empty()) U_NEW(UString, yygotominor.yy3, UString(*UString::str_true));
   else            U_NEW(UString, yygotominor.yy3, UString);

   delete yymsp[0].minor.yy3;

   U_INTERNAL_DUMP("yygotominor.yy3 = %V", yygotominor.yy3->rep)
  yy_destructor(yypParser,18,&yymsp[-1].minor);
}
#line 1124 "expression.c"
        break;
      case 12: /* unaryExpression ::= primaryExpression */
#line 307 "expression.y"
{
   U_TRACE_NO_PARAM(0, "unaryExpression(yygotominor.yy3) ::= primaryExpression(yymsp[0].minor.yy3)")

   U_INTERNAL_DUMP("yygotominor.yy3 = %p yymsp[0].minor.yy3 = %p", yygotominor.yy3, yymsp[0].minor.yy3)

   U_INTERNAL_ASSERT_POINTER(yymsp[0].minor.yy3)

   yygotominor.yy3 = yymsp[0].minor.yy3;

   U_INTERNAL_DUMP("yygotominor.yy3 = %V", yygotominor.yy3->rep)
}
#line 1139 "expression.c"
        break;
      case 13: /* primaryExpression ::= LPAREN booleanExpression RPAREN */
#line 319 "expression.y"
{
   U_TRACE_NO_PARAM(0, "primaryExpression(yygotominor.yy3) ::= LPAREN booleanExpression(yymsp[-1].minor.yy3) RPAREN")

   U_INTERNAL_DUMP("yygotominor.yy3 = %p yymsp[-1].minor.yy3 = %p", yygotominor.yy3, yymsp[-1].minor.yy3)

   U_INTERNAL_ASSERT_POINTER(yymsp[-1].minor.yy3)

   yygotominor.yy3 = yymsp[-1].minor.yy3;

   U_INTERNAL_DUMP("yygotominor.yy3 = %V", yygotominor.yy3->rep)
  yy_destructor(yypParser,20,&yymsp[-2].minor);
  yy_destructor(yypParser,21,&yymsp[0].minor);
}
#line 1156 "expression.c"
        break;
      case 14: /* primaryExpression ::= value */
#line 331 "expression.y"
{
   U_TRACE_NO_PARAM(0, "primaryExpression(yygotominor.yy3) ::= value(yymsp[0].minor.yy3)")

   U_INTERNAL_DUMP("yygotominor.yy3 = %p yymsp[0].minor.yy3 = %p", yygotominor.yy3, yymsp[0].minor.yy3)

   U_INTERNAL_ASSERT_POINTER(yymsp[0].minor.yy3)

   yygotominor.yy3 = yymsp[0].minor.yy3;

   U_INTERNAL_DUMP("yygotominor.yy3 = %V", yygotominor.yy3->rep)
}
#line 1171 "expression.c"
        break;
      case 15: /* value ::= VALUE */
#line 343 "expression.y"
{
   U_TRACE_NO_PARAM(0, "value(yygotominor.yy3) ::= VALUE(yymsp[0].minor.yy0)")

   U_INTERNAL_DUMP("yygotominor.yy3 = %p yymsp[0].minor.yy0 = %p", yygotominor.yy3, yymsp[0].minor.yy0)

   U_INTERNAL_ASSERT_POINTER(yymsp[0].minor.yy0)

   yygotominor.yy3 = yymsp[0].minor.yy0;

   U_INTERNAL_DUMP("yygotominor.yy3 = %V", yygotominor.yy3->rep)
}
#line 1186 "expression.c"
        break;
      case 16: /* value ::= value VALUE */
#line 355 "expression.y"
{
   U_TRACE_NO_PARAM(0, "value(yygotominor.yy3) ::= value(yymsp[-1].minor.yy3) VALUE(yymsp[0].minor.yy0)")

   U_INTERNAL_ASSERT_POINTER(yymsp[-1].minor.yy3)
   U_INTERNAL_ASSERT_POINTER(yymsp[0].minor.yy0)

   U_INTERNAL_DUMP("yymsp[-1].minor.yy3 = %V yymsp[0].minor.yy0 = %V", yymsp[-1].minor.yy3->rep, yymsp[0].minor.yy0->rep)

    yygotominor.yy3  =  yymsp[-1].minor.yy3;
   *yygotominor.yy3 += *yymsp[0].minor.yy0;

   delete yymsp[0].minor.yy0;

   U_INTERNAL_DUMP("yygotominor.yy3 = %V", yygotominor.yy3->rep)
}
#line 1205 "expression.c"
        break;
      case 17: /* booleanCond ::= OR */
#line 371 "expression.y"
{
   U_TRACE_NO_PARAM(0, "booleanCond(yygotominor.yy12) ::= OR")
   yygotominor.yy12 = U_TK_OR;
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 1214 "expression.c"
        break;
      case 18: /* booleanCond ::= AND */
#line 375 "expression.y"
{
   U_TRACE_NO_PARAM(0, "booleanCond(yygotominor.yy12) ::= AND")
   yygotominor.yy12 = U_TK_AND;
  yy_destructor(yypParser,1,&yymsp[0].minor);
}
#line 1223 "expression.c"
        break;
      case 19: /* equalityCond ::= EQ */
#line 379 "expression.y"
{
   U_TRACE_NO_PARAM(0, "equalityCond(yygotominor.yy12) ::= EQ")
   yygotominor.yy12 = U_TK_EQ;
  yy_destructor(yypParser,3,&yymsp[0].minor);
}
#line 1232 "expression.c"
        break;
      case 20: /* equalityCond ::= NE */
#line 383 "expression.y"
{
   U_TRACE_NO_PARAM(0, "equalityCond(yygotominor.yy12) ::= NE")
   yygotominor.yy12 = U_TK_NE;
  yy_destructor(yypParser,4,&yymsp[0].minor);
}
#line 1241 "expression.c"
        break;
      case 21: /* equalityCond ::= STARTS_WITH */
#line 387 "expression.y"
{
   U_TRACE_NO_PARAM(0, "equalityCond(yygotominor.yy12) ::= STARTS_WITH")
   yygotominor.yy12 = U_TK_STARTS_WITH;
  yy_destructor(yypParser,9,&yymsp[0].minor);
}
#line 1250 "expression.c"
        break;
      case 22: /* equalityCond ::= ENDS_WITH */
#line 391 "expression.y"
{
   U_TRACE_NO_PARAM(0, "equalityCond(yygotominor.yy12) ::= ENDS_WITH")
   yygotominor.yy12 = U_TK_ENDS_WITH;
  yy_destructor(yypParser,10,&yymsp[0].minor);
}
#line 1259 "expression.c"
        break;
      case 23: /* equalityCond ::= IS_PRESENT */
#line 395 "expression.y"
{
   U_TRACE_NO_PARAM(0, "equalityCond(yygotominor.yy12) ::= IS_PRESENT")
   yygotominor.yy12 = U_TK_IS_PRESENT;
  yy_destructor(yypParser,11,&yymsp[0].minor);
}
#line 1268 "expression.c"
        break;
      case 24: /* equalityCond ::= CONTAINS */
#line 399 "expression.y"
{
   U_TRACE_NO_PARAM(0, "equalityCond(yygotominor.yy12) ::= CONTAINS")
   yygotominor.yy12 = U_TK_CONTAINS;
  yy_destructor(yypParser,12,&yymsp[0].minor);
}
#line 1277 "expression.c"
        break;
      case 25: /* relationalCond ::= GT */
#line 403 "expression.y"
{
   U_TRACE_NO_PARAM(0, "relationalCond(yygotominor.yy12) ::= GT")
   yygotominor.yy12 = U_TK_GT;
  yy_destructor(yypParser,5,&yymsp[0].minor);
}
#line 1286 "expression.c"
        break;
      case 26: /* relationalCond ::= GE */
#line 407 "expression.y"
{
   U_TRACE_NO_PARAM(0, "relationalCond(yygotominor.yy12) ::= GE")
   yygotominor.yy12 = U_TK_GE;
  yy_destructor(yypParser,6,&yymsp[0].minor);
}
#line 1295 "expression.c"
        break;
      case 27: /* relationalCond ::= LT */
#line 411 "expression.y"
{
   U_TRACE_NO_PARAM(0, "relationalCond(yygotominor.yy12) ::= LT")
   yygotominor.yy12 = U_TK_LT;
  yy_destructor(yypParser,7,&yymsp[0].minor);
}
#line 1304 "expression.c"
        break;
      case 28: /* relationalCond ::= LE */
#line 415 "expression.y"
{
   U_TRACE_NO_PARAM(0, "relationalCond(yygotominor.yy12) ::= LE")
   yygotominor.yy12 = U_TK_LE;
  yy_destructor(yypParser,8,&yymsp[0].minor);
}
#line 1313 "expression.c"
        break;
      case 29: /* additiveCond ::= PLUS */
#line 419 "expression.y"
{
   U_TRACE_NO_PARAM(0, "additiveCond(yygotominor.yy12) ::= PLUS")
   yygotominor.yy12 = U_TK_PLUS;
  yy_destructor(yypParser,13,&yymsp[0].minor);
}
#line 1322 "expression.c"
        break;
      case 30: /* additiveCond ::= MINUS */
#line 423 "expression.y"
{
   U_TRACE_NO_PARAM(0, "additiveCond(yygotominor.yy12) ::= MINUS")
   yygotominor.yy12 = U_TK_MINUS;
  yy_destructor(yypParser,14,&yymsp[0].minor);
}
#line 1331 "expression.c"
        break;
      case 31: /* multiplicativeCond ::= MULT */
#line 427 "expression.y"
{
   U_TRACE_NO_PARAM(0, "multiplicativeCond(yygotominor.yy12) ::= MULT")
   yygotominor.yy12 = U_TK_MULT;
  yy_destructor(yypParser,15,&yymsp[0].minor);
}
#line 1340 "expression.c"
        break;
      case 32: /* multiplicativeCond ::= DIV */
#line 431 "expression.y"
{
   U_TRACE(0, "multiplicativeCond(yygotominor.yy12) ::= DIV")
   yygotominor.yy12 = U_TK_DIV;
  yy_destructor(yypParser,16,&yymsp[0].minor);
}
#line 1349 "expression.c"
        break;
      case 33: /* multiplicativeCond ::= MOD */
#line 435 "expression.y"
{
   U_TRACE_NO_PARAM(0, "multiplicativeCond(yygotominor.yy12) ::= MOD")
   yygotominor.yy12 = U_TK_MOD;
  yy_destructor(yypParser,17,&yymsp[0].minor);
}
#line 1358 "expression.c"
        break;
      case 34: /* primaryExpression ::= FN_CALL LPAREN RPAREN */
#line 440 "expression.y"
{
   U_TRACE_NO_PARAM(0, "primaryExpression(yygotominor.yy3) ::= FN_CALL(yymsp[-2].minor.yy0) LPAREN RPAREN")

   U_INTERNAL_ASSERT_POINTER(yymsp[-2].minor.yy0)

   U_INTERNAL_DUMP("yygotominor.yy3 = %p yymsp[-2].minor.yy0 = %V", yygotominor.yy3, yymsp[-2].minor.yy0->rep)

   long lo;
#ifdef __MINGW32__
   lPFv addr = (lPFv) ::GetProcAddress((HMODULE)0, yymsp[-2].minor.yy0->c_str());
#else
   lPFv addr = (lPFv) U_SYSCALL(dlsym, "%p,%S", RTLD_DEFAULT, yymsp[-2].minor.yy0->c_str());
#endif

   if (addr == 0) U_NEW(UString, yygotominor.yy3, UString);
   else
      {
      lo = (*addr)();

      U_NEW(UString, yygotominor.yy3, UString(UStringExt::stringFromNumber(lo)));
      }

   delete yymsp[-2].minor.yy0;
  yy_destructor(yypParser,20,&yymsp[-1].minor);
  yy_destructor(yypParser,21,&yymsp[0].minor);
}
#line 1388 "expression.c"
        break;
      case 35: /* primaryExpression ::= FN_CALL LPAREN params RPAREN */
#line 465 "expression.y"
{
   U_TRACE_NO_PARAM(0, "primaryExpression(yygotominor.yy3) ::= FN_CALL(yymsp[-3].minor.yy0) LPAREN params(yymsp[-1].minor.yy49) RPAREN")

   U_INTERNAL_ASSERT_POINTER(yymsp[-3].minor.yy0)
   U_INTERNAL_ASSERT_POINTER(yymsp[-1].minor.yy49)

   U_INTERNAL_DUMP("yymsp[-3].minor.yy0 = %V yymsp[-1].minor.yy49 = %p", yymsp[-3].minor.yy0->rep, yymsp[-1].minor.yy49)

   long lo, lo0, lo1;
#ifdef __MINGW32__
   lPFll addr = (lPFll) ::GetProcAddress((HMODULE)0, yymsp[-3].minor.yy0->c_str());
#else
   lPFll addr = (lPFll) U_SYSCALL(dlsym, "%p,%S", RTLD_DEFAULT, yymsp[-3].minor.yy0->c_str());
#endif

   if (addr == 0) U_NEW(UString, yygotominor.yy3, UString);
   else
      {
      lo0 = (*yymsp[-1].minor.yy49)[0].strtol();
      lo1 = (*yymsp[-1].minor.yy49)[1].strtol();
      lo  = (*addr)(lo0, lo1);
      
      U_NEW(UString, yygotominor.yy3, UString(UStringExt::stringFromNumber(lo)));
      }

   delete yymsp[-3].minor.yy0;
  yy_destructor(yypParser,20,&yymsp[-2].minor);
  yy_destructor(yypParser,21,&yymsp[0].minor);
}
#line 1421 "expression.c"
        break;
      case 36: /* params ::= VALUE */
#line 493 "expression.y"
{
   U_TRACE_NO_PARAM(0, "params(yygotominor.yy49) ::= VALUE(yymsp[0].minor.yy0)")

   U_INTERNAL_ASSERT_POINTER(yymsp[0].minor.yy0)

   U_INTERNAL_DUMP("yygotominor.yy49 = %p yymsp[0].minor.yy0 = %V", yygotominor.yy49, yymsp[0].minor.yy0->rep)

   U_NEW(Items, yygotominor.yy49, Items);

   yygotominor.yy49->push_back(*yymsp[0].minor.yy0);

   delete yymsp[0].minor.yy0;
}
#line 1438 "expression.c"
        break;
      case 37: /* params ::= params COMMA VALUE */
#line 507 "expression.y"
{
   U_TRACE_NO_PARAM(0, "params(yygotominor.yy49) ::= params(yymsp[-2].minor.yy49) COMMA VALUE(yymsp[0].minor.yy0)")

   U_INTERNAL_ASSERT_POINTER(yymsp[-2].minor.yy49)
   U_INTERNAL_ASSERT_POINTER(yymsp[0].minor.yy0)

   U_INTERNAL_DUMP("yymsp[-2].minor.yy49 = %p yymsp[0].minor.yy0 = %V", yymsp[-2].minor.yy49, yymsp[0].minor.yy0->rep)

   yymsp[-2].minor.yy49->push_back(*yymsp[0].minor.yy0);

   yygotominor.yy49 = yymsp[-2].minor.yy49;

   delete yymsp[0].minor.yy0;
  yy_destructor(yypParser,23,&yymsp[-1].minor);
}
#line 1457 "expression.c"
        break;
      default:
        break;
/********** End reduce actions ************************************************/
  };
  yygoto = yyRuleInfo[yyruleno].lhs;
  yysize = yyRuleInfo[yyruleno].nrhs;
  yypParser->yyidx -= yysize;
  yyact = yy_find_reduce_action(yymsp[-yysize].stateno,(YYCODETYPE)yygoto);
  if( yyact <= YY_MAX_SHIFTREDUCE ){
    if( yyact>YY_MAX_SHIFT ) yyact += YY_MIN_REDUCE - YY_MIN_SHIFTREDUCE;
    /* If the reduce action popped at least
    ** one element off the stack, then we can push the new element back
    ** onto the stack here, and skip the stack overflow test in yy_shift().
    ** That gives a significant speed improvement. */
    if( yysize ){
      yypParser->yyidx++;
      yymsp -= yysize-1;
      yymsp->stateno = (YYACTIONTYPE)yyact;
      yymsp->major = (YYCODETYPE)yygoto;
      yymsp->minor = yygotominor;
      yyTraceShift(yypParser, yyact);
    }else{
      yy_shift(yypParser,yyact,yygoto,&yygotominor);
    }
  }else{
    assert( yyact == YY_ACCEPT_ACTION );
    yy_accept(yypParser);
  }
}

/*
** The following code executes when the parse fails
*/
#ifndef YYNOERRORRECOVERY
static void yy_parse_failed(
  yyParser *yypParser           /* The parser */
){
  expressionParserARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sFail!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser fails */
/************ Begin %parse_failure code ***************************************/
#line 44 "expression.y"

   U_TRACE_NO_PARAM(0, "::parse_failure()")
   U_INTERNAL_ASSERT_POINTER(result)
   result->clear();
   U_WARNING("Parse failure!");
#line 1513 "expression.c"
/************ End %parse_failure code *****************************************/
  expressionParserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}
#endif /* YYNOERRORRECOVERY */

/*
** The following code executes when a syntax error first occurs.
*/
static void yy_syntax_error(
  yyParser *yypParser,           /* The parser */
  int yymajor,                   /* The major type of the error token */
  YYMINORTYPE yyminor            /* The minor type of the error token */
){
  expressionParserARG_FETCH;
#define TOKEN (yyminor.yy0)
/************ Begin %syntax_error code ****************************************/
#line 50 "expression.y"

   U_TRACE_NO_PARAM(0, "::syntax_error()")
   U_INTERNAL_ASSERT_POINTER(result)
   result->clear();
   U_WARNING("Syntax error!");
#line 1536 "expression.c"
/************ End %syntax_error code ******************************************/
  expressionParserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following is executed when the parser accepts
*/
static void yy_accept(
  yyParser *yypParser           /* The parser */
){
  expressionParserARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sAccept!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser accepts */
/*********** Begin %parse_accept code *****************************************/
#line 39 "expression.y"

   U_TRACE_NO_PARAM(0, "::parse_accept()")
   U_INTERNAL_ASSERT_POINTER(result)
   U_INTERNAL_DUMP("result = %V", result->rep)
#line 1562 "expression.c"
/*********** End %parse_accept code *******************************************/
  expressionParserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/* The main parser program.
** The first argument is a pointer to a structure obtained from
** "expressionParserAlloc" which describes the current state of the parser.
** The second argument is the major token number.  The third is
** the minor token.  The fourth optional argument is whatever the
** user wants (and specified in the grammar) and is available for
** use by the action routines.
**
** Inputs:
** <ul>
** <li> A pointer to the parser (an opaque structure.)
** <li> The major token number.
** <li> The minor token number.
** <li> An option argument of a grammar-specified type.
** </ul>
**
** Outputs:
** None.
*/
void expressionParser(
  void *yyp,                   /* The parser */
  int yymajor,                 /* The major token code number */
  expressionParserTOKENTYPE yyminor       /* The value for the token */
  expressionParserARG_PDECL               /* Optional %extra_argument parameter */
){
  YYMINORTYPE yyminorunion;
  int yyact;            /* The parser action. */
#if !defined(YYERRORSYMBOL) && !defined(YYNOERRORRECOVERY)
  int yyendofinput;     /* True if we are at the end of input */
#endif
#ifdef YYERRORSYMBOL
  int yyerrorhit = 0;   /* True if yymajor has invoked an error */
#endif
  yyParser *yypParser;  /* The parser */

  /* (re)initialize the parser, if necessary */
  yypParser = (yyParser*)yyp;
  if( yypParser->yyidx<0 ){
#if YYSTACKDEPTH<=0
    if( yypParser->yystksz <=0 ){
      /*memset(&yyminorunion, 0, sizeof(yyminorunion));*/
      yyminorunion = yyzerominor;
      yyStackOverflow(yypParser, &yyminorunion);
      return;
    }
#endif
    yypParser->yyidx = 0;
    yypParser->yyerrcnt = -1;
    yypParser->yystack[0].stateno = 0;
    yypParser->yystack[0].major = 0;
#ifndef NDEBUG
    if( yyTraceFILE ){
      fprintf(yyTraceFILE,"%sInitialize. Empty stack. State 0\n",
              yyTracePrompt);
    }
#endif
  }
  yyminorunion.yy0 = yyminor;
#if !defined(YYERRORSYMBOL) && !defined(YYNOERRORRECOVERY)
  yyendofinput = (yymajor==0);
#endif
  expressionParserARG_STORE;

#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sInput '%s'\n",yyTracePrompt,yyTokenName[yymajor]);
  }
#endif

  do{
    yyact = yy_find_shift_action(yypParser,(YYCODETYPE)yymajor);
    if( yyact <= YY_MAX_SHIFTREDUCE ){
      if( yyact > YY_MAX_SHIFT ) yyact += YY_MIN_REDUCE - YY_MIN_SHIFTREDUCE;
      yy_shift(yypParser,yyact,yymajor,&yyminorunion);
      yypParser->yyerrcnt--;
      yymajor = YYNOCODE;
    }else if( yyact <= YY_MAX_REDUCE ){
      yy_reduce(yypParser,yyact-YY_MIN_REDUCE);
    }else{
      assert( yyact == YY_ERROR_ACTION );
#ifdef YYERRORSYMBOL
      int yymx;
#endif
#ifndef NDEBUG
      if( yyTraceFILE ){
        fprintf(yyTraceFILE,"%sSyntax Error!\n",yyTracePrompt);
      }
#endif
#ifdef YYERRORSYMBOL
      /* A syntax error has occurred.
      ** The response to an error depends upon whether or not the
      ** grammar defines an error token "ERROR".  
      **
      ** This is what we do if the grammar does define ERROR:
      **
      **  * Call the %syntax_error function.
      **
      **  * Begin popping the stack until we enter a state where
      **    it is legal to shift the error symbol, then shift
      **    the error symbol.
      **
      **  * Set the error count to three.
      **
      **  * Begin accepting and shifting new tokens.  No new error
      **    processing will occur until three tokens have been
      **    shifted successfully.
      **
      */
      if( yypParser->yyerrcnt<0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      yymx = yypParser->yystack[yypParser->yyidx].major;
      if( yymx==YYERRORSYMBOL || yyerrorhit ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE,"%sDiscard input token %s\n",
             yyTracePrompt,yyTokenName[yymajor]);
        }
#endif
        yy_destructor(yypParser, (YYCODETYPE)yymajor,&yyminorunion);
        yymajor = YYNOCODE;
      }else{
         while(
          yypParser->yyidx >= 0 &&
          yymx != YYERRORSYMBOL &&
          (yyact = yy_find_reduce_action(
                        yypParser->yystack[yypParser->yyidx].stateno,
                        YYERRORSYMBOL)) >= YY_MIN_REDUCE
        ){
          yy_pop_parser_stack(yypParser);
        }
        if( yypParser->yyidx < 0 || yymajor==0 ){
          yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
          yy_parse_failed(yypParser);
          yymajor = YYNOCODE;
        }else if( yymx!=YYERRORSYMBOL ){
          YYMINORTYPE u2;
          u2.YYERRSYMDT = 0;
          yy_shift(yypParser,yyact,YYERRORSYMBOL,&u2);
        }
      }
      yypParser->yyerrcnt = 3;
      yyerrorhit = 1;
#elif defined(YYNOERRORRECOVERY)
      /* If the YYNOERRORRECOVERY macro is defined, then do not attempt to
      ** do any kind of error recovery.  Instead, simply invoke the syntax
      ** error routine and continue going as if nothing had happened.
      **
      ** Applications can set this macro (for example inside %include) if
      ** they intend to abandon the parse upon the first syntax error seen.
      */
      yy_syntax_error(yypParser,yymajor,yyminorunion);
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      yymajor = YYNOCODE;
      
#else  /* YYERRORSYMBOL is not defined */
      /* This is what we do if the grammar does not define ERROR:
      **
      **  * Report an error message, and throw away the input token.
      **
      **  * If the input token is $, then fail the parse.
      **
      ** As before, subsequent error messages are suppressed until
      ** three input tokens have been successfully shifted.
      */
      if( yypParser->yyerrcnt<=0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      yypParser->yyerrcnt = 3;
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      if( yyendofinput ){
        yy_parse_failed(yypParser);
      }
      yymajor = YYNOCODE;
#endif
    }
  }while( yymajor!=YYNOCODE && yypParser->yyidx>=0 );
#ifndef NDEBUG
  if( yyTraceFILE ){
    int i;
    fprintf(yyTraceFILE,"%sReturn. Stack=",yyTracePrompt);
    for(i=1; i<=yypParser->yyidx; i++)
      fprintf(yyTraceFILE,"%c%s", i==1 ? '[' : ' ', 
              yyTokenName[yypParser->yystack[i].major]);
    fprintf(yyTraceFILE,"]\n");
  }
#endif
  return;
}
