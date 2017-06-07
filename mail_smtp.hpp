#pragma once

const int MAX_ANSWER_BUF								= 1024;
const char * const DEFAULT_SMTP_SERVER	= "127.0.0.1";
const int DEFAULT_SMTP_PORT = 25;
const char * const DEFAULT_HELO_HOST  = "localhost";

#include "string.hpp"
#include "socket.hpp"

namespace lb
{
	class MailSMTP
	{
	public:
		MailSMTP(const char *host = NULL)
			:hostName(host ? host : DEFAULT_HELO_HOST), isConnected(false), messageBuf(MAX_ANSWER_BUF, MAX_ANSWER_BUF)
		{
		}
		bool Connect(const char *server = DEFAULT_SMTP_SERVER, const int port = DEFAULT_SMTP_PORT);
		bool Send(const char *from, const char *to, const char *buf, int bufLen, const char *appendHeader = NULL);
		void Close();
		~MailSMTP()
		{
			Close();
		}

	private:
		inline bool CheckResponse(const char status[3]);

		class String hostName;
		SocketClient socket;
		bool isConnected;
		class RString messageBuf;		// Answer string buffer
	};
};
