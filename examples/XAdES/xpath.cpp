// ============================================================================
//
// = LIBRARY
//    ulib - c++ library
//
// = FILENAME
//    xpath.cpp - xml Digital SIGnature with libxml2
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include "xpath.h"

#include <libxml/xpathInternals.h>

UNodeSet::UNodeSet(xmlDocPtr _doc, xmlNodeSetPtr _nodes, int _type)
{
   U_TRACE_REGISTER_OBJECT(0, UNodeSet, "%p,%p,%d", _doc, _nodes, _type)

   op          = 0;
   doc         = _doc;
   nodes       = _nodes;
   type        = _type;
   children    = U_NULLPTR;
   next        = prev = this;
   destroyDoc  = false;
}

void UNodeSet::destroy(UNodeSet* nset)
{
   U_TRACE(1, "UNodeSet::destroy(%p)", nset)

   U_INTERNAL_ASSERT_POINTER(nset)

   UNodeSet* tmp;

   while ((tmp = nset))
      {
      if ( nset->next &&
          (nset->next != nset))
         {
         nset->next->prev = nset->prev;
         nset->prev->next = nset->next;

         nset = nset->next;
         }
      else
         {
         nset = U_NULLPTR;
         }

      if (tmp->nodes) U_SYSCALL_VOID(xmlXPathFreeNodeSet, "%p", tmp->nodes);

      if (tmp->children) destroy(tmp->children);

      if (tmp->doc && tmp->destroyDoc) U_SYSCALL_VOID(xmlFreeDoc, "%p", tmp->doc);
      }
}

UNodeSet::~UNodeSet()
{
   U_TRACE_UNREGISTER_OBJECT(0, UNodeSet)

   destroy(this);
}

UXPathData::UXPathData(int data_type, int _nodeSetType, const char* _expr)
{
   U_TRACE_REGISTER_OBJECT(0, UXPathData, "%d,%d,%S", data_type, _nodeSetType, _expr)

   ctx         = U_NULLPTR;
   expr        = _expr;
   type        = data_type;
   nodeSetOp   = UNodeSet::INTERSECTION;
   nodeSetType = _nodeSetType;

   /* create xpath or xpointer context */

   switch (type)
      {
      case XPATH:
      case XPATH2:
         ctx = (xmlXPathContextPtr) U_SYSCALL(xmlXPathNewContext, "%p", U_NULLPTR); /* we'll set doc in the context later */
      break;

      case XPOINTER:
         ctx = (xmlXPathContextPtr) U_SYSCALL(xmlXPtrNewContext, "%p,%p,%p", U_NULLPTR, U_NULLPTR, U_NULLPTR); /* we'll set doc in the context later */
      break;
      }
}

UXPathData::~UXPathData()
{
   U_TRACE_UNREGISTER_OBJECT(0, UXPathData)

   if (ctx) U_SYSCALL_VOID(xmlXPathFreeContext, "%p", ctx);
}

bool UXPathData::registerNamespaces(xmlNodePtr node)
{
   U_TRACE(1, "UXPathData::registerNamespaces(%p)", node)

   U_INTERNAL_ASSERT_POINTER(ctx)

   /* register namespaces */

   for (xmlNodePtr cur = node; cur; cur = cur->parent)
      {
      for (xmlNsPtr ns = cur->nsDef; ns; ns = ns->next)
         {
         /* check that we have no other namespace with same prefix already */

         if (ns->prefix &&
             (U_SYSCALL(xmlXPathNsLookup, "%p,%S", ctx, ns->prefix) == U_NULLPTR))
            {
            if (U_SYSCALL(xmlXPathRegisterNs, "%p,%S,%S", ctx, ns->prefix, ns->href)) U_RETURN(false);
            }
         }
      }

   U_RETURN(true);
}

// DEBUG

#ifdef DEBUG
#  include <ulib/internal/objectIO.h>

const char* UNodeSet::dump(bool reset) const
{
   *UObjectIO::os << "op   " << op           << '\n'
                  << "type " << type         << '\n'
                  << "next " << (void*)next  << '\n'
                  << "prev " << (void*)prev;

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return U_NULLPTR;
}

const char* UXPathData::dump(bool reset) const
{
   *UObjectIO::os << "ctx         " << (void*)ctx         << '\n'
                  << "type        " << type               << '\n'
                  << "expr        " << (expr ? expr : "") << '\n'
                  << "nodeSetOp   " << nodeSetOp          << '\n'
                  << "nodeSetType " << nodeSetType;

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return U_NULLPTR;
}

#endif
