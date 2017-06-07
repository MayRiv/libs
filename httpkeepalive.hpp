#pragma once

#include "httpevents.hpp"

namespace lb
{

	class ErrorSetter
	{
	public:
		ErrorSetter();
	};

	class HttpKeepAlivePostEvent : public HttpPostEvent
	{
	public:
		static const char* const CONNECTION_HEADER_KEEPALIVE;
		static const char* const CONNECTION_HEADER_CLOSE; 
		HttpKeepAlivePostEvent(SocketClient *client, class EHttpThread *thread, HttpEventListener *listener);
		virtual ~HttpKeepAlivePostEvent() {};
		virtual void Call(int events);
		virtual void Info(RString &buf, Time &curTime);
		virtual void SendErrorPage();
		void SendErrorPageAndClose();
		virtual void sendAnswer();
	protected:
		bool _TrySendAnswer();
		bool _ReceiveQuery();
		bool _ParseURI(char *beginHeader, char *endHeader, bool usingSSL);
		bool _ParseHeader(char *beginHeader, char *endHeader);
		bool _checkKeepAlive(char *beginHeader, int valLen);
		virtual void _reset();
		bool _isKeepAlive;
		Word32 _countRequests;
		static RString _errorPageClose;
		static RString _errorPageKeep;
		friend class ErrorSetter;
	};
}
