// test_command.cpp

#include <ulib/command.h>

#define ENVIRONMENT "HOME=LCSP/DB_CA \
                     FILE_LOG=log \
                     \"RSIGN_CMD=./rsaprivenc.sh -c rsa_priv_enc.cfg -k INKEY < DIGEST\" \
                     \"MSG_LOG=***** LCSP-CA *****\""

int
U_EXPORT main (int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   UCommand cmd;
   UString result1, result2, result3;
   UString env = U_STRING_FROM_CONSTANT(ENVIRONMENT);
   int fd_stderr = U_SYSCALL(open, "%S,%d", "err/command.err", O_WRONLY);

   cmd.set(U_STRING_FROM_CONSTANT("ls test_command.cpp"));
   cmd.setEnvironment(&env);

   bool result = cmd.execute(0, &result1, -1, fd_stderr);

   U_ASSERT( result == true )
   U_ASSERT( result1.empty() == false )

   cmd.set(U_STRING_FROM_CONSTANT("cat test_command.cpp"));

   result = cmd.execute(0, &result2, -1, fd_stderr);

   U_ASSERT( result == true )
   U_ASSERT( result2.empty() == false )

   cout.write(result2.data(), result2.size());

   cmd.set(U_STRING_FROM_CONSTANT("/usr/bin/catdog test_command.cpp"));

   result = cmd.execute(0, &result3, -1, fd_stderr);

   U_ASSERT( result == false )
   U_ASSERT( result3.empty() )

   UCommand::setMsgError(cmd.getCommand(), false);
   UCommand::printMsgError();
}
