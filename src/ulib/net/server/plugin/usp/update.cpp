// update.cpp - dynamic page translation (update.usp => update.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
#include "world.h"
static char* pquery;
static char query[8192];
static UOrmStatement* pstmt_update;
static void usp_fork_update()
{
 U_TRACE(5, "::usp_fork_update()")
 World::handlerForkSql();
 if (World::psql_query)
  {
  if (UOrmDriver::isAsyncPipelineModeAvaliable()) (void) memcpy(query, U_CONSTANT_TO_PARAM("UPDATE World SET randomNumber = v.randomNumber FROM (VALUES"));
  else
   {
   U_NEW(UOrmStatement, pstmt_update, UOrmStatement(*World::psql_query, U_CONSTANT_TO_PARAM("UPDATE World SET randomNumber = ? WHERE id = ?")));
   pstmt_update->use(World::pworld_query->randomNumber, World::pworld_query->id);
   }
  }
}
static void usp_end_update()
{
 U_TRACE(5, "::usp_end_update()")
 World::handlerEndSql();
 if (pstmt_update)
  {
  U_DELETE(pstmt_update)
  pstmt_update = U_NULLPTR;
  }
}
   
extern "C" {
extern U_EXPORT void runDynamicPage_update(int param);
       U_EXPORT void runDynamicPage_update(int param)
{
   U_TRACE(0, "::runDynamicPage_update(%d)", param)
   
   
   if (param)
      {
      if (param == U_DPAGE_DESTROY) { usp_end_update(); return; }
      if (param == U_DPAGE_FORK) { usp_fork_update(); return; }
      return;
      }
   
   U_ASSERT_EQUALS(UClientImage_Base::wbuffer->findEndHeader(),false)
   U_http_info.endHeader = 34;
   U_http_content_type_len = 1;
   
   (void) UClientImage_Base::wbuffer->insert(0, U_CONSTANT_TO_PARAM("Content-Type: application/json\r\n\r\n"));
   
   UClientImage_Base::setRequestNoCache();
   
   uint32_t i, num_queries = UHTTP::getFormFirstNumericValue(1, 500);
   World::initResult(num_queries);
   if (UOrmDriver::isAsyncPipelineModeAvaliable() == false)
    {
    for (i = 0; i < num_queries; ++i)
     {
     World::pworld_query->id = World::rnumber[i];
     World::pstmt_query->execute();
     World::pworld_query->randomNumber = u_get_num_random(10000-1);
     pstmt_update->execute();
     World::handlerResultSql(i);
     }
    World::endResult();
    return;
    }
   (void) World::pstmt_query->asyncPipelineMode(World::handlerResult);
   pquery = query + U_CONSTANT_SIZE("UPDATE World SET randomNumber = v.randomNumber FROM (VALUES");
   for (i = 0; i < num_queries; ++i)
    {
    *pquery++ = '(';
    pquery = u_num2str32(World::pworld_query->id = World::rnumber[i], pquery);
    (void) World::pstmt_query->asyncPipelineSendQueryPrepared(i);
    *pquery++ = ',';
    pquery = u_num2str32(World::pworld_query->randomNumber = u_get_num_random(10000-1), pquery);
    u_put_unalignedp16(pquery, U_MULTICHAR_CONSTANT16(')',','));
           pquery += 2;
    World::handlerResultSql(i);
    }
   World::endResult();
   (void) memcpy(pquery-1, ") AS v (id,randomNumber) WHERE World.id = v.id;",
       U_CONSTANT_SIZE(") AS v (id,randomNumber) WHERE World.id = v.id;")+1);
   (void) World::pstmt_query->asyncPipelineSendQuery(query, pquery+U_CONSTANT_SIZE(") AS v (id,randomNumber) WHERE World.id = v.id;")-1-query, num_queries);
   
   
} }