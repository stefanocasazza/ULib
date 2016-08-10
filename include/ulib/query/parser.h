// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    parser.h - Disjunctive Normal Form boolean expression (BoolStuff - Sarrazin)
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_QUERY_PARSER_H
#define ULIB_QUERY_PARSER_H 1

#include <ulib/tokenizer.h>
#include <ulib/container/vector.h>

class UQueryParser;

class U_EXPORT UQueryNode {
public:
   // Check Memory
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   // Possible types for a boolean expression tree node.
   // VALUE nodes have no children, while NOT nodes only have a right-hand child

   enum Type { VALUE = 0,
               AND   = 1,
               OR    = 2,
               NOT   = 3 };

   /**
    * Creates a node of the given type and with the given children.
    * A NOT node must only have a right-hand child, while AND and OR
    * nodes must have both left-hand and right-hand children.
    *
    * @param t type of the node (must be AND, OR or NOT)
    * @param l subtree to attach as the left-hand child (may be NULL)
    * @param r subtree to attach as the right-hand child (may be NULL)
    */

    UQueryNode(const UString& d);
    UQueryNode(Type t, UQueryNode* l, UQueryNode* r);
   ~UQueryNode();

   /**
    * Gets the roots of the terms of an tree in DNF form. The DNF is a sum of products. Each term in this sum is
    * represented by a subtree of the tree rooted at the current node. This method produces the UQueryNode pointers
    * that represent the roots of the term subtrees. The tree must first be in DNF. See getDisjunctiveNormalForm().
    * For example, if the current node is the root a of DNF tree representing the expression a&b | c | d&e, then
    * three pointers will be stored: one for the 'a&b' subtree, one for the 'c' subtree (a single node) and one for
    * the 'd&e' subtree. If the tree is a single node, then 'this' designates the only term in the sum and it is
    * returned as the root of the unique term. The stored pointers must not be destroyed directly.  
    *
    * @param dest output vector of UQueryNode pointer
    */

   void getDNFTermRoots(UVector<UQueryNode*>* dest);

   /**
    * Returns the variables that are used in the tree root at this node.
    * Example: with the expression a&b&!a&!c, the 'positives' set will contain "a" and "b" and the
    * 'negatives' set will contain "a" and "c". When the intersection between the two sets is not empty
    * and the only binary operator used in the tree is AND, the tree always evaluates to false (because
    * we have an expression of the form (a&!a)&(whatever)). If the only binary operator is OR, the tree
    * always evaluates to true.
    *
    * @param positives set that receives the values of the variables that are used positively
    * @param negatives set that receives the values of the variables that are used negatively
    */

   void getTreeVariables(UVector<UString>* positives, UVector<UString>* negatives) const;

   /**
    * Transforms the designated tree into its Disjunctive Normal Form.
    * The original tree root does not necessarily remain the root of the transformed tree.
    * A simplification is applied: when a term of the form a&!a&(something) is seen, it is deleted
    * unless it is the root of the whole tree. CAUTION: this method can return a NULL pointer; such
    * a result should be interpreted as a "false" boolean expression. Examples are when the original
    * (or resulting) tree is a&!a, or a&!a|b&!b. This method also returns 0 if 'root' is 0.
    *
    * @param   root  root of the tree to transform
    * @returns the root of the transformed tree (may be 0)
    */

   static UQueryNode* getDisjunctiveNormalForm(UQueryNode* root);

   // STREAMS

#ifdef U_STDCPP_ENABLE
   friend ostream& operator<<(ostream& os, const UQueryNode& e);

   // DEBUG

# ifdef DEBUG
   const char* dump(bool reset) const;
# endif
#endif

protected:
   UQueryNode* left;
   UQueryNode* right;
   UString value;
   Type type;

private:
   static void        destroyDNFOrNodes(UQueryNode* root) U_NO_EXPORT;
   static UQueryNode* joinTreesWithOrNodes(UVector<UQueryNode*>& trees) U_NO_EXPORT;

   /**
    * Determines if the tree rooted at this node is in the DNF
    */

   bool isDisjunctiveNormalForm() const U_NO_EXPORT __pure;

   /**
    * Determines if this DNF term always evaluates to false.
    * Must only be called on a term of a DNF tree, which can be obtained with the getDNFTermRoots() method.
    * (e.g., a&b&!a).
    *
    * @returns true if and only if this term always evaluates to false
    */

   bool isDNFTermUseful() const U_NO_EXPORT;

   /**
    * Returns a copy of the designated tree.
    * All nodes in the returned tree are independent copies of those in the original tree.
    * All the cloned nodes are created with operator new.
    * The caller must eventually destroy the cloned tree by calling operator delete on its root node.
    *
    * @param   root the root of the tree to be copied
    * @returns the root of the created tree (0 if root was 0)
    */

   static UQueryNode* cloneTree(const UQueryNode* root) U_NO_EXPORT;

   /**
    * Like getDisjunctiveNormalForm(), but without simplifications
    */

   static UQueryNode* getRawDNF(UQueryNode* root) U_NO_EXPORT;
   static void        swap(UQueryNode*& a, UQueryNode*& b) U_NO_EXPORT;

   U_DISALLOW_ASSIGN(UQueryNode)

   friend class UQueryParser;
};

// override the default...
template <> inline void u_destroy(const UQueryNode** ptr, uint32_t n) { U_TRACE(0,"u_destroy<UQueryNode*>(%p,%u)", ptr, n) }

/**
 * Parser for a language of boolean expressions.
 * The parse() method dynamically allocates a binary tree of nodes that
 * represents the syntactic structure of a textual boolean expression
 */

typedef bool (*bPFpr)(UStringRep*);

class U_EXPORT UQueryParser {
public:

   // Check Memory
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   UQueryParser()
      {
      U_TRACE_REGISTER_OBJECT(0, UQueryParser, "", 0)

      U_INTERNAL_ASSERT_POINTER(UString::str_not);

      tree = 0;
      }

   ~UQueryParser()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UQueryParser)

      clear();
      }

   // SERVICES

   void clear();

   UQueryNode* getTree() const { return tree; }

   /**
    * Parses a textual boolean expression and creates a binary syntax tree.
    * Dynamically allocates a tree of nodes that represents the syntactic structure of 'query'.
    * The returned tree must eventually be destroyed with operator delete 
    *
    * @param query text of the boolean expression to parse
    */

   bool parse(const UString& query);

   /**
    * The Disjunctive Normal Form is an ORing of ANDed terms.
    * In other words, if the OR is considered an additive operation and
    * the AND a multiplicative operation, then the DNF is a sum of products
    */

   bool evaluate() const;

   void startEvaluate(bPFpr function);

   // STREAMS

#ifdef U_STDCPP_ENABLE
   friend ostream& operator<<(ostream& os, const UQueryParser& p) { return os << *(p.tree); }

   // DEBUG

# ifdef DEBUG
   const char* dump(bool reset) const;
# endif
#endif

protected:
   UQueryNode* tree;
   UString word;
   UTokenizer t;

   static bPFpr function;
   static uint32_t partial;
   static UVector<UString>** positives;
   static UVector<UString>** negatives;
   static UVector<UQueryNode*>* termRoots;

private:
   // Implementation methods

   UQueryNode* parseAtom() U_NO_EXPORT;
   UQueryNode* parseTerm() U_NO_EXPORT;
   UQueryNode* parseExpr() U_NO_EXPORT;
   UQueryNode* parseFactor() U_NO_EXPORT;

   bool isBraceStart()
      {
      U_TRACE_NO_PARAM(0, "UQueryParser::isBraceStart()")

      bool result = t.tokenSeen(UString::str_p1);

      U_RETURN(result);
      }

   bool isBraceEnd()
      {
      U_TRACE_NO_PARAM(0, "UQueryParser::isBraceEnd()")

      bool result = t.tokenSeen(UString::str_p2);

      U_RETURN(result);
      }

   bool isOR()
      {
      U_TRACE_NO_PARAM(0, "UQueryParser::isOR()")

      bool result = t.tokenSeen(UString::str_or);

      U_RETURN(result);
      }

   bool isAND()
      {
      U_TRACE_NO_PARAM(0, "UQueryParser::isAND()")

      bool result = t.tokenSeen(UString::str_and);

      U_RETURN(result);
      }

   bool isNOT()
      {
      U_TRACE_NO_PARAM(0, "UQueryParser::isNOT()")

      bool result = t.tokenSeen(UString::str_not);

      U_RETURN(result);
      }

   static inline bool pos_evaluate(void* word) U_NO_EXPORT;
   static inline bool neg_evaluate(void* word) U_NO_EXPORT;

   static void evaluate(UStringRep* word, bool positive) U_NO_EXPORT;

   U_DISALLOW_COPY_AND_ASSIGN(UQueryParser)
};

#endif
