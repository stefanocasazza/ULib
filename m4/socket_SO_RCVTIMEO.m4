dnl socket_SO_RCVTIMEO

dnl Automake macro to check for SO_RCVTIMEO option, which specifies
dnl the maximum number of milliseconds that a blocking read() call will
dnl wait for data to arrive on the socket

AC_DEFUN([AM_SOCKET_SO_RCVTIMEO], [
  AC_CACHE_CHECK(for working socket option SO_RCVTIMEO, ac_cv_socket_SO_RCVTIMEO, [
    AC_RUN_IFELSE([
	 	AC_LANG_PROGRAM([
#ifdef __MINGW32__
#	include <winsock.h>
#else
#	include <netdb.h>
#	include <netinet/in.h>
#	include <sys/socket.h>
#endif
#include <errno.h>
#include <sys/types.h>
], [[
int result = 2;
#ifdef SO_RCVTIMEO
#ifdef __MINGW32__
WSADATA wsaData;
WORD version_requested = MAKEWORD(2, 2); /* version_high, version_low */
int err = WSAStartup(version_requested, &wsaData);
#endif
int n;
char buffer[4096];
struct hostent* host;
struct sockaddr_in addr;
int sockfd = socket(AF_INET, SOCK_STREAM, 0);
#ifdef __MINGW32__
int timer = 1000;
#else
struct timeval timer;
timer.tv_sec  = 1;
timer.tv_usec = 0;
#endif
if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const void*)&timer, sizeof(timer)) == 0) {
host = gethostbyname("www.gnu.org");
addr.sin_family = AF_INET;
addr.sin_port = htons(80);
addr.sin_addr = *((struct in_addr*)host->h_addr);
connect(sockfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));
errno = 0;
n = recv(sockfd, buffer, 4096, 0);
#ifdef __MINGW32__
result = (n == -1 && WSAGetLastError() ==  WSAETIMEDOUT ? 0 : errno);
#else
result = (n == -1 && errno == EAGAIN ? 0 : errno);
#endif
}
#endif
return result;
]])], [ac_cv_socket_SO_RCVTIMEO="yes"], [ac_cv_socket_SO_RCVTIMEO="no"])
])
	if test "$ac_cv_socket_SO_RCVTIMEO" = "yes"; then
      AC_DEFINE_UNQUOTED(HAVE_WORKING_SOCKET_OPTION_SO_RCVTIMEO, 1, [Define if socket option SO_RCVTIMEO is implemented])
	fi
])
