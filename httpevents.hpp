#pragma once

#include "eventpoller.hpp"
#include "string.hpp"
#include "log.hpp"
#include "time.hpp"
#include "socket.hpp"
#include "mutexsync.hpp"
#include "httpsocket.hpp"

namespace lb
{
	class AcceptEvent : public Event
	{
	public:
		AcceptEvent(class SocketClient *server, class EHttpThread *thread, class HttpEventFactory* factory);
		virtual ~AcceptEvent();
		virtual void Call(int events);
	private:
		class EHttpThread *_thread;
		SocketClient *_server;
		class HttpEventFactory* _eventFactory;
	};

	enum EHttpReqType 
	{
		T_HTTP_GET,
		T_HTTPS_GET,
		T_HTTP_POST,
		T_HTTP_HEAD,
	};

	class HttpEventListener
	{
	public:
		enum EFormAnswer
		{
			ANS_ERROR,
			ANS_WAIT_TIMER,
			ANS_WAIT_RESULT,
			ANS_OK,
			ANS_SEND_N_CLOSE
		};
		virtual ~HttpEventListener() {}
		virtual bool ParseQuery(const EHttpReqType httpReqType, String &host, String &req, String &query) 
		{
			return true;
		};
		virtual bool ParsePostQuery(const char *content, const int contentLength)
		{
			return true;
		}
		virtual bool ParseHeader(int nameLen, char *beginHeaderName, char *beginHeader, char *endHeader, int len)
		{
			return true;
		}
		virtual EFormAnswer FormAnswer(RString &buf, class HttpEvent *http)
		{
			buf.snprintf("HTTP/1.0 400 Bad Request\r\nConnection: close\r\nContent-Length: 0\r\n\r\n");
			return ANS_OK;
		}

		//required by HttpKeepAliveEvent
		// in case isKeepAlive true - must be set "Connection: keep-alive", proper Content-Length and return ANS_OK; to close connection properly set "Connection: close" and return ANS_SEND_N_CLOSE, or just return ANS_ERROR for default error(also must have "Connection: close")
		// in case isKeepAlive false - always set "Connection: close"
		virtual EFormAnswer FormKeepAliveAnswer(RString &buf, class HttpEvent *http, bool isKeepAlive)
		{
			buf.snprintf("HTTP/1.0 400 Bad Request\r\nConnection: close\r\nContent-Length: 0\r\n\r\n");
			return ANS_SEND_N_CLOSE;
		}

		virtual bool callTimer(class HttpEvent *http)
		{
			return true;
		}
		virtual void WaitQueryTimeOut() {};
		virtual void TimeOut() {};
		virtual void reset() {};


		typedef Byte TOsID;
		typedef Byte TBrwsID;
		

		static const int  OS_WIN95			= 1;
		static const int  OS_WINNT			= 2;
		static const int  OS_WIN3X			= 3;
		static const int  OS_WIN98			= 10;
		static const int  OS_WINME			= 11;
	
		static const int  OS_WIN2000		= 12;
		static const int  OS_WINXP			= 13;
		static const int  OS_WIN2003		= 14;
		static const int  OS_WINVISTA		= 15;
		static const int  OS_WIN7			= 20;
		static const int  OS_WIN8			= 22;
		static const int  OS_WIN10			= 23;

		static const int  OS_J2ME			= 18;
		static const int  OS_WINMOBILE		= 17;
		static const int  OS_SYMBIAN		= 16;
	
		static const int  OS_LINUX			= 4;
	//	const int  OS_FBSD			= 5;//old
	//	const int  OS_SUN			= 6;//old
		static const int  OS_MAC			= 9;
		static const int  OS_IOS			= 19;
		static const int  OS_ANDROID		= 21;


		static const int  BR_OPERA   = 19;
		static const int  BR_OPERA10   = 20;
		static const int  BR_OPERA11   = 27;
		static const int  BR_OPERA12   = 29;
		static const int  BR_OPERAMOBILE= 22;
		static const int  BR_OPERA15   = 28;

		static const int  BR_MSIE		= 2;//early versions
	//	const int  BR_MSIE5		= 1;
//		static const int  BR_MSIE6		= 5;
		static const int  BR_MSIE7		= 6;
		static const int  BR_MSIE8		= 3;
		static const int  BR_MSIE9		= 7;
		static const int  BR_MSIE10		= 14;
		static const int  BR_MSIE11		= 1;
		static const int  BR_MSIE12		= 5;
		static const int  BR_EDGE		= 31;

		static const int  BR_MSIEMOBILE	= 30;

		static const int  BR_NETSCAPE  = 12;

		static const int  BR_MOZILLA  = 23;
		static const int  BR_FIREFOX  = 24;
	//	static const int  BR_FIREFOX2  = 13;
		static const int  BR_FIREFOX3  = 4;
	//	static const int  BR_FIREFOX4  = 8;
	//	static const int  BR_FIREFOX5  = 10;
	//	static const int  BR_FIREFOX6  = 11;
	//	static const int  BR_FIREFOX7 = 12;
	//	static const int  BR_FIREFOX8  = 13;
	//	static const int  BR_FIREFOX9  = 15;
		
		static const int  BR_YANDEX  = 13;


		static const int  BR_ANDROID = 25;
		static const int  BR_SAFARI	= 26;
		static const int  BR_CHROME	= 9;


		static const int  NOT_PRESENT_BROWSER	= 255;
		static void GetOsBrws(const String &userAgent, TOsID &os, TBrwsID &brws)
		{
			_GetOsBrws(userAgent.c_str(), userAgent.c_str() + userAgent.GetLength(), os, brws);
		}
		static bool IsMobile(const TOsID os)
		{
		    TOsID initOSList[] = { OS_ANDROID, OS_IOS, OS_WINMOBILE, OS_SYMBIAN, OS_J2ME};
		    for (Byte i = 0; i < (sizeof(initOSList) / sizeof(initOSList[0])); ++i) {
			if (initOSList[i] == os)
				return true;
		    }
		    return false;
		}
	protected:
		static bool _IsBot(char *userAgent, int len);
		static void _GetOsBrws(char *userAgent, char *userAgentEnd, TOsID &os, TBrwsID &brws);
		static TOsID _ParseOs(char *os_str, char *userAgentEnd);
		static char *_GetNextParam(char *start);
		static bool _CheckHost(int nameLen, char *beginHeaderName, char *beginHeader, const int len, String &host);
		static bool _CheckXforwarded(int nameLen, char *beginHeaderName, char *beginHeader, char *endHeader, Word32 &xForwardIP, Word32 *lastXForwardIP = NULL);
                static bool _CheckXRealIP(int nameLen, char *beginHeaderName, char *beginHeader, char *endHeader, Word32 &xRealIP);
                static bool _CheckHTTPS(int nameLen, char *beginHeaderName, char *beginHeader, char *endHeader, bool &isHttps);
		static bool _CheckUserAgent(int nameLen, char *beginHeaderName, char *beginHeader, char *endHeader, TOsID &os, TBrwsID &brws);
		static bool _CheckReferer(int nameLen, char *beginHeaderName, char *beginHeader, const int len, String &referer);
		static bool _CheckOrigin(int nameLen, char *beginHeaderName, char *beginHeader, const int len, String &origin);
		static bool _IsCookieHeader(int nameLen, char *beginHeaderName);
		static bool _CheckIfModSince(int nameLen, char *beginHeaderName, char *beginHeader, const int len, Word32 &ifModSince);
	};

	class HttpEvent : public Event
	{
	public:
		HttpEvent(SocketClient *client, class EHttpThread *thread, HttpEventListener *listener);
		virtual ~HttpEvent();
		virtual void Call(int events);

		virtual bool IsEnded(const Word32 curTime);

		virtual void Info(RString &buf, Time &curTime);
		void LU(ELogTypes type, const char *fmt, ...);
		virtual void EndWork();

		const Word32 GetIP() const
		{
			return _client->GetIP();
		}

		HttpEventListener *listener()
		{
			return _listener;
		}

		virtual HttpSocket *getBuffer()
		{
			return _buf;
		}

		void setThread(class EHttpThread* th)
		{
			_thread = th;
		}

		class EHttpThread *GetThread()
		{
			return _thread;
		}
		static Word32 MAX_WAIT_USER_REQ_TIME;
		static Word32 MAX_WAIT_TIME;

		virtual void sendAnswer();
		virtual void SendErrorPage();
		bool setTimeout(const timespec &tm);
		void clearTimeout();
		const int getTimerDescr() const{ return _timeOutDescr; }

		//!!!WARNING!!! Selfdestruct method
		virtual void free();

		static RString &GetErrorPage();
		enum EEventState
		{
			WAIT_QUERY,
			RECEIVE_QUERY,
			SEND_ANSWER,
			FORM_FIRST_ANSWER,
			WAIT_DATA,
			END_WORK,
			DO_HANDSHAKE,
			SHUTDOWN,
			WAIT_ANSWER_TIMER,
			WAIT_ANSWER_RESULT,
			SEND_N_CLOSE,
			MAX_STATE,
		};
		static Word32 MAX_QUERY_CHUNK;
	protected:
		static const char *endChars;
		
		static int MAX_CHUNK_COUNT;
		class EHttpThread *_thread;
		SocketClient *_client;
		HttpEventListener *_listener;
		Word32 _lastOpTime;

		void _UpdateOpTime();
		bool _ReceiveQuery();
		bool _TrySendAnswer();
		bool _ParseHeader(char *beginHeader, char *endHeader);
		bool _ParseURI(char *beginHeader, char *endHeader, bool usingSSL);

		static const char *_STATE_NAMES[MAX_STATE];
		Byte _curState;
		Byte _readChunk;

		const char *_curEndChar;
		int _firstHeaderCharPos;
		int _lastParseCharPos;
		int _timeOutDescr;


		class HttpSocket *_buf;
		static RString _errorPage;
	};

	class HttpPostEvent : public HttpEvent
	{
	public:
		HttpPostEvent(SocketClient *client, class EHttpThread *thread, HttpEventListener *listener)
			: HttpEvent(client, thread, listener), _contentLength(0)
		{

		}
		virtual ~HttpPostEvent() {};
		virtual void Call(int events);
		static int MAX_POST_SIZE;
		SocketClient *ReleaseSocket();
	protected:
		bool _ReceiveQuery();
		bool _ParseHeader(char *beginHeader, char *endHeader);
		bool _CheckContentLength(int nameLen, char *beginHeaderName, char *beginHeader, const int len);
		bool _ParsePOST(const char *buf);
		bool _ReceivePOST();

		int _contentLength;
	};

	class ThreadSpecData
	{
	public:
		virtual void Info(RString &buf)
		{
		};
	};

	class HttpEventFactory 
	{
	public:
		virtual HttpEvent *Create(SocketClient *client, class EHttpThread *thread, SocketClient *server) = 0;
		virtual ThreadSpecData *CreateThreadSpecData() { return NULL; };
		virtual ~HttpEventFactory() {};
	};

};
