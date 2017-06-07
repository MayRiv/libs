#include <stdlib.h>
#include "tools.hpp"
#include "socket.hpp"
#include "log.hpp"
#include "mail_smtp.hpp"

using namespace lb;

inline bool MailSMTP::CheckResponse(const char status[3])
{
	char *buf = messageBuf;
	int res = socket.Read(buf, messageBuf.MaxSize());
	if (res < 4)
		return false;
	//L(LOG_INFO, "S: %s", messageBuf.Data());
	if (buf[0] == status[0] /*&& buf[1] == status[1] && buf[2] == status[2]*/)
		return true;
	return false;
}

bool MailSMTP::Connect(const char *server, const int port)
{
	Close();
	if (socket.Connect(server, port))
	{
		if (!CheckResponse("220"))
		{
			socket.Shutdown();
			return false;
		}
		messageBuf.snprintf("HELO %s\r\n", hostName.c_str());
		if (socket.Send(messageBuf.Data(), messageBuf.Length()) == -1)
		{
			socket.Shutdown();
			return false;
		}
		if (!CheckResponse("250"))
		{
			socket.Shutdown();
			return false;
		}
		isConnected = true;
		return true;
	}
	return false;
}

void MailSMTP::Close()
{
	if (isConnected)
	{
		if (socket.Send("QUIT\r\n", 6) == -1)
		{
			socket.Shutdown();
			isConnected = false;
			return;
		}

		CheckResponse("221");

		socket.Shutdown();
		isConnected = false;
	}
	return;
}

bool MailSMTP::Send(const char *from, const char *to, const char *buf, int bufLen, const char *appendHeader)
{
	if (!isConnected)
	{
		return false;
	}

	messageBuf.snprintf("MAIL FROM:<%s>\r\n", from);
	if (socket.Send(messageBuf.Data(), messageBuf.Length()) == -1)
		return false;	
	if (!CheckResponse("250"))
		return false;

	messageBuf.snprintf("RCPT TO:<%s>\r\n", to);
	if (socket.Send(messageBuf.Data(), messageBuf.Length()) == -1)
		return false;	
	if (!CheckResponse("250"))
		return false;

	if (socket.Send("DATA\r\n", 6) == -1)
		return false;	
	if (!CheckResponse("354"))
		return false;

	// send data with transparency
	if (appendHeader)
	{
		if (socket.Send(appendHeader, strlen(appendHeader)) == -1)
			return false;	
	}
	const char *p1 = buf;
	char *p2;
	const char *pEnd = buf + bufLen;
	if (*p1 == '.')
	{
		if (socket.Send(".", 1) == -1)
			return false;
	}
	while ((p2 = strnstr(p1, "\n.", pEnd - p1)))
	{
		if (socket.Send(p1, ++p2 - p1) == -1)
			return false;
		if (socket.Send(".", 1) == -1)
			return false;
		p1 = p2;
	}
	if (socket.Send(p1, pEnd - p1) == -1)
		return false;

	if (socket.Send("\r\n.\r\n", 5) == -1)
		return false;
	if (!CheckResponse("250"))
		return false;

	return true;
}
