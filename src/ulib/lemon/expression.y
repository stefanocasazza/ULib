%include {
#include <ulib/tokenizer.h>
#include <ulib/dynamic/dynamic.h>
#include <ulib/utility/string_ext.h>

typedef long (*lPFv)(void);
typedef long (*lPFll)(long, long);

typedef UVector<UString> Items;

extern void* expressionParserAlloc(void* (*mallocProc)(size_t));
extern void  expressionParserFree(void* p, void (*freeProc)(void*));
extern void  expressionParserTrace(FILE* stream, char* zPrefix);
extern void  expressionParser(void* yyp, int yymajor, UString* yyminor, UString* result);

extern void token_destructor(UString* token);

void token_destructor(UString* token) {
	U_TRACE(0, "::token_destructor(%.*S)", U_STRING_TO_TRACE(*token))

	delete token;
}

} /* %include */

%name expressionParser
%extra_argument { UString* result }

%token_prefix U_TK_
%token_type { UString* }
%token_destructor { token_destructor($$); }

%destructor params {
	U_TRACE(0, "::params_destructor(%p)", $$)
	delete $$;
}

%parse_accept {
	U_TRACE(0, "::parse_accept()")
	U_INTERNAL_ASSERT_POINTER(result)
	U_INTERNAL_DUMP("result = %.*S", U_STRING_TO_TRACE(*result))
}
%parse_failure {
	U_TRACE(0, "::parse_failure()")
	U_INTERNAL_ASSERT_POINTER(result)
	result->clear();
	U_WARNING("Parse failure!");
}
%syntax_error {
	U_TRACE(0, "::syntax_error()")
	U_INTERNAL_ASSERT_POINTER(result)
	result->clear();
	U_WARNING("Syntax error!");
}
%stack_overflow {
	U_TRACE(0, "::stack_overflow()")
	U_WARNING("Parse stack overflow");
}

%left AND.
%left OR.
%nonassoc EQ NE GT GE LT LE STARTS_WITH ENDS_WITH IS_PRESENT CONTAINS.
%left PLUS MINUS.
%left MULT DIV MOD.
%right NOT.
%nonassoc FN_CALL .

%default_type { UString* }

%type params { Items* }

%type booleanCond { int }
%type equalityCond { int }
%type relationalCond { int }
%type additiveCond { int }
%type multiplicativeCond { int }

input ::= booleanExpression(B). {
	U_TRACE(0, "input ::= booleanExpression(B)")

	U_INTERNAL_ASSERT_POINTER(B)
	U_INTERNAL_ASSERT_POINTER(result)

	U_INTERNAL_DUMP("B = %.*S result = %.*S", U_STRING_TO_TRACE(*B), U_STRING_TO_TRACE(*result))

	*result = *B;

	delete B;
}

booleanExpression(A) ::= booleanExpression(B) booleanCond(C) equalityExpression(D). {
   U_TRACE(0, "booleanExpression(A) ::= booleanExpression(B) booleanCond(C) equalityExpression(D)")

   U_INTERNAL_ASSERT_POINTER(B)
   U_INTERNAL_ASSERT_POINTER(D)

	U_INTERNAL_DUMP("B = %.*S C = %d D = %.*S", U_STRING_TO_TRACE(*B), C, U_STRING_TO_TRACE(*D))

   bool Bbo = (B->empty() == false),
        Dbo = (D->empty() == false),
		   bo = (C == U_TK_AND ? Bbo && Dbo
									  : Bbo || Dbo);

	A = (bo ? U_NEW(UString(U_CONSTANT_TO_PARAM("true"))) : U_NEW(UString));

	delete B;
	delete D;

	U_INTERNAL_DUMP("A = %.*S", U_STRING_TO_TRACE(*A))
}

booleanExpression(A) ::= equalityExpression(B). {
	U_TRACE(0, "booleanExpression(A) ::= equalityExpression(B)")

	U_INTERNAL_DUMP("A = %p B = %p", A, B)

	U_INTERNAL_ASSERT_POINTER(B)

	A = B;

	U_INTERNAL_DUMP("A = %.*S", U_STRING_TO_TRACE(*A))
}

equalityExpression(A) ::= equalityExpression(B) equalityCond(C) relationalExpression(D). {
   U_TRACE(0, "equalityExpression(A) ::= equalityExpression(B) equalityCond(C) relationalExpression(D)")

   U_INTERNAL_ASSERT_POINTER(B)
   U_INTERNAL_ASSERT_POINTER(D)

	U_INTERNAL_DUMP("B = %.*S C = %d D = %.*S", U_STRING_TO_TRACE(*B), C, U_STRING_TO_TRACE(*D))

   bool bo = false,
		 Bbo = (B->empty() == false),
		 Dbo = (D->empty() == false);

	int cmp = (Bbo && Dbo ? B->compare(D->rep) : Bbo - Dbo);

   switch (C)
		{
		case U_TK_EQ: bo = (cmp == 0); break;
		case U_TK_NE: bo = (cmp != 0); break;

		case U_TK_ENDS_WITH:		if (Bbo && Dbo) bo = UStringExt::endsWith(  *B, *D); break;
		case U_TK_STARTS_WITH:	if (Bbo && Dbo) bo = UStringExt::startsWith(*B, *D); break;
		case U_TK_CONTAINS:		if (Bbo && Dbo) bo = (B->find(*D) != U_NOT_FOUND);	  break;
		case U_TK_IS_PRESENT:	if (Bbo && Dbo) bo = Items(*B).isContained(*D);		  break;
		}

   U_INTERNAL_DUMP("bo = %b cmp = %d", bo, cmp)

	A = (bo ? U_NEW(UString(U_CONSTANT_TO_PARAM("true"))) : U_NEW(UString));

	delete B;
	delete D;

	U_INTERNAL_DUMP("A = %.*S", U_STRING_TO_TRACE(*A))
}

equalityExpression(A) ::= relationalExpression(B). {
	U_TRACE(0, "equalityExpression(A) ::= relationalExpression(B)")

	U_INTERNAL_DUMP("A = %p B = %p", A, B)

	U_INTERNAL_ASSERT_POINTER(B)

	A = B;

	U_INTERNAL_DUMP("A = %.*S", U_STRING_TO_TRACE(*A))
}

relationalExpression(A) ::= relationalExpression(B) relationalCond(C) additiveExpression(D). {
   U_TRACE(0, "relationalExpression(A) ::= relationalExpression(B) relationalCond(C) additiveExpression(D)")

   U_INTERNAL_ASSERT_POINTER(B)
   U_INTERNAL_ASSERT_POINTER(D)

	U_INTERNAL_DUMP("B = %.*S C = %d D = %.*S", U_STRING_TO_TRACE(*B), C, U_STRING_TO_TRACE(*D))

   bool bo = false,
		 Bbo = (B->empty() == false),
		 Dbo = (D->empty() == false);

	int cmp = (Bbo && Dbo ? B->compare(D->rep) : Bbo - Dbo);

   switch (C)
      {
      case U_TK_LT: bo = (cmp <  0); break;
      case U_TK_LE: bo = (cmp <= 0); break;
      case U_TK_GT: bo = (cmp >  0); break;
      case U_TK_GE: bo = (cmp >= 0); break;
      }

   U_INTERNAL_DUMP("bo = %b cmp = %d", bo, cmp)

	A = (bo ? U_NEW(UString(U_CONSTANT_TO_PARAM("true"))) : U_NEW(UString));

	delete B;
	delete D;

	U_INTERNAL_DUMP("A = %.*S", U_STRING_TO_TRACE(*A))
}

relationalExpression(A) ::= additiveExpression(B). {
	U_TRACE(0, "relationalExpression(A) ::= additiveExpression(B)")

	U_INTERNAL_DUMP("A = %p B = %p", A, B)

	U_INTERNAL_ASSERT_POINTER(B)

	A = B;

	U_INTERNAL_DUMP("A = %.*S", U_STRING_TO_TRACE(*A))
}

additiveExpression(A) ::= additiveExpression(B) additiveCond(C) multiplicativeExpression(D). {
   U_TRACE(0, "additiveExpression(A) ::= additiveExpression(B) additiveCond(C) multiplicativeExpression(D)")

   U_INTERNAL_ASSERT_POINTER(B)
   U_INTERNAL_ASSERT_POINTER(D)

	U_INTERNAL_DUMP("B = %.*S C = %d D = %.*S", U_STRING_TO_TRACE(*B), C, U_STRING_TO_TRACE(*D))

   long Blo = B->strtol(),
        Dlo = D->strtol(),
		   lo = (C == U_TK_PLUS ? Blo + Dlo
									   : Blo - Dlo);

	A = U_NEW(UString(UStringExt::stringFromNumber(lo)));

	delete B;
	delete D;

	U_INTERNAL_DUMP("A = %.*S", U_STRING_TO_TRACE(*A))
}

additiveExpression(A) ::= multiplicativeExpression(B). {
	U_TRACE(0, "additiveExpression(A) ::= multiplicativeExpression(B)")

	U_INTERNAL_DUMP("A = %p B = %p", A, B)

	U_INTERNAL_ASSERT_POINTER(B)

	A = B;

	U_INTERNAL_DUMP("A = %.*S", U_STRING_TO_TRACE(*A))
}

multiplicativeExpression(A) ::= multiplicativeExpression(B) multiplicativeCond(C) unaryExpression(D). {
   U_TRACE(0, "multiplicativeExpression(A) ::= multiplicativeExpression(B) multiplicativeCond(C) unaryExpression(D)")

   U_INTERNAL_ASSERT_POINTER(B)
   U_INTERNAL_ASSERT_POINTER(D)

	U_INTERNAL_DUMP("B = %.*S C = %d D = %.*S", U_STRING_TO_TRACE(*B), C, U_STRING_TO_TRACE(*D))

   long lo,
		  Blo = B->strtol(),
        Dlo = D->strtol();

	if (Dlo == 0) A = U_NEW(UString(U_STRING_FROM_CONSTANT("0")));
	else
		{
	   lo = (C == U_TK_MULT ? Blo * Dlo :
			   C == U_TK_DIV  ? Blo / Dlo :
									  Blo % Dlo);

		A = U_NEW(UString(UStringExt::stringFromNumber(lo)));
		}

	delete B;
	delete D;

	U_INTERNAL_DUMP("A = %.*S", U_STRING_TO_TRACE(*A))
}

multiplicativeExpression(A) ::= unaryExpression(B). {
	U_TRACE(0, "multiplicativeExpression(A) ::= unaryExpression(B)")

	U_INTERNAL_DUMP("A = %p B = %p", A, B)

	U_INTERNAL_ASSERT_POINTER(B)

	A = B;

	U_INTERNAL_DUMP("A = %.*S", U_STRING_TO_TRACE(*A))
}

unaryExpression(A) ::= NOT primaryExpression(B). {
	U_TRACE(0, "unaryExpression(A) ::= NOT primaryExpression(B)")

	U_INTERNAL_DUMP("A = %p B = %p", A, B)

	U_INTERNAL_ASSERT_POINTER(B)

	A = (B->empty() ? U_NEW(UString(U_CONSTANT_TO_PARAM("true"))) : U_NEW(UString));

	delete B;

	U_INTERNAL_DUMP("A = %.*S", U_STRING_TO_TRACE(*A))
}

unaryExpression(A) ::= primaryExpression(B). {
	U_TRACE(0, "unaryExpression(A) ::= primaryExpression(B)")

	U_INTERNAL_DUMP("A = %p B = %p", A, B)

	U_INTERNAL_ASSERT_POINTER(B)

	A = B;

	U_INTERNAL_DUMP("A = %.*S", U_STRING_TO_TRACE(*A))
}

primaryExpression(A) ::= LPAREN booleanExpression(B) RPAREN. {
	U_TRACE(0, "primaryExpression(A) ::= LPAREN booleanExpression(B) RPAREN")

	U_INTERNAL_DUMP("A = %p B = %p", A, B)

	U_INTERNAL_ASSERT_POINTER(B)

	A = B;

	U_INTERNAL_DUMP("A = %.*S", U_STRING_TO_TRACE(*A))
}

primaryExpression(A) ::= value(B). {
	U_TRACE(0, "primaryExpression(A) ::= value(B)")

	U_INTERNAL_DUMP("A = %p B = %p", A, B)

	U_INTERNAL_ASSERT_POINTER(B)

	A = B;

	U_INTERNAL_DUMP("A = %.*S", U_STRING_TO_TRACE(*A))
}

value(A) ::= VALUE(B). {
	U_TRACE(0, "value(A) ::= VALUE(B)")

	U_INTERNAL_DUMP("A = %p B = %p", A, B)

	U_INTERNAL_ASSERT_POINTER(B)

	A = B;

	U_INTERNAL_DUMP("A = %.*S", U_STRING_TO_TRACE(*A))
}

value(A) ::= value(B) VALUE(C). {
	U_TRACE(0, "value(A) ::= value(B) VALUE(C)")

	U_INTERNAL_ASSERT_POINTER(B)
	U_INTERNAL_ASSERT_POINTER(C)

	U_INTERNAL_DUMP("B = %.*S C = %.*S", U_STRING_TO_TRACE(*B), U_STRING_TO_TRACE(*C))

	 A  =  B;
	*A += *C;

	delete C;

	U_INTERNAL_DUMP("A = %.*S", U_STRING_TO_TRACE(*A))
}

booleanCond(A) ::= OR. {
   U_TRACE(0, "booleanCond(A) ::= OR")
   A = U_TK_OR;
}
booleanCond(A) ::= AND. {
   U_TRACE(0, "booleanCond(A) ::= AND")
   A = U_TK_AND;
}
equalityCond(A) ::= EQ. {
   U_TRACE(0, "equalityCond(A) ::= EQ")
   A = U_TK_EQ;
}
equalityCond(A) ::= NE. {
   U_TRACE(0, "equalityCond(A) ::= NE")
   A = U_TK_NE;
}
equalityCond(A) ::= STARTS_WITH. {
   U_TRACE(0, "equalityCond(A) ::= STARTS_WITH")
   A = U_TK_STARTS_WITH;
}
equalityCond(A) ::= ENDS_WITH. {
   U_TRACE(0, "equalityCond(A) ::= ENDS_WITH")
   A = U_TK_ENDS_WITH;
}
equalityCond(A) ::= IS_PRESENT. {
   U_TRACE(0, "equalityCond(A) ::= IS_PRESENT")
   A = U_TK_IS_PRESENT;
}
equalityCond(A) ::= CONTAINS. {
   U_TRACE(0, "equalityCond(A) ::= CONTAINS")
   A = U_TK_CONTAINS;
}
relationalCond(A) ::= GT. {
   U_TRACE(0, "relationalCond(A) ::= GT")
   A = U_TK_GT;
}
relationalCond(A) ::= GE. {
   U_TRACE(0, "relationalCond(A) ::= GE")
   A = U_TK_GE;
}
relationalCond(A) ::= LT. {
   U_TRACE(0, "relationalCond(A) ::= LT")
   A = U_TK_LT;
}
relationalCond(A) ::= LE. {
   U_TRACE(0, "relationalCond(A) ::= LE")
   A = U_TK_LE;
}
additiveCond(A) ::= PLUS. {
   U_TRACE(0, "additiveCond(A) ::= PLUS")
   A = U_TK_PLUS;
}
additiveCond(A) ::= MINUS. {
   U_TRACE(0, "additiveCond(A) ::= MINUS")
   A = U_TK_MINUS;
}
multiplicativeCond(A) ::= MULT. {
   U_TRACE(0, "multiplicativeCond(A) ::= MULT")
   A = U_TK_MULT;
}
multiplicativeCond(A) ::= DIV. {
   U_TRACE(0, "multiplicativeCond(A) ::= DIV")
   A = U_TK_DIV;
}
multiplicativeCond(A) ::= MOD. {
   U_TRACE(0, "multiplicativeCond(A) ::= MOD")
   A = U_TK_MOD;
}

primaryExpression(A) ::= FN_CALL(B) LPAREN RPAREN. {
	U_TRACE(0, "primaryExpression(A) ::= FN_CALL(B) LPAREN RPAREN")

	U_INTERNAL_ASSERT_POINTER(B)

	U_INTERNAL_DUMP("A = %p B = %.*S", A, U_STRING_TO_TRACE(*B))

   long lo;
#ifdef __MINGW32__
   lPFv addr = (lPFv) ::GetProcAddress((HMODULE)0, B->c_str());
#else
	lPFv addr = (lPFv) U_SYSCALL(dlsym, "%p,%S", RTLD_DEFAULT, B->c_str());
#endif

	A = (addr ? (lo = (*addr)(), U_NEW(UString(UStringExt::stringFromNumber(lo)))) : U_NEW(UString));

	delete B;
}

primaryExpression(A) ::= FN_CALL(B) LPAREN params(C) RPAREN. {
	U_TRACE(0, "primaryExpression(A) ::= FN_CALL(B) LPAREN params(C) RPAREN")

   U_INTERNAL_ASSERT_POINTER(B)
   U_INTERNAL_ASSERT_POINTER(C)

	U_INTERNAL_DUMP("B = %.*S C = %p", U_STRING_TO_TRACE(*B), C)

   long lo, lo0, lo1;
#ifdef __MINGW32__
   lPFll addr = (lPFll) ::GetProcAddress((HMODULE)0, B->c_str());
#else
	lPFll addr = (lPFll) U_SYSCALL(dlsym, "%p,%S", RTLD_DEFAULT, B->c_str());
#endif

	A = (addr ? (lo0 = (*C)[0].strtol(), lo1 = (*C)[1].strtol(), lo = (*addr)(lo0, lo1), U_NEW(UString(UStringExt::stringFromNumber(lo)))) : U_NEW(UString));

	delete B;
}

params(A) ::= VALUE(B). {
	U_TRACE(0, "params(A) ::= VALUE(B)")

	U_INTERNAL_ASSERT_POINTER(B)

	U_INTERNAL_DUMP("A = %p B = %.*S", A, U_STRING_TO_TRACE(*B))

	A = U_NEW(Items);

	A->push_back(*B);

	delete B;
}

params(A) ::= params(B) COMMA VALUE(C). {
	U_TRACE(0, "params(A) ::= params(B) COMMA VALUE(C)")

   U_INTERNAL_ASSERT_POINTER(B)
   U_INTERNAL_ASSERT_POINTER(C)

	U_INTERNAL_DUMP("B = %p C = %.*S", B, U_STRING_TO_TRACE(*C))

	B->push_back(*C);

	A = B;

	delete C;
}
