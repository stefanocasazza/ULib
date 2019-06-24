// example_declaration.h

#ifndef U_EXAMPLE_DECLARATION_H
#define U_EXAMPLE_DECLARATION_H 1

static void usp_config_index()
{
   U_TRACE_NO_PARAM(5, "::usp_config_index()")

   U_INTERNAL_ASSERT_POINTER(UServer_Base::pcfg)

   // --------------------------------------------------------------------------------------------------------------------------------------
   // configuration parameters
   // --------------------------------------------------------------------------------------------------------------------------------------
   // INDEX_PARAM ... 
   // --------------------------------------------------------------------------------------------------------------------------------------

   UString x = UServer_Base::pcfg->at(U_CONSTANT_TO_PARAM("INDEX_PARAM"));

   if (x)
      {
      }
}

static void usp_init_index()
{
   U_TRACE_NO_PARAM(5, "::usp_init_index()")
}

static void usp_fork_index()
{
   U_TRACE_NO_PARAM(5, "::usp_fork_index()")
}

static void usp_end_index()
{
   U_TRACE_NO_PARAM(5, "::usp_end_index()")
}

static void usp_auth_index()
{
   U_TRACE_NO_PARAM(5, "::usp_auth_index()")

   if (UHTTP::isPostLogin()) // POST /login => check username and password
      {
      if (UHTTP::loginCookieUser->equal(U_CONSTANT_TO_PARAM("admin"))  == false ||
          UHTTP::loginCookiePasswd->equal(U_CONSTANT_TO_PARAM("1234")) == false)
         {
         UHTTP::loginCookie->clear();
         }
      else
         {
         U_ASSERT(UHTTP::loginCookie->empty())

         (void) UHTTP::loginCookie->replace(U_CONSTANT_TO_PARAM("key-id")); // some key-id

         UString param(200U);

         // -----------------------------------------------------------------------------------------------------------------------------------
         // param: "[ data expire path domain secure HttpOnly ]"
         // -----------------------------------------------------------------------------------------------------------------------------------
         // string -- key_id or data to put in cookie    -- must
         // int    -- lifetime of the cookie in HOURS    -- must (0 -> valid until browser exit)
         // string -- path where the cookie can be used  -- opt
         // string -- domain which can read the cookie   -- opt
         // bool   -- secure mode                        -- opt
         // bool   -- only allow HTTP usage              -- opt
         // -----------------------------------------------------------------------------------------------------------------------------------
         // RET: Set-Cookie: ulib.s<counter>=data&expire&HMAC-MD5(data&expire); expires=expire(GMT); path=path; domain=domain; secure; HttpOnly
         // -----------------------------------------------------------------------------------------------------------------------------------

         param.snprintf(U_CONSTANT_TO_PARAM("[ %v %u / www.jon.org ]"), UHTTP::getKeyIdDataSession(*UHTTP::loginCookie).rep, 24 * 30);

         UHTTP::setCookie(param);
         }
      }
   else
      {
      // if (check_cookie_data() == false)
         {
         UHTTP::loginCookie->clear();
         }
      }
}

// GET

static void GET_logout()
{
   U_TRACE_NO_PARAM(5, "::GET_logout()")
}

static void GET_welcome()
{
   U_TRACE_NO_PARAM(5, "::GET_welcome()")
}

// POST

static void POST_service()
{
   U_TRACE_NO_PARAM(5, "::POST_service()")
}
#endif
