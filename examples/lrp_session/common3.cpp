// common3.cpp

   // write request to file

   name.snprintf("%s/%s_%s_%#4D.req", directory.c_str(), filtro, policy, tm_session);

   (void) UFile::writeTo(name, request);
