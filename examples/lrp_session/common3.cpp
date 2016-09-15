// common3.cpp

   // write request to file

   name.snprintf(U_CONSTANT_TO_PARAM("%v/%s_%s_%#4D.req"), directory.rep, filtro, policy, tm_session);

   (void) UFile::writeTo(name, request);
