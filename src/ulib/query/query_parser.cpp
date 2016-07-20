// =================================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    parser.cpp - Disjunctive Normal Form boolean expression (BoolStuff - Sarrazin)
//
// = AUTHOR
//    Stefano Casazza
//
// =================================================================================

#include <ulib/query/parser.h>

bPFpr                 UQueryParser::function;
uint32_t              UQueryParser::partial;
UVector<UString>**    UQueryParser::positives;
UVector<UString>**    UQueryParser::negatives;
UVector<UQueryNode*>* UQueryParser::termRoots;

UQueryNode::UQueryNode(Type t, UQueryNode* l, UQueryNode* r) : left(l), right(r), type(t)
{
   U_TRACE_REGISTER_OBJECT(0, UQueryNode, "%d,%p,%p", t, l, r)
}

UQueryNode::UQueryNode(const UString& d) : left(0), right(0), value(d), type(VALUE)
{
   U_TRACE_REGISTER_OBJECT(0, UQueryNode, "%V", d.rep)
}

UQueryNode::~UQueryNode()
{
   U_TRACE_UNREGISTER_OBJECT(0, UQueryNode)

   if (left)  delete left;
   if (right) delete right;
}

void UQueryParser::clear()
{
   U_TRACE_NO_PARAM(0, "UQueryParser::clear()")

   if (tree)
      {
      delete tree;
             tree = 0;

      if (termRoots)
         {
         uint32_t i, sz = termRoots->size();

         for (i = 0; i < sz; ++i)
            {
            delete negatives[i];
            delete positives[i];
            }

         UMemoryPool::_free(negatives, sz, sizeof(void*));
         UMemoryPool::_free(positives, sz, sizeof(void*));

         delete termRoots;
                termRoots = 0;
         }
      }

   t.str.clear();
}

/**
 * Create a tree for the expression: a AND NOT (b OR c AND d)
 * 
 * The tree structure is supposed to be this:
 * 
 *   AND
 *  /   \
 * a     NOT
 *         \
 *          OR
 *         /  \
 *        b    AND
 *            /   \
 *           c     d
 */

bool UQueryParser::parse(const UString& query)
{
   U_TRACE(0, "UQueryParser::parse(%V)", query.rep)

   t.setData(query);

   tree = parseExpr();

   if (tree)
      {
      if (t.atEnd() == false) U_ERROR("Syntax error on query - GARBAGE AT END");

      switch (tree->type)
         {
         case UQueryNode::VALUE:
            U_INTERNAL_ASSERT_EQUALS(tree->left,0)
            U_INTERNAL_ASSERT_EQUALS(tree->right,0)
         break;

         case UQueryNode::OR:
         case UQueryNode::AND:
            U_INTERNAL_ASSERT_POINTER(tree->left)
            U_INTERNAL_ASSERT_POINTER(tree->right)
         break;

         case UQueryNode::NOT:
            U_INTERNAL_ASSERT_EQUALS(tree->left,0)
            U_INTERNAL_ASSERT_POINTER(tree->right)
         break;
         }

      tree = UQueryNode::getDisjunctiveNormalForm(tree);

      U_ASSERT(tree->isDisjunctiveNormalForm())

      U_RETURN(true);
      }

   U_RETURN(false);
}

U_NO_EXPORT UQueryNode* UQueryParser::parseExpr()
{
   U_TRACE_NO_PARAM(0, "UQueryParser::parseExpr()")

   UQueryNode* left = parseTerm();

   if (isOR() == false) U_RETURN_POINTER(left, UQueryNode);

   UQueryNode* right = parseExpr();

   UQueryNode* result;

   U_NEW(UQueryNode, result, UQueryNode(UQueryNode::OR, left, right));

   U_RETURN_POINTER(result, UQueryNode);
}

U_NO_EXPORT UQueryNode* UQueryParser::parseTerm()
{
   U_TRACE_NO_PARAM(0, "UQueryParser::parseTerm()")

   UQueryNode* left = parseFactor();

   if (isAND() == false) U_RETURN_POINTER(left,UQueryNode);

   UQueryNode* right = parseTerm();

   UQueryNode* result;

   U_NEW(UQueryNode, result, UQueryNode(UQueryNode::AND, left, right));

   U_RETURN_POINTER(result, UQueryNode);
}

U_NO_EXPORT UQueryNode* UQueryParser::parseFactor()
{
   U_TRACE_NO_PARAM(0, "UQueryParser::parseFactor()")

   bool v = true;

   while (isNOT()) v = !v;

   UQueryNode* atom = parseAtom();

   if (v) U_RETURN_POINTER(atom, UQueryNode);

   UQueryNode* result;
   
   U_NEW(UQueryNode, result, UQueryNode(UQueryNode::NOT, 0, atom));

   U_RETURN_POINTER(result, UQueryNode);
}

U_NO_EXPORT UQueryNode* UQueryParser::parseAtom()
{
   U_TRACE_NO_PARAM(0, "UQueryParser::parseAtom()")

   if (isBraceStart())
      {
      UQueryNode* expr = parseExpr();

      if (isBraceEnd() == false)
         {
         U_ERROR("Syntax error on query - RUNAWAY PARENTHESIS");
         }

      U_RETURN_POINTER(expr, UQueryNode);
      }

   UQueryNode* node;

   U_NEW(UQueryNode, node, UQueryNode(t.getTokenQueryParser()));

   U_RETURN_POINTER(node, UQueryNode);
}

/**
 * Determines if the tree rooted at this node is in the DNF
 */

__pure bool UQueryNode::isDisjunctiveNormalForm() const
{
   U_TRACE_NO_PARAM(0, "UQueryNode::isDisjunctiveNormalForm()")

   U_CHECK_MEMORY

   if (type == VALUE) U_RETURN(left == 0 && right == 0);

   if (type == NOT)
      {
      if (left       ||
          right == 0 ||
          !right->isDisjunctiveNormalForm())
         {
         U_RETURN(false);
         }

      if (right->type == VALUE ||
          right->type == NOT)
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

   if (type == AND)
      {
      if (left == 0 || right == 0         ||
         !left->isDisjunctiveNormalForm() ||
         !right->isDisjunctiveNormalForm())
         {
         U_RETURN(false);
         }

      if (left->type  != OR &&
          right->type != OR)
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

   if (type == OR)
      {
      if (left  == 0                       ||
          right == 0                       ||
          !left->isDisjunctiveNormalForm() ||
          !right->isDisjunctiveNormalForm())
         {
         U_RETURN(false);
         }

      U_RETURN(true);
      }

   U_RETURN(false);
}

/**
 * Returns a copy of the designated tree. All nodes in the returned tree are independent copies of those in
 * the original tree. All the cloned nodes are created with operator new. The caller must eventually destroy
 * the cloned tree by calling operator delete on its root node
 */

U_NO_EXPORT UQueryNode* UQueryNode::cloneTree(const UQueryNode* root)
{
   U_TRACE(0, "UQueryNode::cloneTree(%p)", root)

   if (root == 0) U_RETURN_POINTER(0, UQueryNode);

   UQueryNode* cloneRoot;
   UQueryNode* leftClone  = cloneTree(root->left);
   UQueryNode* rightClone = cloneTree(root->right);
   
   U_NEW(UQueryNode, cloneRoot, UQueryNode(root->value));

   cloneRoot->type  = root->type;
   cloneRoot->left  = leftClone;
   cloneRoot->right = rightClone;

   U_RETURN_POINTER(cloneRoot, UQueryNode);
}

/* Like getDisjunctiveNormalForm(), but without simplifications */

U_NO_EXPORT void UQueryNode::swap(UQueryNode*& a, UQueryNode*& b)
{
   U_TRACE(0, "UQueryNode::swap(%p,%p)", a, b)

   UQueryNode* tmp = a;

   a = b;
   b = tmp;
}

U_NO_EXPORT UQueryNode* UQueryNode::getRawDNF(UQueryNode* root)
{
   U_TRACE(0, "UQueryNode::getRawDNF(%p)", root)

   if (root == 0) U_RETURN_POINTER(0, UQueryNode);

   // One-level trees

   if (root->type == VALUE) U_RETURN_POINTER(root, UQueryNode);

   // Two levels trees

   U_INTERNAL_ASSERT_POINTER(root->right)

   switch (root->type)
      {
      case NOT:
         {
         U_INTERNAL_ASSERT_EQUALS(root->left,0)

         if (root->right->type == VALUE) U_RETURN_POINTER(root, UQueryNode);
         }
      break;

      case OR:
      case AND:
         {
         U_INTERNAL_ASSERT_POINTER(root->left)

         if (root->left->type  == VALUE &&
             root->right->type == VALUE)
            {
            U_RETURN_POINTER(root, UQueryNode);
            }
         }
      break;

      case VALUE:
         U_INTERNAL_ASSERT(false)
      break;
      }

   // Three or more levels

   root->left  = getRawDNF(root->left);
   root->right = getRawDNF(root->right);

   /**
    * Here, because of some simplifications, we may now have a two-level tree. For example, if the original tree was
    * (!(!a)) & (!(!m)). The double negations have been simplified and we now have a&m
    */

   U_INTERNAL_ASSERT_POINTER(root->right)

   if (root->type == NOT)
      {
      U_INTERNAL_ASSERT_EQUALS(root->left, 0)

      if (root->right->type == NOT)
         {
         // Two NOTs make a positive

         U_INTERNAL_ASSERT_POINTER(root->right)
         U_INTERNAL_ASSERT_EQUALS(root->left, 0)

         UQueryNode* newRoot = root->right->right;
                               root->right->right = 0;

         delete root; // deletes two nodes

         UQueryNode* result = getRawDNF(newRoot);

         U_RETURN_POINTER(result, UQueryNode);
         }

      if (root->right->type == OR)
         {
         UQueryNode* a = root->right->left;
         UQueryNode* b = root->right->right;

         // We have !(a | b), which becomes !a && !b:

         root->right->left  =
         root->right->right = 0;

         delete root->right;  // destroy the OR node

         UQueryNode* notA = root;
                     notA->right = a;

         UQueryNode* notB;
         UQueryNode* newRoot;

         U_NEW(UQueryNode, notB,    UQueryNode(NOT, 0, b));
         U_NEW(UQueryNode, newRoot, UQueryNode(AND, notA, notB));

         UQueryNode* result = getRawDNF(newRoot);

         U_RETURN_POINTER(result, UQueryNode);
         }

      if (root->right->type == AND)
         {
         UQueryNode* a = root->right->left;
         UQueryNode* b = root->right->right;

         // We have !(a & b), which becomes !a | !b:

         root->right->left  =
         root->right->right = 0;

         delete root->right; // destroy the AND node

         UQueryNode* notA = root;
                     notA->right = a;

         UQueryNode* notB;
         UQueryNode* newRoot;

         U_NEW(UQueryNode, notB,    UQueryNode(NOT, 0, b));
         U_NEW(UQueryNode, newRoot, UQueryNode(OR, notA, notB));

         UQueryNode* result = getRawDNF(newRoot);

         U_RETURN_POINTER(result, UQueryNode);
         }

      U_INTERNAL_ASSERT(false)
      }

   U_INTERNAL_ASSERT_POINTER(root->left)
   U_INTERNAL_ASSERT_DIFFERS(root->type,NOT)

   // If one side is a value, make sure that this value is at the left

   if (root->left->type  != VALUE &&
       root->right->type == VALUE)
      {
      swap(root->left, root->right);
      }

   // Permutate the left and right subtrees if they are not in our "conventional order"

   if (root->left->type  == NOT &&
       root->right->type == OR)
      {
      swap(root->left, root->right);
      }
   else if (root->left->type  == NOT &&
            root->right->type == AND)
      {
      swap(root->left, root->right);
      }
   else if (root->left->type  == OR &&
            root->right->type == AND)
      {
      swap(root->left, root->right);
      }

   /* Conventional order: root->left->type and root->right->type are expected to be equal or to be one of (OR, NOT), (AND, NOT), (AND, OR) */

   if (root->type == OR)
      {
      if (root->left->type  == VALUE &&
          root->right->type == NOT)
         {
         // Expected because of recursion step

         U_INTERNAL_ASSERT_EQUALS(root->right->right->type, VALUE)

         U_RETURN_POINTER(root, UQueryNode);
         }

      if (root->left->type == VALUE)
         {
         U_RETURN_POINTER(root, UQueryNode);
         }

      if (root->left->type  != NOT &&
          root->right->type != NOT)
         {
         U_RETURN_POINTER(root, UQueryNode);
         }

      if (root->left->type == NOT)
         {
         U_INTERNAL_ASSERT_EQUALS(root->left->left, 0)
         U_INTERNAL_ASSERT_EQUALS(root->right->left, 0)
         U_INTERNAL_ASSERT_EQUALS(root->right->type, NOT) // expected re: conv. order

         // Expected because of recursion step

         U_INTERNAL_ASSERT_EQUALS(root->left->right->type,  VALUE)
         U_INTERNAL_ASSERT_EQUALS(root->right->right->type, VALUE)

         U_RETURN_POINTER(root, UQueryNode);
         }

      if (root->right->type != NOT) U_RETURN_POINTER(root,UQueryNode);

      // Expected because of recursion step

      U_INTERNAL_ASSERT_EQUALS(root->right->right->type, VALUE)

      U_RETURN_POINTER(root, UQueryNode);
      }

   if (root->type == AND)
      {
      if (root->left->type  == VALUE &&
          root->right->type == NOT)
         {
         // Expected because of recursion step

         U_INTERNAL_ASSERT_EQUALS(root->right->right->type, VALUE)

         U_RETURN_POINTER(root, UQueryNode);
         }

      if (root->left->type  == VALUE &&
          root->right->type == AND)
         {
         U_RETURN_POINTER(root, UQueryNode);
         }

      if (root->left->type  == VALUE &&
          root->right->type == OR)
         {
         UQueryNode* andNode = root;
         UQueryNode* x       = root->left;
         UQueryNode* orNode  = root->right;
         UQueryNode* a       = root->right->left;
         UQueryNode* b       = root->right->right;

         andNode->right = a;

         UQueryNode* xClone;
         UQueryNode* newAndNode;

         U_NEW(UQueryNode, xClone,     UQueryNode(x->value));
         U_NEW(UQueryNode, newAndNode, UQueryNode(AND, xClone, b));

         orNode->left  = andNode;
         orNode->right = newAndNode;

         UQueryNode* result = getRawDNF(orNode);

         U_RETURN_POINTER(result, UQueryNode);
         }

      if (root->left->type  == AND &&
          root->right->type == AND)
         {
         U_RETURN_POINTER(root,UQueryNode);
         }

      if (root->left->type  == VALUE &&
          root->right->type == VALUE)
         {
         U_RETURN_POINTER(root,UQueryNode);
         }

      if (root->left->type == NOT)
         {
         U_INTERNAL_ASSERT_EQUALS(root->left->left, 0)
         U_INTERNAL_ASSERT_EQUALS(root->right->left, 0)
         U_INTERNAL_ASSERT_EQUALS(root->right->type, NOT) // expected re: conv. order

         // Expected because of recursion step

         U_INTERNAL_ASSERT_EQUALS(root->left->right->type,  VALUE)
         U_INTERNAL_ASSERT_EQUALS(root->right->right->type, VALUE)

         U_RETURN_POINTER(root, UQueryNode);
         }

      if (root->right->type == NOT)
         {
         U_INTERNAL_ASSERT_POINTER(root->right->right)
         U_INTERNAL_ASSERT_EQUALS(root->right->left, 0)
         U_INTERNAL_ASSERT_EQUALS(root->right->right->type, VALUE)

         if (root->left->type == AND) U_RETURN_POINTER(root,UQueryNode);

         UQueryNode* a       = root->left->left;
         UQueryNode* b       = root->left->right;
         UQueryNode* c       = root->right->right;
         UQueryNode* andNode = root;
         UQueryNode* orNode  = root->left;
         UQueryNode* notNode = root->right;

         // We have (a|b) & !c, which becomes (a&!c) | (b&!c):

         UQueryNode* newCNode;
         UQueryNode* newNotNode;
         UQueryNode* newAndNode;

         U_NEW(UQueryNode, newCNode,   UQueryNode(c->value));
         U_NEW(UQueryNode, newNotNode, UQueryNode(NOT, 0, newCNode));
         U_NEW(UQueryNode, newAndNode, UQueryNode(AND, b, newNotNode));

         orNode->left   = andNode;
         orNode->right  = newAndNode;
         andNode->left  = a;
         andNode->right = notNode;
         notNode->right = c;

         UQueryNode* result = getRawDNF(orNode);

         U_RETURN_POINTER(result, UQueryNode);
         }

      U_INTERNAL_ASSERT_EQUALS(root->right->type, OR)

      if (root->left->type == OR)
         {
         UQueryNode* a           = root->left->left;
         UQueryNode* b           = root->left->right;
         UQueryNode* c           = root->right->left;
         UQueryNode* d           = root->right->right;
         UQueryNode* andNode     = root;
         UQueryNode* leftOrNode  = root->left;
         UQueryNode* rightOrNode = root->right;

         // We have (a|b) & (c|d), which becomes a&b | a&c | b&c | b&d:

         andNode->left  = a;
         andNode->right = c;

         UQueryNode* newRoot;
         UQueryNode* firstNewAndNode;
         UQueryNode* secondNewAndNode;
         UQueryNode* thirdNewAndNode;

         UQueryNode* aClone = cloneTree(a);

         U_NEW(UQueryNode, firstNewAndNode, UQueryNode(AND, aClone, d));

         UQueryNode* cClone = cloneTree(c);

         U_NEW(UQueryNode, secondNewAndNode, UQueryNode(AND, b, cClone));

         UQueryNode* bClone = cloneTree(b);
         UQueryNode* dClone = cloneTree(d);

         U_NEW(UQueryNode, thirdNewAndNode, UQueryNode(AND, bClone, dClone));

         leftOrNode->left  = andNode;
         leftOrNode->right = firstNewAndNode;

         rightOrNode->left  = secondNewAndNode;
         rightOrNode->right = thirdNewAndNode;

         U_NEW(UQueryNode, newRoot, UQueryNode(OR, leftOrNode, rightOrNode));

         UQueryNode* result = getRawDNF(newRoot);

         U_RETURN_POINTER(result, UQueryNode);
         }

      if (root->left->type == AND)
         {
         UQueryNode* a           = root->left->left;
         UQueryNode* b           = root->left->right;
         UQueryNode* c           = root->right->left;
         UQueryNode* d           = root->right->right;
         UQueryNode* topAndNode  = root;
         UQueryNode* rightOrNode = root->right;

         UQueryNode* aClone = cloneTree(a);
         UQueryNode* bClone = cloneTree(b);

         UQueryNode* newLowAndNode;
         UQueryNode* newHighAndNode;

         U_NEW(UQueryNode, newLowAndNode,  UQueryNode(AND, aClone, bClone));
         U_NEW(UQueryNode, newHighAndNode, UQueryNode(AND, newLowAndNode, d));

         topAndNode->right = c;

         rightOrNode->left  = topAndNode;
         rightOrNode->right = newHighAndNode;

         UQueryNode* result = getRawDNF(rightOrNode);

         U_RETURN_POINTER(result,UQueryNode);
         }

      U_INTERNAL_ASSERT(false)
      }

   U_RETURN_POINTER(0, UQueryNode);
}

/**
 * Returns the variables that are used in the tree root at this node.
 * Example: with the expression a&b&!a&!c, the 'positives' set will contain "a" and "b" and the
 * 'negatives' set will contain "a" and "c". When the intersection between the two sets is not empty
 * and the only binary operator used in the tree is AND, the tree always evaluates to false (because
 * we have an expression of the form (a&!a)&(whatever)). If the only binary operator is OR, the tree
 * always evaluates to true
 */

void UQueryNode::getTreeVariables(UVector<UString>* positives, UVector<UString>* negatives) const
{
   U_TRACE(0, "UQueryNode::getTreeVariables(%p,%p)", positives, negatives)

   U_CHECK_MEMORY

   if (type == VALUE)
      {
      positives->insertAsSet(value);

      return;
      }

   if (type == NOT)
      {
      U_INTERNAL_ASSERT_POINTER(right)

      negatives->insertAsSet(right->value);

      return;
      }

   U_INTERNAL_ASSERT_POINTER(left)
   U_INTERNAL_ASSERT_POINTER(right)
   U_INTERNAL_ASSERT(type == OR || type == AND)

    left->getTreeVariables(positives, negatives);
   right->getTreeVariables(positives, negatives);
}

U_NO_EXPORT bool UQueryNode::isDNFTermUseful() const
{
   U_TRACE_NO_PARAM(0, "UQueryNode::isDNFTermUseful()")

   U_CHECK_MEMORY

   UVector<UString> positives, negatives, intersection;

   getTreeVariables(&positives, &negatives);

   uint32_t sz = intersection.intersection(positives, negatives);

   if (sz == 0) U_RETURN(true);

   U_RETURN(false);
}

U_NO_EXPORT void UQueryNode::destroyDNFOrNodes(UQueryNode* root)
{
   U_TRACE(0, "UQueryNode::destroyDNFOrNodes(%p)", root)

   if (root == 0 ||
       root->type != OR)
      {
      return;
      }

   // Detach the OR node's subtrees, so that 'delete' does not affect them:

   UQueryNode* _left  = root->left;
   UQueryNode* _right = root->right;

   U_INTERNAL_ASSERT_POINTER(_left)
   U_INTERNAL_ASSERT_POINTER(_right)

   root->left = root->right = 0;

   delete root;

   destroyDNFOrNodes(_left);
   destroyDNFOrNodes(_right);
}

// override the default...
// template <> inline void u_destroy(const UQueryNode** ptr, uint32_t n) { U_TRACE(0,"u_destroy<UQueryNode*>(%p,%u)", ptr, n) }

U_NO_EXPORT UQueryNode* UQueryNode::joinTreesWithOrNodes(UVector<UQueryNode*>& trees)
{
   U_TRACE(0, "UQueryNode::joinTreesWithOrNodes(%p)", &trees)

   uint32_t sz = trees.size();

   if (sz == 0) U_RETURN_POINTER(0,        UQueryNode);
   if (sz == 1) U_RETURN_POINTER(trees[0], UQueryNode);

   uint32_t i = 0;

   UQueryNode* result;

   U_NEW(UQueryNode, result, UQueryNode(OR, trees[i], 0));

   result->right = trees[++i];

   for (++i; i < sz; ++i) U_NEW(UQueryNode, result, UQueryNode(OR, result, trees[i]));

   U_RETURN_POINTER(result, UQueryNode);
}

/**
 * Gets the roots of the terms of an tree in DNF form. The DNF is a sum of products. Each term in this sum is
 * represented by a subtree of the tree rooted at the current node. This method produces the UQueryNode pointers
 * that represent the roots of the term subtrees. The tree must first be in DNF. See getDisjunctiveNormalForm().
 * For example, if the current node is the root a of DNF tree representing the expression a&b | c | d&e, then
 * three pointers will be stored: one for the 'a&b' subtree, one for the 'c' subtree (a single node) and one for
 * the 'd&e' subtree. If the tree is a single node, then 'this' designates the only term in the sum and it is
 * returned as the root of the unique term. The stored pointers must not be destroyed directly
 */

void UQueryNode::getDNFTermRoots(UVector<UQueryNode*>* dest)
{
   U_TRACE(0, "UQueryNode::getDNFTermRoots(%p)", dest)

   U_CHECK_MEMORY

   switch (type)
      {
      case OR:
         {
         U_INTERNAL_ASSERT_POINTER(left)
         U_INTERNAL_ASSERT_POINTER(right)

          left->getDNFTermRoots(dest);
         right->getDNFTermRoots(dest);
         }
      break;

      case AND:
      case NOT:
      case VALUE:
         dest->push(this);
      break;
      }
}

UQueryNode* UQueryNode::getDisjunctiveNormalForm(UQueryNode* root)
{
   U_TRACE(0, "UQueryNode::getDisjunctiveNormalForm(%p)", root)

   UQueryNode* dnfRoot = getRawDNF(root);

   if (dnfRoot == 0) U_RETURN_POINTER(dnfRoot,UQueryNode);

   UVector<UQueryNode*> termRoots, usefulTerms;

   dnfRoot->getDNFTermRoots(&termRoots);

   // Determine which terms are useful

   uint32_t i, sz = termRoots.size();

   for (i = 0; i < sz; ++i)
      {
      UQueryNode* dn = termRoots.at(i);

      U_INTERNAL_ASSERT_POINTER(dn)

      if (dn->isDNFTermUseful()) usefulTerms.push_back(dn);
      }

   // Nothing to do if all the terms are useful

   if (usefulTerms.size() == sz) U_RETURN_POINTER(dnfRoot, UQueryNode);

   destroyDNFOrNodes(dnfRoot);

   // dnfRoot now invalid

   UQueryNode* result = joinTreesWithOrNodes(usefulTerms);

   U_RETURN_POINTER(result, UQueryNode);
}

/**
 * The Disjunctive Normal Form is an ORing of ANDed terms.
 * In other words, if the OR is considered an additive operation and
 * the AND a multiplicative operation, then the DNF is a sum of products
 */

void UQueryParser::startEvaluate(bPFpr func)
{
   U_TRACE(0, "UQueryParser::startEvaluate(%p)", func)

   U_INTERNAL_ASSERT_POINTER(tree)
   U_INTERNAL_ASSERT_POINTER(func)
   U_INTERNAL_ASSERT_EQUALS(termRoots,0)

   function = func;

   U_NEW(UVector<UQueryNode*>, termRoots, UVector<UQueryNode*>);

   tree->getDNFTermRoots(termRoots);

   uint32_t sz = termRoots->size();

   negatives = (UVector<UString>**) UMemoryPool::_malloc(sz, sizeof(void*));
   positives = (UVector<UString>**) UMemoryPool::_malloc(sz, sizeof(void*));

   for (uint32_t i = 0; i < sz; ++i)
      {
      U_NEW(UVector<UString>, negatives[i], UVector<UString>);
      U_NEW(UVector<UString>, positives[i], UVector<UString>);

      UQueryNode* term = termRoots->at(i);

      U_INTERNAL_ASSERT_POINTER(term)

      term->getTreeVariables(positives[i], negatives[i]);
      }
}

void UQueryParser::evaluate(UStringRep* _word, bool positive)
{
   U_TRACE(0, "UQueryParser::evaluate(%V,%b)", _word, positive)

   U_INTERNAL_ASSERT_EQUALS(partial, 1)

   U_INTERNAL_DUMP("u_buffer_len = %u", u_buffer_len)

   /**
    * EXAMPLE
    * --------------------------------------------------------------------------------------
    * Original expression     : (a OR b) AND NOT (c AND d)
    * Disjunctive normal form : a AND  NOT c OR a AND  NOT d OR b AND  NOT c OR b AND  NOT d
    * --------------------------------------------------------------------------------------
    *  Term       : a AND  NOT c
    *  Positives: ( a )
    *  Negatives: ( c )
    * --------------------------------------------------------------------------------------
    *  Term       : a AND  NOT d
    *  Positives: ( a )
    *  Negatives: ( d )
    * --------------------------------------------------------------------------------------
    *  Term       : b AND  NOT c
    *  Positives: ( b )
    *  Negatives: ( c )
    * --------------------------------------------------------------------------------------
    *  Term       : b AND  NOT d
    *  Positives: ( b )
    *  Negatives: ( d )
    * --------------------------------------------------------------------------------------
    */

   char* p = (u_buffer_len ? (char*) u_find(u_buffer, u_buffer_len, (const char*)&_word, sizeof(void*)) : 0);

   bool result = (p ? (p[sizeof(void*)] == '1') : function(_word));

   if (!p)
      {
      union _uustringrep {
         UStringRep* p2;
         long*       p3;
      };

      _uustringrep pword;

      p = u_buffer + u_buffer_len;

      pword.p2 = _word;

#  if SIZEOF_LONG == 8
      u_put_unalignedp64(p, *pword.p3);
#  else
      u_put_unalignedp32(p, *pword.p3);
#  endif

      u_buffer_len += sizeof(long);

      u_buffer[u_buffer_len++] = (result ? '1' : '0');

      U_INTERNAL_ASSERT_MINOR(u_buffer_len, U_BUFFER_SIZE)
      }

   if (( positive && !result) ||
       (!positive &&  result))
      {
      partial = 0;
      }

   U_INTERNAL_DUMP("partial = %u result = %b", partial, result)
}

inline bool UQueryParser::pos_evaluate(void* _word)
{
   U_TRACE(0, "UQueryParser::pos_evaluate(%p)", _word)

   evaluate((UStringRep*)_word, true);

   if (partial == 1) U_RETURN(true);

   U_RETURN(false);
}

inline bool UQueryParser::neg_evaluate(void* _word)
{
   U_TRACE(0, "UQueryParser::neg_evaluate(%p)", _word)

   if (partial) evaluate((UStringRep*)_word, false);

   if (partial == 1) U_RETURN(true);

   U_RETURN(false);
}

bool UQueryParser::evaluate() const
{
   U_TRACE_NO_PARAM(0, "UQueryParser::evaluate()")

   U_INTERNAL_ASSERT_POINTER(termRoots)

   U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

   for (uint32_t i = 0, sz = termRoots->size(); i < sz; ++i)
      {
      partial = 1;

      if (positives[i]->empty() == false) positives[i]->callForAllEntry(pos_evaluate);
      if (negatives[i]->empty() == false) negatives[i]->callForAllEntry(neg_evaluate);

      U_INTERNAL_DUMP("partial = %u", partial)

      if (partial)
         {
         u_buffer_len = 0;

         U_RETURN(true);
         }
      }

   u_buffer_len = 0;

   U_RETURN(false);
}

// STREAMS

#ifdef U_STDCPP_ENABLE
U_EXPORT ostream& operator<<(ostream& os, const UQueryNode& e)
{
   switch (e.type)
      {
      case UQueryNode::VALUE:
         os << e.value;
      break;

      case UQueryNode::NOT:
         {
         bool parent = (e.right->type == UQueryNode::AND ||
                        e.right->type == UQueryNode::OR);

         os << ' ' << *UString::str_not << ' ';

         if (parent) os << *UString::str_p1;

         os << *(e.right);

         if (parent) os << *UString::str_p2;
         }
      break;

      case UQueryNode::OR:
         {
         os << *(e.left);

         os << ' ' << *UString::str_or << ' ';

         os << *(e.right);
         }
      break;

      case UQueryNode::AND:
         {
         bool parent = (e.left->type == UQueryNode::OR);

         if (parent) os << *UString::str_p1;

         os << *(e.left);

         if (parent) os << *UString::str_p2;

         os << ' ' << *UString::str_and << ' ';

         parent = (e.right->type == UQueryNode::OR);

         if (parent) os << *UString::str_p1;

         os << *(e.right);

         if (parent) os << *UString::str_p2;
         }
      break;
      }

   return os;
}

// DEBUG

#  ifdef DEBUG
const char* UQueryNode::dump(bool reset) const
{
   *UObjectIO::os << "type           " << type          << '\n'
                  << "left           " << (void*)left   << '\n'
                  << "right          " << (void*)right  << '\n'
                  << "value (UString " << (void*)&value << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}

const char* UQueryParser::dump(bool reset) const
{
   *UObjectIO::os << "tree              " << (void*)tree    << '\n'
                  << "word  (UString    " << (void*)&word   << ")\n"
                  << "t     (UTokenizer " << (void*)&t      << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#  endif
#endif
