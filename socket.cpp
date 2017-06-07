#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#ifdef WIN32
	#include <winsock2.h>
#else
	#include <sys/types.h>
        #include <sys/socket.h>
	#include <sys/time.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <fcntl.h>
	#include <unistd.h>
	#include <netinet/tcp.h>
	#include <sys/poll.h>

	#define INVALID_SOCKET	-1
	#define SOCKET_ERROR	-1
#endif

#ifndef BSD
	#include "tools.hpp"
#endif

//#ifdef _SOCKET_LOG
#include "log.hpp"
//#endif


#ifdef _THREAD_SAFE
#include "mutexsync.hpp"
lb::MutexSyncData mutex;
#endif

#include "socket.hpp"

using namespace lb;

SocketClient::SocketClient()
{
	isOk	= 1;

	descr = 0;
#ifdef WIN32
	WORD		wVersionRequested = MAKEWORD(2,0);
	WSADATA		wsaData;

	if (WSAStartup(wVersionRequested, &wsaData))
	{
#ifdef _SOCKET_LOG
		L(LOG_ERROR, "No useable WINSOCK.DLL\n");
#endif
		isOk = 0;
	}
#endif

	if ((descr = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
#ifdef _SOCKET_LOG
		L(LOG_CRITICAL, "Socket Error (%d)\n", errno);
#endif
		isOk = 0;
	}

}

SocketClient::SocketClient(Descriptor descr, sockaddr_in &sock_addr)
{

  this->descr = descr;
	SetIp(sock_addr);
	isOk = 1;
}

SocketClient::~SocketClient()
{
	if (descr != INVALID_SOCKET)
	{
		shutdown(descr, 2);
		close(descr);
	}
}

void SocketClient::Close()
{
	shutdown(descr, 2);
	close(descr);
	descr = INVALID_SOCKET;
}

void SocketClient::Shutdown()
{
	isOk = 1;
	shutdown(descr, 2);
	close(descr);
	if ((descr = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("Socket Error %u\n", errno);
		isOk = 0;
	}
}

void SocketClient::Open()
{
  if ((descr = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
  {
    printf("Socket Error %u\n", errno);
    isOk = 0;
  }
	isOk = 1;
}

/*int main()
{
	char buf[1000] = "User Den Draal draal :Denis Misko\nNick den\nJoin #love";
	SocketClient client;
	client.Connect("draal.com.ua", 6669);
	client.Send(buf, strlen(buf), 0);
	client.Read(buf, 1000);
}*/

unsigned int SocketClient::MthrGetIpStr(const char *host)
{
	struct addrinfo hints, *res, *res0;
  int error;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  error = getaddrinfo(host, NULL, &hints, &res0);
  if (error) {
#ifdef _SOCKET_LOG
          L(LOG_ERROR, "Host not resolv: %s", gai_strerror(error));
#endif
          return 0;
  }
  //s = -1;
  //cause = "no addresses";
  //errno = EADDRNOTAVAIL;
  unsigned int ip = 0;
  for (res = res0; res; res = res->ai_next)
  {
  	if (res->ai_addr)
  	{
  		sockaddr_in *addr = (sockaddr_in*) res->ai_addr;
  		ip = ntohl(addr->sin_addr.s_addr);
			if (ip)
				break;
		}
  }
  freeaddrinfo(res0);
  return ip;
}

unsigned int SocketClient::GetIPStr(const char *host)
{
	sockaddr_in addr;
	memset(&addr, 0, sizeof(sockaddr_in));
	addr.sin_family = AF_INET;

#ifdef _THREAD_SAFE
	mutex.LockData();
#endif
	struct hostent *hp = gethostbyname(host);
	if (hp==0)
	{
		printf("Get Host Error %s\n", host);
#ifdef _THREAD_SAFE
		mutex.UnLockData();
#endif
		return 0;
	}
	memcpy(&addr.sin_addr, hp->h_addr, hp->h_length);
#ifdef _THREAD_SAFE
	mutex.UnLockData();
#endif
	return ntohl(addr.sin_addr.s_addr);
}

int SocketClient::Connect(unsigned int ip, int port, int nsec)
{
	int n;
	sockaddr_in addr;
	memset(&addr, 0, sizeof(sockaddr_in));
	addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = ntohl(ip);
	addr.sin_port = htons(port);
	SetNonBlock();

	if  ( (n = ::connect(descr, (sockaddr *)&addr, sizeof(addr))) != 0)
	{
		if (errno == EADDRNOTAVAIL) {
			L(LOG_ERROR, "[CN] Error %d. No free ports\n", errno);
			isOk = 0;
			return isOk;
		}
		if (errno != EINPROGRESS)
		{
			L(LOG_ERROR, "[CN] Connect error %d\n", errno);
			isOk = 0;
			return isOk;
		}
		int emptyPoll = 0;
		while (1)
		{
			struct pollfd poll_list[1];
			poll_list[0].fd = descr;
			poll_list[0].events = POLLOUT;
			poll_list[0].revents = 0;
			int retval = poll(poll_list, 1, nsec * 1000);

			if(retval < 0)
			{
				L(LOG_ERROR, "[CN] Error while polling: %s\n",strerror(errno));
				isOk = 0;
				return isOk;
			}
			if (retval == 0)
			{
#ifdef _SOCKET_LOG
				L(LOG_ERROR, "[CN] Timeout while polling: %s\n",strerror(errno));
#endif
				isOk = 0;
				return isOk;
			}
			if( (poll_list[0].revents&POLLHUP) == POLLHUP)
			{
				L(LOG_ERROR, "[CN] Poll error POLLHUP (%d-%d-%s) - %u\n", retval, errno, strerror(errno), nsec);
				isOk = 0;
				return isOk;
			}
			if ((poll_list[0].revents&POLLERR) == POLLERR)
			{
				L(LOG_ERROR, "[CN] Poll error POLLERR(%d-%d-%s) - %u\n", retval, errno, strerror(errno), nsec);
				isOk = 0;
				return isOk;
			}
			if ((poll_list[0].revents&POLLNVAL) == POLLNVAL)
			{
				L(LOG_ERROR, "[CN] Poll error POLLNVAL (%d-%d-%s) - %u\n", retval, errno, strerror(errno), nsec);
				isOk = 0;
				return isOk;
			}

			if ((poll_list[0].revents&POLLOUT) != POLLOUT)
			{
				emptyPoll++;
				if (emptyPoll < 3)
				{
					L(LOG_ERROR, "[CN] %u Get empty poll try continue error (%d-%d-%s) - %u\n", emptyPoll, retval, errno, strerror(errno), nsec);
					continue;
				}
				isOk = 0;
				L(LOG_ERROR, "[CN]  %u Get empty poll try continue error (%d-%d-%s) - %u\n", emptyPoll, retval, errno, strerror(errno), nsec);
				isOk = 0;
				return isOk;
			}
			int error = 0;
			socklen_t len = sizeof(error);
			if (getsockopt(descr, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
			{
				L(LOG_ERROR, "[CN] Error wait work\n");
				isOk = 0;
				return isOk;
			}
			if (error)
			{
				L(LOG_WARNING, "[CN] Error connected %d\n\n", error);
				isOk = 0;
				return isOk;
			}
			break;
		}
	}
	SetBlock();
	return isOk;
}

int SocketClient::Connect(unsigned int ip, int port)
{
	sockaddr_in addr;
	memset(&addr, 0, sizeof(sockaddr_in));
	addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = ntohl(ip);
	addr.sin_port = htons(port);
	if (::connect(descr, (sockaddr *)&addr, sizeof(addr)))
	{
		if (errno == EISCONN)
			return EISCONN;
#ifdef _SOCKET_LOG
		printf("Connected error\n");
#endif
		isOk = 0;
		return isOk;
	}
	return isOk;
}

int SocketClient::Connect(const char *host, int port)
{
	sockaddr_in addr;
	memset(&addr, 0, sizeof(sockaddr_in));
	addr.sin_family = AF_INET;
#ifdef _THREAD_SAFE
	mutex.LockData();
#endif
	struct hostent *hp = gethostbyname(host);
	if (hp==0)
	{
		printf("Get Host Error\n");
		isOk = 0;
#ifdef _THREAD_SAFE
		mutex.UnLockData();
#endif
		return 0;
	}
	memcpy(&addr.sin_addr, hp->h_addr, hp->h_length);
#ifdef _THREAD_SAFE
	mutex.UnLockData();
#endif
	addr.sin_port = htons(port);
	if (::connect(descr, (sockaddr *)&addr, sizeof(addr)))
	{
		if (errno == EISCONN)
			return EISCONN;
#ifdef _SOCKET_LOG
		printf("Connected error\n");
#endif
		isOk = 0;
		return isOk;
	}
	return isOk;
}

int SocketClient::Read(char *buf, int len, int flag, int timeOut)
{
	struct pollfd poll_list[1];
	int emptyPoll = 0;
  do
  {
		poll_list[0].fd = descr;
		poll_list[0].events = POLLIN|POLLPRI;
		poll_list[0].revents = 0;
		int retval = poll(poll_list, 1, timeOut * 1000);
		if(retval < 0)
    {
	    L(LOG_ERROR, "[RD] Error while polling: %s\n",strerror(errno));
      isOk = 0;
      return 0;
    }
		if (retval == 0)
		{
#ifdef _SOCKET_LOG
			L(LOG_ERROR, "[RD] Timeout while polling: %s\n",strerror(errno));
#endif
			isOk = 0;
 			return 0;
		}
    if(((poll_list[0].revents&POLLHUP) == POLLHUP) ||
      ((poll_list[0].revents&POLLERR) == POLLERR) ||
      ((poll_list[0].revents&POLLNVAL) == POLLNVAL))
    {
			L(LOG_ERROR, "[RD] Poll error (%d-%d-%s) - %u\n", retval, errno, strerror(errno), timeOut);
			isOk = 0;
			return 0;
    }
    if ((poll_list[0].revents&POLLIN) != POLLIN)
    {
			emptyPoll++;
			if (emptyPoll < 3)
							continue;
			isOk = 0;
			L(LOG_ERROR, "Get empty poll try continue error (%d-%d-%s) - %u\n", retval, errno, strerror(errno), timeOut);
			return 0;
    }
    break;
  } while (true);
	return recv(descr,  buf, len, MSG_NOSIGNAL|MSG_DONTWAIT);
}


int SocketClient::ReadBin(char * buf, int len, int timeout, int flag)
{
	int	readNum;
	int curReadLen = 0;

  struct pollfd poll_list[1];
  int retval;
	int emptyPoll = 0;

	for (;;)
	{
		poll_list[0].fd = descr;
		poll_list[0].events = POLLIN|POLLPRI;
		poll_list[0].revents = 0;

		retval = poll(poll_list, 1, timeout * 1000);
		if(retval < 0)
		{
			L(LOG_ERROR, "Error while polling: %s\n",strerror(errno));
			isOk = 0;
			break;
		}
		if (retval == 0)
		{
#ifdef _SOCKET_LOG
			L(LOG_ERROR, "Timeout while polling: %s\n",strerror(errno));
#endif
			isOk = 0;
			break;
		}

		if(	((poll_list[0].revents&POLLHUP) == POLLHUP) ||
			((poll_list[0].revents&POLLERR) == POLLERR) ||
			((poll_list[0].revents&POLLNVAL) == POLLNVAL))
		{
			L(LOG_ERROR, "Poll error (%d-%d-%s) - %u - %u\n", retval, errno, strerror(errno), curReadLen, timeout);
			isOk = 0;
			break;
		}
		if ((poll_list[0].revents&POLLIN) != POLLIN)
		{
			emptyPoll++;
			if (emptyPoll < 3)
				continue;
			isOk = 0;
			L(LOG_ERROR, "Get empty poll try continue error (%d-%d-%s) - %u - %u\n", retval, errno, strerror(errno), curReadLen, timeout);
			break;
		}


		readNum = recv(descr,  buf+curReadLen, len-curReadLen, MSG_NOSIGNAL | MSG_DONTWAIT);
		if (readNum == 0)
		{
			if (flag == RETURN_RECV_SIZE)
				return curReadLen;
			else
			{
		#ifdef _SOCKET_LOG
				L(LOG_ERROR, "EOF encountered on input (%d)\n", errno);
		#endif
				isOk = 0;
				return 0;
			}
		}
		else if (curReadLen+readNum>=len)
			return len;
		else if (errno == EAGAIN)
		{
			if (curReadLen+readNum >= len)
				return len;
			else
			{
				curReadLen += readNum;
				continue;
			}
		}
		else if (readNum < 0)
		{
		#ifdef _SOCKET_LOG
			L(LOG_ERROR, "Read from descriptor (%d)\n", errno);
		#endif
			isOk = 0;
			break;
		}
		curReadLen += readNum;
	}
	if (isOk || flag == RETURN_RECV_SIZE)
		return curReadLen;
	else
		return 0;
	return curReadLen;
}

int SocketClient::ReadHttpPost(char *buf, int len, int flag)
{
	len--; // allow one byte for \0
	if (len <= 0)
		return 0;
	int	readNum;
	int curReadLen = 0;
	char *pbuf = NULL;
	int contentLength = 0;
	int headerSize = 0;

  struct pollfd poll_list[1];
  int retval;
	int emptyPoll = 0;
	int timeout = SOCK_WAIT_REQUEST;
	for (;;)
	{
		poll_list[0].fd = descr;
		poll_list[0].events = POLLIN|POLLPRI;
		poll_list[0].revents = 0;

		retval = poll(poll_list, 1, timeout * 1000);
		if(retval < 0)
		{
			L(LOG_ERROR, "Error while polling: %s\n",strerror(errno));
			isOk = 0;
			break;
		}
		if (retval == 0)
		{
#ifdef _SOCKET_LOG
			L(LOG_ERROR, "Timeout while polling: %s\n",strerror(errno));
#endif
			isOk = 0;
			break;
		}

		if(	((poll_list[0].revents&POLLHUP) == POLLHUP) ||
			((poll_list[0].revents&POLLERR) == POLLERR) ||
			((poll_list[0].revents&POLLNVAL) == POLLNVAL))
		{
			L(LOG_ERROR, "Poll error (%d-%d-%s) - %u - %u\n", retval, errno, strerror(errno), curReadLen, timeout);
			isOk = 0;
			break;
		}
		if ((poll_list[0].revents&POLLIN) != POLLIN)
		{
			emptyPoll++;
			if (emptyPoll < 3)
				continue;
			isOk = 0;
			L(LOG_ERROR, "Get empty poll try continue error (%d-%d-%s) - %u - %u\n", poll_list[0].revents, errno, strerror(errno), curReadLen, timeout);
			break;
		}		readNum = recv(descr,  buf+curReadLen, len-curReadLen, flag|MSG_NOSIGNAL);
		if (readNum == 0)
		{
#ifdef _SOCKET_LOG
			L(LOG_ERROR, "EOF encountered on input\n");
#endif
			isOk = 0;
			return 0;
		}
		else if (readNum < 0)
		{
#ifdef _SOCKET_LOG
			L(LOG_ERROR, "Read from descriptor (%d)\n", errno);
#endif
			return 0;
		}

		buf[curReadLen + readNum] = 0;
		if (!contentLength && ((pbuf = strcasestr(buf+curReadLen, "Content-length: ")) != NULL))
			contentLength = strtol(pbuf+16, (char **)NULL, 10);
		if (!headerSize && ((pbuf = strstr(buf+curReadLen, "\r\n\r\n")) != NULL))
			headerSize = pbuf - buf;
		if (headerSize && contentLength && headerSize + contentLength <= curReadLen + readNum)
		{
			curReadLen += readNum;
			break;
		}
		curReadLen += readNum;
		if (buf[0] != 'P' && buf[curReadLen-1] == '\n' && buf[curReadLen-2] == '\r' && buf[curReadLen-3] == '\n')
				break;
	}
	buf[curReadLen] = '\0';
	return curReadLen;
}

int SocketClient::ReadHttp(char *buf, int len, int timeout, int flag)
{
	len--; // allow one byte for \0
	if (len <= 0)
		return 0;
	int	readNum;
	int curReadLen = 0;

  struct pollfd poll_list[1];
  int retval;
	int emptyPoll = 0;

	for (;;)
	{
		poll_list[0].fd = descr;
		poll_list[0].events = POLLIN|POLLPRI;
		poll_list[0].revents = 0;

		retval = poll(poll_list, 1, timeout * 1000);
		if(retval < 0)
		{
			L(LOG_ERROR, "Error while polling: %s\n",strerror(errno));
			isOk = 0;
			break;
		}
		if (retval == 0)
		{
#ifdef _SOCKET_LOG
			L(LOG_ERROR, "Timeout while polling: %s\n",strerror(errno));
#endif
			isOk = 0;
			break;
		}

		if(	((poll_list[0].revents&POLLHUP) == POLLHUP) ||
			((poll_list[0].revents&POLLERR) == POLLERR) ||
			((poll_list[0].revents&POLLNVAL) == POLLNVAL))
		{
			L(LOG_ERROR, "Poll error (%d-%d-%s) - %u - %u\n", retval, errno, strerror(errno), curReadLen, timeout);
			isOk = 0;
			break;
		}
		if ((poll_list[0].revents&POLLIN) != POLLIN)
		{
			emptyPoll++;
			if (emptyPoll < 3)
				continue;
			isOk = 0;
			L(LOG_ERROR, "Get empty poll try continue error (%d-%d-%s) - %u - %u\n", poll_list[0].revents, errno, strerror(errno), curReadLen, timeout);
			break;
		}

		readNum = recv(descr,  buf+curReadLen, len-curReadLen, flag|MSG_NOSIGNAL);
		if (readNum == 0)
		{
#ifdef _SOCKET_LOG
			L(LOG_ERROR, "EOF encountered on input\n");
#endif
			isOk = 0;
			return 0;
		}
		else if (readNum < 0)
		{
#ifdef _SOCKET_LOG
			L(LOG_ERROR, "Read from descriptor (%d)\n", errno);
#endif
			return 0;
		}
		curReadLen += readNum;
		if (buf[curReadLen-1] == '\n' && buf[curReadLen-2] == '\r' && buf[curReadLen-3] == '\n')
			break;
	}
	buf[curReadLen] = '\0';
	return curReadLen;
}

int SocketClient::ReadUntilStr(RString &buf, const char *endTemple, int timeout, int flag) // Read until find endTemple
{
	int	readNum;
	int curReadLen = 0;

	int templLen = strlen(endTemple);
	if (!templLen)
		return 0;
	int len = buf.MaxSize() - 1;
	buf.GetBuf(len);

  struct pollfd poll_list[1];
  int retval;
	int emptyPoll = 0;

	for (;;)
	{
		poll_list[0].fd = descr;
		poll_list[0].events = POLLIN|POLLPRI;
		poll_list[0].revents = 0;

		retval = poll(poll_list, 1, timeout * 1000);
		if(retval < 0)
		{
			L(LOG_ERROR, "Error while polling: %s\n",strerror(errno));
			isOk = 0;
			break;
		}
		if (retval == 0)
		{
#ifdef _SOCKET_LOG
			L(LOG_ERROR, "Timeout while polling: %s\n",strerror(errno));
#endif
			isOk = 0;
			break;
		}

		if(	((poll_list[0].revents&POLLHUP) == POLLHUP) ||
			((poll_list[0].revents&POLLERR) == POLLERR) ||
			((poll_list[0].revents&POLLNVAL) == POLLNVAL))
		{
			L(LOG_ERROR, "Poll error (%d-%d-%s) - %u - %u\n", retval, errno, strerror(errno), curReadLen, timeout);
			isOk = 0;
			break;
		}
		if ((poll_list[0].revents&POLLIN) != POLLIN)
		{
			emptyPoll++;
			if (emptyPoll < 3)
				continue;
			isOk = 0;
			L(LOG_ERROR, "Get empty poll try continue error (%d-%d-%s) - %u - %u\n", poll_list[0].revents, errno, strerror(errno), curReadLen, timeout);
			break;
		}
		if (len - curReadLen <= 0)
		{
			buf.GetBuf(len);
			len = buf.Length();
		}

		readNum = recv(descr,  (char*)buf + curReadLen, len - curReadLen, flag|MSG_NOSIGNAL);
		if (readNum == 0)
		{
#ifdef _SOCKET_LOG
			L(LOG_ERROR, "EOF encountered on input\n");
#endif
			isOk = 0;
			return 0;
		}
		else if (readNum < 0)
		{
#ifdef _SOCKET_LOG
			L(LOG_ERROR, "Read from descriptor (%d)\n", errno);
#endif
			return 0;
		}
		curReadLen += readNum;
		if (curReadLen >= templLen && (!strncasecmp(buf.Data()+(curReadLen-templLen), endTemple, templLen)))
			break;
	}
	buf.SetLength(curReadLen);
	return curReadLen;
}

int SocketClient::ReadImapAnswer(RString &buf, const char **cmdLabel, int timeout, int flag) // Read until find endTemple
{
	int	readNum;
	int curReadLen = 0;

	int labelLen = strlen(*cmdLabel);
	if (!labelLen)
		return 0;
	int len = buf.MaxSize() - 1;
	buf.GetBuf(len);

  struct pollfd poll_list[1];
  int retval;
	int emptyPoll = 0;

	for (;;)
	{
		poll_list[0].fd = descr;
		poll_list[0].events = POLLIN|POLLPRI;
		poll_list[0].revents = 0;

		retval = poll(poll_list, 1, timeout * 1000);
		if(retval < 0)
		{
			L(LOG_ERROR, "Error while polling: %s\n",strerror(errno));
			isOk = 0;
			break;
		}
		if (retval == 0)
		{
#ifdef _SOCKET_LOG
			L(LOG_ERROR, "Timeout while polling: %s\n",strerror(errno));
#endif
			isOk = 0;
			break;
		}

		if(	((poll_list[0].revents&POLLHUP) == POLLHUP) ||
			((poll_list[0].revents&POLLERR) == POLLERR) ||
			((poll_list[0].revents&POLLNVAL) == POLLNVAL))
		{
			L(LOG_ERROR, "Poll error (%d-%d-%s) - %u - %u\n", retval, errno, strerror(errno), curReadLen, timeout);
			isOk = 0;
			break;
		}
		if ((poll_list[0].revents&POLLIN) != POLLIN)
		{
			emptyPoll++;
			if (emptyPoll < 3)
				continue;
			isOk = 0;
			L(LOG_ERROR, "Get empty poll try continue error (%d-%d-%s) - %u - %u\n", poll_list[0].revents, errno, strerror(errno), curReadLen, timeout);
			break;
		}

		if (len - curReadLen <= 0)
		{
			buf.GetBuf(len);
			len = buf.Length();
		}

		readNum = recv(descr,  (char*)buf + curReadLen, len - curReadLen, flag|MSG_NOSIGNAL);
		if (readNum == 0)
		{
#ifdef _SOCKET_LOG
			L(LOG_ERROR, "EOF encountered on input\n");
#endif
			isOk = 0;
			return 0;
		}
		else if (readNum < 0)
		{
#ifdef _SOCKET_LOG
			L(LOG_ERROR, "Read from descriptor (%d)\n", errno);
#endif
			return 0;
		}
		curReadLen += readNum;
		if (curReadLen >= labelLen && !strncmp(buf.Data() + curReadLen - 2, "\r\n", 2)) {
			if (!strncmp(buf.Data(), *cmdLabel, labelLen)) {
				*cmdLabel = buf.Data();
				break;
			}

			const char *p = buf.Data() + labelLen;
			const char *pEnd = buf.Data() + curReadLen;
			while ((p = strnstr(p, *cmdLabel, pEnd - p))) {
				if (*(p - 2) == '\r' && *(p - 1) == '\n')
					break;
				p += labelLen;
			}

			if (p) {
				*cmdLabel = p;
				break;
			}
		}
	}

	buf.SetLength(curReadLen);
	return curReadLen;
}

int SocketClient::ReadUntilStr(char *buf, int len, const char *endTemple, int timeout, int flag) // Read until find endTemple
{
	len--; // allow one byte for \0
	if (len <= 0)
		return 0;
	int	readNum;
	int curReadLen = 0;

	int templLen = strlen(endTemple);
	if (!templLen)
		return 0;

  struct pollfd poll_list[1];
  int retval;
	int emptyPoll = 0;

	for (;;)
	{
		poll_list[0].fd = descr;
		poll_list[0].events = POLLIN|POLLPRI;
		poll_list[0].revents = 0;

		retval = poll(poll_list, 1, timeout * 1000);
		if(retval < 0)
		{
			L(LOG_ERROR, "Error while polling: %s\n",strerror(errno));
			isOk = 0;
			break;
		}
		if (retval == 0)
		{
#ifdef _SOCKET_LOG
			L(LOG_ERROR, "Timeout while polling: %s\n",strerror(errno));
#endif
			isOk = 0;
			break;
		}

		if(	((poll_list[0].revents&POLLHUP) == POLLHUP) ||
			((poll_list[0].revents&POLLERR) == POLLERR) ||
			((poll_list[0].revents&POLLNVAL) == POLLNVAL))
		{
			L(LOG_ERROR, "Poll error (%d-%d-%s) - %u - %u\n", retval, errno, strerror(errno), curReadLen, timeout);
			isOk = 0;
			break;
		}
		if ((poll_list[0].revents&POLLIN) != POLLIN)
		{
			emptyPoll++;
			if (emptyPoll < 3)
				continue;
			isOk = 0;
			L(LOG_ERROR, "Get empty poll try continue error (%d-%d-%s) - %u - %u\n", poll_list[0].revents, errno, strerror(errno), curReadLen, timeout);
			break;
		}
		readNum = recv(descr,  buf+curReadLen, len-curReadLen, flag|MSG_NOSIGNAL);
		if (readNum == 0)
		{
#ifdef _SOCKET_LOG
			L(LOG_ERROR, "EOF encountered on input\n");
#endif
			isOk = 0;
			return 0;
		}
		else if (readNum < 0)
		{
#ifdef _SOCKET_LOG
			L(LOG_ERROR, "Read from descriptor (%d)\n", errno);
#endif
			return 0;
		}
		curReadLen += readNum;
		if (curReadLen >= templLen && (!strncasecmp(buf+(curReadLen-templLen), endTemple, templLen)))
			break;
	}
	buf[curReadLen] = '\0';
	return curReadLen;
}

int SocketClient::ReadUntilInnerStr(char *buf, int len, char **pFind, char *endTemple, int timeout, int flag) // Read until find endTemple
{
	len--; // allow one byte for \0
	if (len <= 0)
		return 0;
	int	readNum;
	int curReadLen = 0;

	int templLen = strlen(endTemple);
	if (!templLen)
		return 0;

  struct pollfd poll_list[1];
  int retval;
	int emptyPoll = 0;

	for (;;)
	{
		poll_list[0].fd = descr;
		poll_list[0].events = POLLIN|POLLPRI;
		poll_list[0].revents = 0;

		retval = poll(poll_list, 1, timeout * 1000);
		if(retval < 0)
		{
			L(LOG_ERROR, "Error while polling: %s\n",strerror(errno));
			isOk = 0;
			break;
		}
		if (retval == 0)
		{
#ifdef _SOCKET_LOG
			L(LOG_ERROR, "Timeout while polling: %s\n",strerror(errno));
#endif
			isOk = 0;
			break;
		}

		if(	((poll_list[0].revents&POLLHUP) == POLLHUP) ||
			((poll_list[0].revents&POLLERR) == POLLERR) ||
			((poll_list[0].revents&POLLNVAL) == POLLNVAL))
		{
			L(LOG_ERROR, "Poll error (%d-%d-%s) - %u - %u\n", retval, errno, strerror(errno), curReadLen, timeout);
			isOk = 0;
			break;
		}
		if ((poll_list[0].revents&POLLIN) != POLLIN)
		{
			emptyPoll++;
			if (emptyPoll < 3)
				continue;
			isOk = 0;
			L(LOG_ERROR, "Get empty poll try continue error (%d-%d-%s) - %u - %u\n", poll_list[0].revents, errno, strerror(errno), curReadLen, timeout);
			break;
		}
		readNum = recv(descr,  buf+curReadLen, len-curReadLen, flag|MSG_NOSIGNAL);
		if (readNum == 0)
		{
#ifdef _SOCKET_LOG
			L(LOG_ERROR, "EOF encountered on input\n");
#endif
			isOk = 0;
			return 0;
		}
		else if (readNum < 0)
		{
#ifdef _SOCKET_LOG
			L(LOG_ERROR, "Read from descriptor (%d)\n", errno);
#endif
			return 0;
		}
		if (readNum > templLen)
		{
			if (curReadLen < templLen)
				*pFind = strnstr(buf+curReadLen, endTemple, readNum);
			else
				*pFind = strnstr(buf+curReadLen-templLen, endTemple, readNum+templLen);
			if (*pFind)
			{
				curReadLen += readNum;
                                break;
			}
		}

		curReadLen += readNum;
	}
	buf[curReadLen] = '\0';
	return curReadLen;
}

int SocketClient::Send(const char *buf, const int len, const int timeout, const int flag)
{
	struct pollfd poll_list[1];
        int retval;

	int num_send = 0;
	int emptyPoll = 0;
	while ( num_send < len)
	{
		poll_list[0].fd = descr;
		poll_list[0].events = POLLOUT;
		poll_list[0].revents = 0;

		retval = poll(poll_list, 1, timeout * 1000);
		if (retval == 0)
		{
#ifdef _SOCKET_LOG
			L(LOG_ERROR, "Timeout while send polling: %s\n",strerror(errno));
#endif
			return -1;
		}

		if(retval < 0)
		{
			L(LOG_ERROR, "Error while send polling: %s\n",strerror(errno));
			return -1;
		}
		if( (poll_list[0].revents&POLLHUP) == POLLHUP)
		{
			isOk = 0;
			L(LOG_ERROR, "Poll error POLLHUP (%d-%d-%s) - %u - %u\n", retval, errno, strerror(errno), num_send, timeout);
			return -1;
		}
		if ((poll_list[0].revents&POLLERR) == POLLERR)
		{
			isOk = 0;
			L(LOG_ERROR, "Poll error POLLERR(%d-%d-%s) - %u - %u\n", retval, errno, strerror(errno), num_send, timeout);
			return -1;
		}
		if ((poll_list[0].revents&POLLNVAL) == POLLNVAL)
		{
			isOk = 0;
			L(LOG_ERROR, "Poll error POLLNVAL (%d-%d-%s) - %u - %u\n", retval, errno, strerror(errno), num_send, timeout);
			return -1;
		}

		if ((poll_list[0].revents&POLLOUT) != POLLOUT)
		{
			emptyPoll++;
			if (emptyPoll < 3)
			{
				L(LOG_ERROR, "%u Get empty poll try continue error (%d-%d-%s) - %u of %u - %u\n", emptyPoll, retval, errno, strerror(errno), num_send, len, timeout);
				continue;
			}
			isOk = 0;
			L(LOG_ERROR, "%u Get empty poll try continue error (%d-%d-%s) - %u of %u - %u\n", emptyPoll, retval, errno, strerror(errno), num_send, len, timeout);
			return -1;
		}

			retval = ::send(descr, buf + num_send, len - num_send, flag|MSG_NOSIGNAL);
			if (retval > 0)
				num_send += retval;
			else
			{
				if (errno != EAGAIN)
				{
					L(LOG_ERROR, "SocketClient::Send error (%d-%d-%s)\n", retval, errno, strerror(errno));
					isOk = 0;
					return -1;
				}
				else
				{
					emptyPoll++;
					if (emptyPoll < 3)
					{
						L(LOG_ERROR, "%u Get EAGAIN error, try to continue (%d-%d-%s) - %u of %u - %u\n", emptyPoll, retval, errno, strerror(errno), num_send, len, timeout);
						continue;
					}
					isOk = 0;
					L(LOG_ERROR, "%u Get EAGAIN error, max retry count (%d-%d-%s) - %u of %u - %u\n", emptyPoll, retval, errno, strerror(errno), num_send, len, timeout);
					return -1;
				}
			}
	}
	return num_send;
}

int SocketClient::SetNonBlock()
{
	int flags = fcntl(descr, F_GETFL, 0);
	if (fcntl(descr, F_SETFL, flags | O_NONBLOCK))
	{
#ifdef _SOCKET_LOG
		L(LOG_ERROR, "Socket unblock set Error\n");
#endif
		isOk = 0;
		return 0;
	}
	return 1;
}

int SocketClient::SetBlock()
{
	int flags = fcntl(descr, F_GETFL, 0);
	if (fcntl(descr, F_SETFL, flags & (~O_NONBLOCK)))
	{
#ifdef _SOCKET_LOG
		L(LOG_ERROR, "Socket block set Error\n");
#endif
		isOk = 0;
		return 0;
	}
	return 1;
}

int SocketClient::BindOnIp(const char *bindIP, int port)
{
	sockaddr_in addr;
	memset(&addr, 0, sizeof(sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(bindIP);

//		INADDR_ANY;
	addr.sin_port = htons(port);
#ifdef BSD
	addr.sin_len = sizeof(sockaddr_in);
#endif
	if (bind(descr, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
#ifdef _SOCKET_LOG
		L(LOG_ERROR,"Init_socket: bind %d | %s\n",errno, strerror(errno));
#endif
		shutdown(descr,2);
		isOk = 0;
		return 0;
	}
	return 1;
}
int SocketClient::InitServer(int port, const char *listenIP)
{
	BOOL		opt = 1;
	sockaddr_in addr;
	memset(&addr, 0, sizeof(sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(listenIP);

//		INADDR_ANY;
	addr.sin_port = htons(port);
#ifdef BSD
	addr.sin_len = sizeof(sockaddr_in);
#endif



	if (setsockopt(descr, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) == SOCKET_ERROR)
	{
#ifdef _SOCKET_LOG
		L(LOG_ERROR, "Init_socket: SO_REUSEADDR");
#endif
		shutdown(descr,2);
		return 0;
		isOk = 0;
	}

	if (bind(descr, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
#ifdef _SOCKET_LOG
		L(LOG_ERROR,"Init_socket: bind %d | %s\n",errno, strerror(errno));
#endif
		shutdown(descr,2);
		isOk = 0;
		return 0;
	}

	if (listen(descr, MAX_LISTEN_BACKLOG) == SOCKET_ERROR)
	{
#ifdef _SOCKET_LOG
		L(LOG_ERROR,"Init_socket: listen");
#endif
		shutdown(descr,2);
		isOk = 0;
		return 0;
	}
	return 1;
}

int SocketClient::SendFile(char *fileName, char *buf, int bufLength)
{
	FILE *file = fopen(fileName, "r");
	if (file)
	{
		int len = 0;
		while ((len = fread(buf, 1, bufLength, file)) >= bufLength && len)
			Send(buf, len);
		if (len)
			Send(buf, len);
		fclose(file);
		return len;
	}
	return 0;
}

SocketClient *SocketClient::GetConnection()
{
//	static struct	timeval	null_time;
//	fd_set			inSet;
//	null_time.tv_usec  = 600;
//	FD_ZERO(&inSet);
//	FD_SET(descr, &inSet);
//	int opt = 1;

	struct	sockaddr_in	sock_addr;
	socklen_t	sa_size = sizeof(sock_addr);
	memset(&sock_addr, 0, sa_size);

	Descriptor newDescr = accept(descr, (struct sockaddr *)&sock_addr, &sa_size);
	if (newDescr == INVALID_SOCKET)
	{
#ifdef _SOCKET_LOG
		L(LOG_CRITICAL, "[ERROR] on accept() %d %s\n\r", errno, strerror(errno));
#endif
#ifdef WIN32
//	else if (ioctlsocket(newDescr, FIONBIO, &parm) == INVALID_SOCKET)
#endif
#ifdef LINUX
//	else if(fcntl(newDescr, F_SETFL, O_NONBLOCK ))
#endif
	//	printf("[ERROR] on ioctlsocket()\n\r");
	}
	else
	{
	/*	if (setsockopt(newDescr, SOL_SOCKET,	SO_REUSEADDR, (char *)&opt, sizeof(opt)) == SOCKET_ERROR)
		{
#ifdef _SOCKET_LOG
			L(LOG_ERROR, "Init_socket: KEEPALIV\n");
#endif
		}
		else*/
			return new SocketClient(newDescr, sock_addr);
	}
	return NULL;
}

unsigned int SocketClient::IpToLong(const char *host)
{
	return inet_network(host);
}

char *SocketClient::GetIPStr(unsigned int ip)
{
  sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = ntohl(ip);
	return inet_ntoa(addr.sin_addr);
}


char *SocketClient::GetIPStr()
{
  sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = ntohl(fromIP);
	return inet_ntoa(addr.sin_addr);
}
int SocketClient::IsBadIP(struct sockaddr_in sock_addr)
{
	char *fromNaddr = inet_ntoa(sock_addr.sin_addr);
	if (strcmp(fromNaddr, "127.0.0.1") && strcmp(fromNaddr, "62.149.0.49"))
	{
		printf(fromNaddr);
		return 1;
	}
	else
		return 0;
}

#ifdef LINUX
int SocketClient::SetDeferAccept(int val)
{
	if (setsockopt( descr, SOL_TCP, TCP_DEFER_ACCEPT, (char *)&val, sizeof(val) ))
	{
		printf("Cannot set TCP_DEFER_ACCEPT\n");
		return 0;
	}
	else
		return 1;
}
#endif

int SocketClient::SetNoDelay(int flag)
{
	if (setsockopt( descr, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag) ))
	{
		printf("Cannot set TCP_NODELAY\n");
		return 0;
	}
	else
		return 1;
}
int SocketClient::HasError()
{
	int error = 0;
	socklen_t len;

	if (getsockopt(descr, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
	{
#ifdef _SOCKET_LOG
	 	L(LOG_ERROR, "Error wait work\n");
#endif
		isOk = 0;
		return 1;
	}
	if (error)
	{
		isOk = 0;
		return 1;
	}
	else
		return 0;
}


/*new socket funcs */
void SocketClient::GetIPStr(RString &buf)
{
	GetIPStr(fromIP, buf);
}

void SocketClient::GetIPStr(const Word32 fromIP, class RString &buf)
{
	Word32 ip = ntohl(fromIP);
	Byte *ucp = (Byte *)&ip;
  /*buf.snaprintf("%d.%d.%d.%d",
               ucp[0] & 0xff,
               ucp[1] & 0xff,
               ucp[2] & 0xff,
               ucp[3] & 0xff);*/

	static const Word32 MAX_IP_LENGTH = 4 * 4;
	static const char DECIMALS[] = "0123456789";
	char *ipBuf = buf.GetBuf(MAX_IP_LENGTH);
	char *curBuf = ipBuf;
	for (Word32 i = 0; i < sizeof(Word32); i++)
	{
		Byte val = ucp[i];
		if (val >= 100)
		{
			if (val >= 200)
				*curBuf++ = '2';
			else
				*curBuf++ = '1';
			val %= 100;
			if (val < 10)
				*curBuf++ = '0';
		}
		if (val >= 10)
			*curBuf++ = DECIMALS[(Byte)(val / 10)];
		*curBuf++ = DECIMALS[(Byte)(val % 10)];
		*curBuf++ = '.';
	}
	buf.SetLength(buf.Length() - (MAX_IP_LENGTH - (curBuf - ipBuf - 1)));
}

SocketClient::ESocketResult SocketClient::PollRead(RString &buf, int timeout)
{
  struct pollfd poll_list[1];
  int retval;
	int emptyPoll = 0;

	poll_list[0].fd = descr;
	poll_list[0].events = POLLIN|POLLPRI;
	poll_list[0].revents = 0;

	while (true)
	{
		retval = poll(poll_list, 1, timeout * 1000);
		if(retval < 0)
		{
			L(LOG_ERROR, "Get socket error %u\n", errno);
			return ERROR;
		}
		if (retval == 0)
		{
			//L(LOG_ERROR, "Get TIME_OUT %u\n", timeout);
			return TIME_OUT;
		}

		if(	((poll_list[0].revents&POLLHUP) == POLLHUP) ||
			((poll_list[0].revents&POLLERR) == POLLERR) ||
			((poll_list[0].revents&POLLNVAL) == POLLNVAL))
		{
			return ERROR;
		}
		if ((poll_list[0].revents&POLLIN) != POLLIN)
		{
			emptyPoll++;
			if (emptyPoll < 3)
				continue;
			L(LOG_ERROR, "poll_list[0].revents=%x,poll_list[0].events=%x\n", poll_list[0].revents, poll_list[0].events);
			L(LOG_ERROR, "Get empty poll try continue error (%d-%d-%s) - %u\n", retval, errno, strerror(errno), timeout);
			return EMPTY_POLL;
		}
		int readChunk = buf.MaxSize() - buf.Length() - 1;
		if (readChunk <= 0)
			return OUT_OF_BUFFER;
		char *readBuf = buf.GetBuf(readChunk);
		int res = recv(descr,  readBuf, readChunk, MSG_NOSIGNAL | MSG_DONTWAIT);
		if (res > 0)
		{
			buf.SetLength(buf.Length() - (readChunk - res));
			return OK;
		}
		buf.SetLength(buf.Length() - readChunk);

		if (res == 0)
			return CONN_CLOSE;
		return ERROR;
	}
}

const char *endChars = "\r\n\r\n";
const int MIN_HTTP_REQ = 10;
const String CONTENT_LENGTH_HEADER("Content-length:");

bool SocketClient::ReadHttpQuery(RString &buf, int timeout)
{
	buf.Null();
	int lastParseCharPos = 0;
	int contentLength = 0;
	const char *curEndChar = endChars;
	while (true)
	{
		ESocketResult res = PollRead(buf, timeout);
		if ((res != OK) && (res !=  CONN_CLOSE) && (res != EMPTY_POLL))
			return false;

		if (buf.Length() < MIN_HTTP_REQ)
		{
			if ((res == CONN_CLOSE) || (res == EMPTY_POLL))
				return false;
			continue;
		}

		char *beginBuf = buf;
		char *pBuf = beginBuf + lastParseCharPos;
		char *beginHeader = pBuf;
		for ( ; lastParseCharPos < buf.Length(); lastParseCharPos++)
		{
			if ((*pBuf == *curEndChar))
			{
				if (*curEndChar == '\n')
				{
					if ((*beginBuf == 'P') && (contentLength == 0)) // post request
					{
						if (!strncasecmp(beginHeader, CONTENT_LENGTH_HEADER.c_str(), CONTENT_LENGTH_HEADER.GetLength()))
						{
							beginHeader += CONTENT_LENGTH_HEADER.GetLength();
							while (*beginHeader == ' ')
								beginHeader++;
							contentLength = atoi(beginHeader);
						}
					}
					beginHeader = pBuf + 1;
				}
				curEndChar++;
				if (!*curEndChar) // \r\n\r\n
					break;
			}
			else
				curEndChar = endChars;
			pBuf++;
		}
		if (!*curEndChar)
		{
			if (contentLength > 0)
			{
				int needReadData =  contentLength + lastParseCharPos;
				while  (needReadData > buf.Length())
				{
					ESocketResult res = PollRead(buf, timeout);
					if (res != OK)
						return false;
				}
				return true;
			}
			else
				return true;
		}
		else if (res == CONN_CLOSE || res == EMPTY_POLL)
		{
			L(LOG_ERROR, "Get res == CONN_CLOSE || res == EMPTY_POLL %u\n", res);
			return ERROR;
		}
	}
}
