#include <unistd.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include "log.hpp"
extern int h_errno, errno;

#include "socket_raw.hpp"

using namespace lb;

TransportBase::TDescriptor TransportBase::open(const unsigned int ip, const short int port, const int timeOut)
{
	if (_descr != INVALID_SOCKET) {
		L(LOG_ERROR, "TransportBase::connect(): warning: _descr exists\n");
	} else {
		_descr = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
		if (_descr == INVALID_SOCKET) {
			return 0;
		}
		int flags = fcntl(_descr, F_GETFL, 0);
		if (fcntl(_descr, F_SETFL, flags | O_NONBLOCK))
			return 0;
	}

	sockaddr_in addr;
	memset(&addr, 0, sizeof(sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ntohl(ip);
	addr.sin_port = htons(port);

	int n = 0;
	if ((n = ::connect(_descr, (sockaddr *) &addr, sizeof(addr))) != 0)
	{
		if (errno != EINPROGRESS)
		{
			L(LOG_ERROR, "TransportBase::connect(): error: connect error %d\n", errno);
			return 0;
		}
		while (1)
		{
			if (!_waitFor(POLLOUT, timeOut))
				return 0;
			int error = 0;
			socklen_t len = sizeof(error);
			if (getsockopt(_descr, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
			{
				L(LOG_ERROR, "TransportBase::connect(): error: wait work\n");
				return 0;
			}
			if (error)
			{
				L(LOG_WARNING, "TransportBase::connect(): warning: connected %d\n\n", error);
				return 0;
			}
			break;
		}
	}
	return _descr;
}

int TransportBase::close()
{
	if (_descr != INVALID_SOCKET) {
		shutdown(_descr, 2);
		::close(_descr);
	}
	_descr = INVALID_SOCKET;

	return 1;
}

bool TransportBase::_waitFor(const int events, const int timeOut)
{
	struct pollfd poll_list[1];
	poll_list[0].fd = _descr;
	poll_list[0].events = events;
	poll_list[0].revents = 0;

	int retval = poll(poll_list, 1, timeOut * 1000);
	if(retval < 0) {
		L(LOG_ERROR, "Error while polling: %s\n", strerror(errno));
		return false;
	}
	if (retval == 0) {
		L(LOG_ERROR, "Timeout while polling: %s\n", strerror(errno));
		return false;
	}

	if (((poll_list[0].revents & POLLHUP) == POLLHUP)
		|| ((poll_list[0].revents & POLLERR) == POLLERR)
		|| ((poll_list[0].revents & POLLNVAL) == POLLNVAL)
	) {
		L(LOG_ERROR, "Poll error (%d-%d-%s) - %d\n", retval, errno, strerror(errno), timeOut);
		return false;
	}
	if (events == POLLIN) {
		if ((poll_list[0].revents & POLLIN) != POLLIN)
			return false;
	}
	if (events == POLLOUT) {
		if ((poll_list[0].revents & POLLOUT) != POLLOUT)
			return false;
	}

	return true;
}

int TransportBase::read(char *buf, const size_t len, const int timeOut)
{
	size_t curLen = 0;
	while (curLen < len) {
		if (!_waitFor(POLLIN, timeOut))
			return 0;

		int num = _read(buf + curLen, len - curLen);
		if (num == 0) {
			return 0;
		} else if (num < 0) {
			if (errno == EAGAIN)
				continue;

			L(LOG_ERROR, "Poll read error (%d-%d-%s)\n", num, errno, strerror(errno));
			return 0;
		}

		curLen += num;
	}

	return curLen;
}

int TransportBase::write(const char *buf, const size_t len, const int timeOut)
{
	size_t curLen = 0;
	while (curLen < len) {
		if (!_waitFor(POLLOUT, timeOut))
			return 0;

		int num = _write(buf + curLen, len - curLen);
		if (num == 0) {
			return 0;
		} else if (num < 0) {
			if (errno == EAGAIN)
				continue;

			L(LOG_ERROR, "Poll write error (%d-%d-%s)\n", num, errno, strerror(errno));
			return 0;
		}

		curLen += num;
	}

	return curLen;
}

int TransportBase::readImapAnswer(RString &buf, const char **cmdLabel, const int timeOut)
{
	int labelLen = strlen(*cmdLabel);
	if (!labelLen)
		return 0;
	int readNum;
	int curReadLen = 0;
	int len = buf.MaxSize() - 1;
	int lastEndRN = 0;

	while (true) {
		if (len - curReadLen <= 0) {
			buf.SetLength(curReadLen);
			buf.GetBuf(len);
			len = buf.Length();
		}

		if (!_waitFor(POLLIN, timeOut)) {
			L(LOG_ERROR, "Timeout while polling: %s\n", strerror(errno));
			return 0;
		}

		readNum = _read((char *) buf + curReadLen, len - curReadLen);
		if (readNum == 0) {
			return 0;
		} else if (readNum < 0) {
			if (errno == EAGAIN)
				continue;

			L(LOG_ERROR, "Poll read error (%d-%d-%s)\n", readNum, errno, strerror(errno));
			return 0;
		}
		curReadLen += readNum;

		const char *pEnd = buf.Data() + curReadLen - 2;
		if (*pEnd == '\r' && *(pEnd + 1) == '\n') {
			const char *pBegin = buf.Data() + lastEndRN;
			lastEndRN = curReadLen;

			const char *p = (const char *) memrchr(pBegin, '\n', pEnd - pBegin);
			if (p != NULL) {
				if (*(p - 1) != '\r')
					continue;

				pBegin = p + 1;
			}

			if (pEnd - pBegin < labelLen)
				continue;

			if (!memcmp(pBegin, *cmdLabel, labelLen)) {
				*cmdLabel = pBegin;
				break;
			}
		}
	}

	buf.SetLength(curReadLen);
	return curReadLen;
}

int TransportBase::readUntilStr(char *buf, int len, const char *endTemple, const int timeOut) // Read until find endTemple
{
	len--; // allow one byte for \0
	if (len <= 0)
		return 0;
	int templLen = strlen(endTemple);
	if (!templLen)
		return 0;
	int curLen = 0;

	while (curLen < len) {
		if (!_waitFor(POLLIN, timeOut)) {
			L(LOG_ERROR, "Timeout while polling: %s\n", strerror(errno));
			return 0;
		}

		int num = _read(buf + curLen, len - curLen);
		if (num == 0) {
			return 0;
		} else if (num < 0) {
			if (errno == EAGAIN)
				continue;

			L(LOG_ERROR, "Poll read error (%d-%d-%s)\n", num, errno, strerror(errno));
			return 0;
		}
		curLen += num;

		if (curLen >= templLen && (!memcmp(buf + (curLen - templLen), endTemple, templLen)))
			break;
	}

	buf[curLen] = '\0';
	return curLen;
}


int Socket::_read(char *buf, const size_t len)
{
	return recv(_descr,  buf, len, MSG_NOSIGNAL | MSG_DONTWAIT);
}

int Socket::_write(const char *buf, const size_t len)
{
	return send(_descr, buf, len, MSG_NOSIGNAL);
}
