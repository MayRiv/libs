#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#ifndef _NO_TIMER
#include <sys/timerfd.h>
#endif

#include "httpevents.hpp"
#include "ehttpthread.hpp"


using namespace lb;


AcceptEvent::AcceptEvent(SocketClient *server, class EHttpThread *thread, class HttpEventFactory* factory)
	: Event(server->GetDescr()), _thread(thread), _server(server), _eventFactory(factory)
{
	server->SetNonBlock();
	static const int MAX_DEFER_TIMEOUT = 60;
	server->SetDeferAccept(MAX_DEFER_TIMEOUT);

	AddEvent(EPOLLIN | EPOLLET);
}

AcceptEvent::~AcceptEvent()
{
	delete _server;
}

void AcceptEvent::Call(int events)
{
	if ((events & EPOLLHUP) == EPOLLHUP)
	{
		L(LOG_ERROR, "AcceptEvent EPoll EPOLLHUP\n");
		//EndWork();
		return;
	}
	if ((events & EPOLLERR) == EPOLLERR)
	{
		L(LOG_ERROR, "AcceptEvent EPoll EPOLLERR\n");
		return;
	}

	SocketClient *client = _server->GetConnection();
	if (client)
	{
		client->SetNonBlock();
		if (!_thread->AddConnection(_eventFactory->Create(client, _thread, _server), _server))
		{
			L(LOG_CRITICAL, "Can't add connection\n");
			delete client;
		}
	}
}


HttpEvent::HttpEvent(SocketClient *client, class EHttpThread *thread, HttpEventListener *listener)
//
//!!!WARNING!!! any HttpEvent(or other that inherits from this event) field should be reset to default in HttpKeepAlivePostEvent::_reset() or other event supported keep alive
//
	: Event(client->GetDescr()), 
	_thread(thread),
	_client(client), 
	_listener(listener),
	_lastOpTime(thread->_curTime.Unix()), 
	_curState(WAIT_QUERY),
	_readChunk(0),
	_curEndChar(endChars), 
	_firstHeaderCharPos(0),
	_lastParseCharPos(0),
	_timeOutDescr(0),
	_buf(NULL)
{
	AddEvent(EPOLLIN | EPOLLERR | EPOLLHUP);
}

HttpEvent::~HttpEvent()
{
	EndWork();
}

void HttpEvent::free()
{
	delete this; //!!! Must be called the last
}

void HttpEvent::sendAnswer()
{
	if (_curState == END_WORK)
		return;
	if (!_TrySendAnswer())
	{
		_curState = SEND_ANSWER;
		ClearEvent(EPOLLIN);
		AddEvent(EPOLLOUT);
		_thread->Ctrl(this);
	}
}


void HttpEvent::clearTimeout()
{
	if (_timeOutDescr)
	{
		close(_timeOutDescr);
		_timeOutDescr = 0;
	}
}

bool HttpEvent::setTimeout(const timespec &tms)
{
#ifndef _NO_TIMER
	if (_timeOutDescr)
		close(_timeOutDescr);
	_timeOutDescr = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK);
	if (_timeOutDescr != -1)
	{
		itimerspec tm;
		tm.it_value = tms;
		tm.it_interval.tv_sec = 0;//ONE SHOT
		tm.it_interval.tv_nsec = 0;
		if (timerfd_settime(_timeOutDescr, 0, &tm, 0))
		{
			L(LOG_ERROR, "HttpEvent: Failed to set timer interval\n");
			return false;
		}
	}
	else 
	{
		_timeOutDescr = 0;
		L(LOG_ERROR, "HttpEvent: Failed to create timer %u\n", errno);
		return false;
	}
	if (_thread->AddTimeoutNL(this))
	{
		_curState = WAIT_ANSWER_TIMER;
		ClearEvent(EPOLLIN | EPOLLOUT);
		return true;
	}
#endif
	return false;
}

void HttpEvent::Call(int events)
{
	if ((events & EPOLLHUP) == EPOLLHUP)
	{
		//LU(LOG_ERROR, "EPoll EPOLLHUP\n");
		EndWork();
		return;
	}
	if ((events & EPOLLERR) == EPOLLERR)
	{
		LU(LOG_ERROR, "EPoll EPOLLERR\n");
		EndWork();
		return;
	}

	if ((events & EPOLLIN) == EPOLLIN) // read user request
	{
		if (_curState == WAIT_ANSWER_TIMER)
		{
			if (_timeOutDescr)
			{
				close(_timeOutDescr);
				_timeOutDescr = 0;
				if (!_listener->callTimer(this))
				{
					SendErrorPage();
					return;
				}
			}
		}
		else if (_curState == WAIT_QUERY)
		{
			_UpdateOpTime();
			if (!_ReceiveQuery())
			{
				SendErrorPage();
				return;
			}
			if (_curState == RECEIVE_QUERY)
			{
				HttpEventListener::EFormAnswer res = _listener->FormAnswer(*_buf, this);
				if (res == HttpEventListener::ANS_ERROR)
				{
					SendErrorPage();
					return;
				}

				if (res == HttpEventListener::ANS_WAIT_RESULT)
				{
					ClearEvent(EPOLLIN | EPOLLOUT);
					_thread->Ctrl(this);
					_curState = WAIT_ANSWER_RESULT;
				}
				else if (res != HttpEventListener::ANS_WAIT_TIMER)
				{
					sendAnswer();
					return;
				}

			}
			else
			{
				AddEvent(EPOLLIN);
				_thread->Ctrl(this);
			}
			return;
		}
	}

	if ((events & EPOLLOUT) == EPOLLOUT)
	{
		sendAnswer();
	}
}

bool HttpEvent::_TrySendAnswer()
{
	HttpSocket::ESocketResult res = _buf->Send(_descr);
	if (res == HttpSocket::OK)
	{
		EndWork();
		return true;
	}
	else if (res != HttpSocket::PROGRESS)
	{
		LU(LOG_WARNING, "[UE] Send data error\n");
		EndWork();
		return true;
	}
	return false;
}


const char *HttpEvent::endChars = "\r\n\r\n";
Word32 HttpEvent::MAX_QUERY_CHUNK = 10000;
int HttpEvent::MAX_CHUNK_COUNT = 20;

bool HttpEvent::_ReceiveQuery()
{
	if (!_buf)
		_buf = HttpSocket::Get(MAX_QUERY_CHUNK);

	HttpSocket::ESocketResult res = _buf->AddRead(_descr);
	if ((res == HttpSocket::ERROR) || (res == HttpSocket::CONNECTION_CLOSE))
		return false;

	_readChunk++;
	if (_readChunk > MAX_CHUNK_COUNT)
	{
		LU(LOG_WARNING, "Too many chunks %u\n", _readChunk);
		return false;
	}

	if (_buf->Length() > 1)
	{
		char *beginQueryBuf = *_buf;
		if (!_lastParseCharPos)
		{
			if ((beginQueryBuf[0] != 'G') && (beginQueryBuf[0] != 'P') && (beginQueryBuf[0] != 'H'))
			{
				LU(LOG_WARNING, "Bad red\n");
				return false;
			}
		}
		beginQueryBuf += _lastParseCharPos;

		
		for ( ; _lastParseCharPos < _buf->Length(); _lastParseCharPos++)
		{
			if ((*beginQueryBuf == *_curEndChar))
			{
				if (*_curEndChar == '\n')
				{
					char *endHeader = beginQueryBuf - 1; // - \r\n
					if (!_firstHeaderCharPos) // query
					{
						if (!_ParseURI(*_buf, endHeader, false))
							return false;
					}
					else if (!_ParseHeader(*_buf + _firstHeaderCharPos, endHeader))
						return false;
					_firstHeaderCharPos = _lastParseCharPos + 1;
				}
				_curEndChar++;
				if (!*_curEndChar) // \r\n\r\n
					break;
			}
			else
				_curEndChar = endChars;
			beginQueryBuf++;
		}
		if (!*_curEndChar) // find end query
			_curState = RECEIVE_QUERY;
	}
	return true;
}


bool HttpEvent::_ParseURI(char *beginHeader, char *endHeader, bool usingSSL)
{
	int len = endHeader - beginHeader;
	static int MIN_URI = 10;
	if (len < MIN_URI)
	{
		LU(LOG_ERROR, "Query < MIN %d\n", len);
		return false;
	}
	EHttpReqType reqType;
	if (*beginHeader == 'G')
	{
		beginHeader += sizeof("GET");
		if (usingSSL)
			reqType = T_HTTPS_GET;
		else
			reqType = T_HTTP_GET;
	}
	else if (*beginHeader == 'P')
	{
		beginHeader += sizeof("POST");
		if (usingSSL)
			reqType = T_HTTP_POST;//T_HTTPS_POST;
		else
			reqType = T_HTTP_POST;
	}
	else if (*beginHeader == 'H')
	{
		beginHeader += sizeof("HEAD");
		reqType = T_HTTP_HEAD;
	}
	else
	{
		LU(LOG_ERROR, "Unknown request [%s]\n", beginHeader);
		return false;
	}

	if (*(beginHeader - 1) != ' ')
	{
		LU(LOG_ERROR, "Can't find begin query %u (%c)\n", reqType, *(beginHeader-1));
		return false;
	}
	while (endHeader > beginHeader)
	{
		if (*endHeader == ' ')
			break;
		endHeader--;
	}

	String host;
	String req;
	String query;

	static const String HTTP_STR("http://");
	static const String HTTPS_STR("https://");
	size_t skipChars = 0;
	if (!strncasecmp(beginHeader, HTTP_STR.Data(), HTTP_STR.GetLength()))
		skipChars = HTTP_STR.GetLength();
	else if (!strncasecmp(beginHeader, HTTPS_STR.Data(), HTTPS_STR.GetLength()))
		skipChars = HTTPS_STR.GetLength();
	if (skipChars > 0)
	{
		beginHeader += skipChars;
		char *beginHost = beginHeader;
		while (beginHeader < endHeader)
		{
			if ((*beginHeader == '/') || (*beginHeader == ':'))
			{
				int len = beginHeader - beginHost;
				if (len > 0)
					host.Set(beginHost, len);
				if (*beginHeader == ':') // skip port
				{
					while (beginHeader < endHeader)
					{
						if (*beginHeader == '/')
							break;
						beginHeader++;
					}
				}
				break;
			}
			beginHeader++;
		}
	}
	char *beginQuery = beginHeader;
	while (beginQuery < endHeader)
	{
		if (*beginQuery == '?')
			break;
		beginQuery++;
	}

	len = endHeader - beginQuery;
	if (len > 1)
		query.Set(beginQuery + 1, len - 1);

	len = beginQuery - beginHeader;
	if (len > 0)
		req.Set(beginHeader, len);

	//LU(LOG_WARNING, "Recive req[%s] query [%s]\n", req.c_str(), query.c_str());
	return _listener->ParseQuery(reqType, host, req, query);
}

bool HttpEvent::_ParseHeader(char *beginHeader, char *endHeader)
{
	char *beginHeaderName = beginHeader;
	while (beginHeader < endHeader)
	{
		if (*beginHeader == ':')
		{
			int nameLen = beginHeader - beginHeaderName;
			beginHeader++;
			while (isspace(*beginHeader) && (beginHeader < endHeader))
				beginHeader++;
			int valLen = endHeader - beginHeader;
			if (valLen <= 0)
				return true;
			return _listener->ParseHeader(nameLen, beginHeaderName, beginHeader, endHeader, valLen);
		}
		beginHeader++;
	}
	return true;
}

bool HttpEventListener::_IsCookieHeader(int nameLen, char *beginHeaderName)
{
	static const String COOKIE_HEADER_NAME("Cookie");
	if ((nameLen == COOKIE_HEADER_NAME.GetLength()) && !strncasecmp(beginHeaderName, COOKIE_HEADER_NAME.c_str(), COOKIE_HEADER_NAME.GetLength()))
	{
		return true;
	}
	return false;
}
bool HttpEventListener::_CheckReferer(int nameLen, char *beginHeaderName, char *beginHeader, const int len, String &referer)
{
	static const String REFERER_HEADER_NAME("Referer");
	if ((nameLen == REFERER_HEADER_NAME.GetLength()) && !strncasecmp(beginHeaderName, REFERER_HEADER_NAME.c_str(), REFERER_HEADER_NAME.GetLength()))
	{
		if (len > 11)
		{
			if (!memcmp(beginHeader, "http://", 7))
			{
				referer.SetUnescapeChars(beginHeader + 7, len - 7);
			}else if (!memcmp(beginHeader, "https://", 8))
			{
				referer.SetUnescapeChars(beginHeader + 8, len - 8);
			}
		}
		return true;
	}
	return false;
}

bool HttpEventListener::_CheckOrigin(int nameLen, char *beginHeaderName, char *beginHeader, const int len, String &origin)
{
	static const String ORIGIN_HEADER_NAME("Origin");
	if ((nameLen == ORIGIN_HEADER_NAME.GetLength()) && !strncasecmp(beginHeaderName, ORIGIN_HEADER_NAME.c_str(), ORIGIN_HEADER_NAME.GetLength()))
	{
		if (len > 11)
		{
			if (!memcmp(beginHeader, "http://", 7))
			{
				origin.SetUnescapeChars(beginHeader + 7, len - 7);
			}else if (!memcmp(beginHeader, "https://", 8))
			{
				origin.SetUnescapeChars(beginHeader + 8, len - 8);
			}
		}
		return true;
	}
	return false;
}

bool HttpEventListener::_CheckUserAgent(int nameLen, char *beginHeaderName, char *beginHeader, char *endHeader, TOsID &os, TBrwsID &brws)
{
	static const String USER_AGENT_HEADER_NAME("USER-AGENT");
	if ((nameLen == USER_AGENT_HEADER_NAME.GetLength()) && !strncasecmp(beginHeaderName, USER_AGENT_HEADER_NAME.c_str(), USER_AGENT_HEADER_NAME.GetLength()))
	{
		_GetOsBrws(beginHeader, endHeader, os, brws);
		return true;
	}
	return false;
}

bool HttpEventListener::_CheckXforwarded(int nameLen, char *beginHeaderName, char *beginHeader, char *endHeader, Word32 &xForwardIP, Word32 *lastXForwardIP)
{
	static const String XFORWARDED_HEADER_NAME("X-Forwarded-For");
	if ((nameLen == XFORWARDED_HEADER_NAME.GetLength()) && !strncasecmp(beginHeaderName, XFORWARDED_HEADER_NAME.c_str(), XFORWARDED_HEADER_NAME.GetLength()))
	{
		char *nextFwrd = beginHeader;
		while (nextFwrd < endHeader)
		{
			if (*nextFwrd == ',')
				break;
			nextFwrd++;
		}
				
		int len = nextFwrd - beginHeader;
		if (len > 0)
		{
			String ip(beginHeader, len);
			if ((xForwardIP = inet_network(ip.c_str())) == INADDR_NONE)
				xForwardIP = 0;
		}
		if (lastXForwardIP) // need check last
		{
			if (*nextFwrd == ',')
			{
				char *endIP = endHeader;
				while (endIP >= beginHeader)
				{
					if (isspace(*endIP))
						endIP--;
					else
						break;
				}

				char *nextFwrd = endIP;
				while (nextFwrd >= beginHeader)
				{
					if (*nextFwrd == ',' || isspace(*nextFwrd))
					{
						nextFwrd++;
						break;
					}
					nextFwrd--;
				}

				int len = endIP - nextFwrd + 1;
				if (len > 0)
				{
					String ip(nextFwrd, len);
					if ((*lastXForwardIP = SocketClient::IpToLong(ip.c_str())) == INADDR_NONE)
							*lastXForwardIP = 0;
				}
			}
			else // get first ForwardIp;
			{
				*lastXForwardIP = xForwardIP;
			}
		}
		return true;
	}
	return false;
}
bool HttpEventListener::_CheckHTTPS(int nameLen, char *beginHeaderName, char *beginHeader, char *endHeader, bool &isHttps)
{
    static const String HTTPS_HEADER_NAME("X-Forwarded-Proto");
    isHttps = false;
    if ((nameLen == HTTPS_HEADER_NAME.GetLength()) && !strncasecmp(beginHeaderName, HTTPS_HEADER_NAME.c_str(), HTTPS_HEADER_NAME.GetLength()))
    {		
        int len = endHeader - beginHeader;
        if (len > 0)
        {
            
            String scheme(beginHeader, len);
            if (!strncasecmp(scheme.c_str(), "https", scheme.GetLength()))
                isHttps = true;
        }
        return true;
    }
    return false;
}
bool HttpEventListener::_CheckXRealIP(int nameLen, char *beginHeaderName, char *beginHeader, char *endHeader, Word32 &xRealIP)
{
	static const String XREALIP_HEADER_NAME("X-Real-IP");
	if ((nameLen == XREALIP_HEADER_NAME.GetLength()) && !strncasecmp(beginHeaderName, XREALIP_HEADER_NAME.c_str(), XREALIP_HEADER_NAME.GetLength()))
	{		
		int len = endHeader - beginHeader;
		if (len > 0)
		{
			String ip(beginHeader, len);
			if ((xRealIP = inet_network(ip.c_str())) == INADDR_NONE)
				xRealIP = 0;
		}
		return true;
	}
	return false;
}
bool HttpEventListener::_CheckHost(int nameLen, char *beginHeaderName, char *beginHeader, const int len, String &host)
{
	static const String HOST_HEADER_NAME("Host");
	if ((nameLen == HOST_HEADER_NAME.GetLength()) && !strncasecmp(beginHeaderName, HOST_HEADER_NAME.c_str(), HOST_HEADER_NAME.GetLength()))
	{
		host.Set(beginHeader, len);
		return true;
	}
	return false;
}

bool HttpEventListener::_CheckIfModSince(int nameLen, char *beginHeaderName, char *beginHeader, const int len, Word32 &ifModSince)
{
	static const String IF_MODIFIED_SINE_HEADER_NAME("If-Modified-Since");
	if ((nameLen == IF_MODIFIED_SINE_HEADER_NAME.GetLength()) && !strncasecmp(beginHeaderName, IF_MODIFIED_SINE_HEADER_NAME.c_str(), IF_MODIFIED_SINE_HEADER_NAME.GetLength()))
	{
		ifModSince =  Time::HTTPStr2Unix(beginHeader, len);
		return true;
	}
	return false;
}

RString HttpEvent::_errorPage;
Word32 HttpEvent::MAX_WAIT_USER_REQ_TIME = 60;
Word32 HttpEvent::MAX_WAIT_TIME					 = 90;


void HttpEvent::SendErrorPage()
{
	_buf->Null();
	_buf->Add(_errorPage.Data(), _errorPage.Length());
	_buf->AddEndZero();
	sendAnswer();
}


void HttpEvent::_UpdateOpTime()
{
	_lastOpTime = _thread->_curTime.Unix();
}

void HttpEvent::EndWork()
{
	_curState = END_WORK;
	if (_client)
	{
		delete _client;
		_client = NULL;
		if (_timeOutDescr)
		{
			close(_timeOutDescr);
			_timeOutDescr = 0;
		}
		if (_buf)
		{
			HttpSocket::Free(_buf);
			_buf = NULL;
		}
		delete _listener;
	}
}

bool HttpEvent::IsEnded(const Word32 curTime)
{
	if (_curState == END_WORK)
		return true;
	if (_curState == WAIT_QUERY)
	{
		if (_lastOpTime < (curTime - MAX_WAIT_USER_REQ_TIME))
		{
			//LU(LOG_WARNING, "[UE] Timeout first req\n");
			_listener->WaitQueryTimeOut();
			return true;
		}
	}
	else if (_lastOpTime < (curTime - MAX_WAIT_TIME))
	{
		LU(LOG_WARNING, "[UE] Timeout wait\n");
		_listener->TimeOut();
		return true;
	}
	return false;
}

const char *HttpEvent::_STATE_NAMES[MAX_STATE] = 
{
	"WAIT_QUERY",
	"RECEIVE_QUERY",
	"SEND_ANSWER",
	"FORM_FIRST_ANSWER",
	"WAIT_DATA",
	"END_WORK",
	"DO_HANDSHAKE",
	"SHUTDOWN",
	"WAIT_ANSWER_TIMER",
	"WAIT_ANSWER_RESULT"
};

void HttpEvent::Info(RString &buf, Time &curTime)
{
	if (_client)
	{
		_client->GetIPStr(buf);
	}
	else
	{
		//buf.snaprintf("Ended");
		return;
	}
	buf.snaprintf("] %s %u (%u)", _STATE_NAMES[_curState], _descr, curTime.Unix() - _lastOpTime);
	buf.snaprintf("\n<br>");
}

RString &HttpEvent::GetErrorPage()
{
	return _errorPage;
}

void HttpEvent::LU(ELogTypes type, const char *fmt, ...)
{
		va_list args;
		va_list argsForScreen;
		va_start(args, fmt);
		va_start(argsForScreen, fmt);
		RString buf;
    buf.snprintf("[");
		if (_client)
			_client->GetIPStr(buf);
    buf.snaprintf("] %s", fmt);
    Log::DefaultLog()->Write(type, buf.Data(), args, argsForScreen);
    va_end(args);
		va_end(argsForScreen);
}

void HttpEventListener::_GetOsBrws(char *userAgent, char *userAgentEnd, TOsID &os, TBrwsID &brws)
{
	int len = userAgentEnd - userAgent;
	if (_IsBot(userAgent, len))
		return;

	static const Byte opera[10]=     {0,20, 19,19,19,19,19,19, 27, 28};
	static const Byte msie[10]=	     {BR_MSIE, BR_MSIE10, BR_MSIE, BR_MSIE, BR_MSIE, BR_MSIE, BR_MSIE, BR_MSIE7, BR_MSIE8, BR_MSIE9};
	static const Byte netscape[10] = {0,0,12,12,12,12,12,12, 12, 0};
	register char *p = userAgent, *s;
	Byte version = 0;
	char ch;

	if (len > 18) //magic length to ensure that strstr are safe
	{
		if (!memcmp("Mozilla", userAgent, 7))
		{
			ch = userAgent[8];
			if (ch >= '0' && ch <= '9')
			{
				version = ch - '0';
				brws = netscape[version];
			}
			p += 9;
			//printf("M %.4s V%c ",p,ch);
			if ((s = (char*)memchr(p, '(', userAgentEnd - p)) != NULL) 
			{
				s++;

				//MSIE || Netscape && OS
				if ((p = strnstr(s, "MSIE", userAgentEnd - s)) != NULL) 
				{
//				printf("Msie %.4s V%d ",p,version);
					if ((p + 7) < userAgentEnd)
					{
						p += 5;
						os = _ParseOs(p + 2, userAgentEnd);
						ch = p[0];
						if (ch>='0' && ch<='9')
						{
							version = ch - '0';
							if ((version == 4 || version == 6) && strnstr(p + 2, "Windows CE", userAgentEnd - p - 2) != NULL)
							{
								brws = BR_MSIEMOBILE;
							}
							else if (version == 1)
							{
								if (p[1] == '1')
								{
									brws = BR_MSIE11;
								}
								else
									brws = BR_MSIE10;
							}
							else
								brws = msie[version];
						}
					}

					if ((p = strnstr(s, "Opera", userAgentEnd - s)) != NULL) 
					{
						//search after MSIE string
						brws = BR_OPERA;
						ch = p[6];
						if((ch >= '0') && (ch <= '9')) 
						{
							version = ch - '0';
							brws = opera[version];
							//printf("\nOpr %.4s V%c ",p,ch);
						}
					}
				}
				else if ( (p = strnstr(s, "Edge/", userAgentEnd - s)) != NULL) 
				{
					brws = BR_EDGE;
				}
				else if ( (p = strnstr(s, "Trident/", userAgentEnd - s)) != NULL) 
				{
					if ((p = strnstr(p, "rv:", userAgentEnd - p)) != NULL) 
					{
						//search after MSIE string
						brws = BR_MSIE11;
						if (p[3] == '1')
						{
							ch = p[4];
							if (ch == '1')
								brws = BR_MSIE11;
							if (ch == '2')
								brws = BR_MSIE12;
						}
					}
				}
				else if ((p = strnstr(s, "YaBrowser", userAgentEnd - s)) != NULL) 
				//Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/33.0.1750.154 YaBrowser/14.4.1750.13414 Safari/537.36
				{
					brws = BR_YANDEX;
				}
				//User-Agent: Mozilla/5.0 (Windows; U; Windows NT 5.0; en-US; rv:1.6) Gecko/20040206 Firefox/0.8
				else if ( (p = strnstr(s, "Gecko/", userAgentEnd - s)) != NULL) 
				{
					char *pGecko = p + 7;
					if (pGecko < userAgentEnd)
					{
						brws = BR_MOZILLA;
						if ((p = strnstr(pGecko, "Firefox", userAgentEnd - pGecko)) != NULL)
						{
							brws = BR_FIREFOX;
							ch = p[8];
							if (((ch == '3') || (ch == '2') || (ch == '1') || (ch == '0') )  
								&& (p[9]=='.') )
								
							{
								brws = BR_FIREFOX3;
							}
						}
						else if ((p = strnstr(pGecko, "Netscape", userAgentEnd - pGecko)) != NULL)
						{	//Gecko/20020823 Netscape/7.0
							brws = BR_NETSCAPE;
						}
					}
				}
				else if ((p = strnstr(s, "OPR", userAgentEnd - s)) != NULL) 
				{
					// Opera NEXT
					brws = BR_OPERA15;
				}
				else if ((p = strnstr(s, "Chrome", userAgentEnd - s)) != NULL)//check before safari because may contain string Safari at the end.
				{
					brws = BR_CHROME;
				}
				else if ((p = strnstr(s, "Safari", userAgentEnd - s)) != NULL) 
				{//Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US) AppleWebKit/525.19 (KHTML, like Gecko) Chrome/1.0.154.36 Safari/525.19
					brws = BR_SAFARI;
				}
				if (!os) {
//					printf("%s\n",s);
					os = _ParseOs(s, userAgentEnd);
				}
			}
		}
		else if (!memcmp("Opera",userAgent, 5)) 
		{
			// Opera Mobile Opera Mini Opera/9 
			brws = BR_OPERA;
			ch = userAgent[6];
			if ((ch >= '0') && (ch <= '9'))
			{
				version = ch - '0';
				brws = opera[version];
				if (version == 9)//trick to parse 10.00 and higher
				{
					char *pVersion = userAgent + 8;
					if ((p = strnstr(pVersion, "Version/", userAgentEnd - pVersion)) != NULL)
					{
						//printf("Version/   %s\n", p+9);
						if (p[8] == '1')
						{
							if (p[9] == '0')
								brws = BR_OPERA10;
							else if (p[9] == '1')
								brws = BR_OPERA11;
							else if (p[9] == '2')
								brws = BR_OPERA12;
						}
					}
				}
			} 
			else if ( ch == 'M' )
			{
				brws = BR_OPERAMOBILE;
			}
			os = _ParseOs(userAgent + 8, userAgentEnd);
			//printf("\nO %.4s V%c ",p,ch);
		}
		else if (!memcmp("Dalvik",userAgent, 6)) 
		{
			brws = BR_ANDROID;
			os = OS_ANDROID;
		}
	}//len smaller than 18
	
	if(!os)
		os = _ParseOs(userAgent, userAgentEnd);
}

HttpEventListener::TOsID HttpEventListener::_ParseOs(char *os_str, char *userAgentEnd)
{
	register char * t;
	//printf("\nPARSE_OS %.10s\n", os_str);
	int len = userAgentEnd - os_str;
	if (len < 3)
		return 0;

	if ((t = strnstr(os_str, "Win", len)) != NULL)
	{
		t += 3;
		//printf("%s\n", t);
		if(memcmp(t, "dows", 4)==0)
			t += 4;
		if(*t== ' ' || *t== '_')
		{
			t++;
		}
		else if (*t==';')	
		{//Windows; U; Windows NT 5.0
			register char * p;
			if ((p = strnstr(t, "Win", userAgentEnd - t))!=NULL) {
				t = p + 3;
				if (memcmp(t,"dows",4)==0)
					t += 4;
				if (*t==' '||*t=='_')
					t++;
			}
		}
		if (t[0]=='X'&&t[1]=='P')
		{
			return OS_WINXP;
		}
		if ( (t[0]=='N' && t[1]=='T')) 
		{
			if (t[2]==' ')
			{
				if (t[3]=='5'){
					if( t[4]=='.') {
						if (t[5]=='1')
							return OS_WINXP;
						if (t[5]=='0')
							return OS_WIN2000;
						if (t[5]=='2')
							return OS_WIN2003;
					}
				}
				if (t[3]=='6')
				{
					if (t[5]=='1')
					{
						return OS_WIN7;
					}
					if (t[5]=='2' || t[5]=='3')
					{
						return OS_WIN8;
					}
					return OS_WINVISTA;
				}
				if (t[3]=='4')
					return OS_WINNT;
				if (t[3]=='1' && t[4]=='0')
					return OS_WIN10;
			}
			return OS_WINNT;
		}
		if (t[0]=='2' && t[1]=='0')
		{
			return OS_WIN2000;
		}
		
		if ((t[0]=='M' && t[1]=='o') || (t[0] == 'P' && t[1] == 'h'))//windows mobile & windows phone
		{
			return OS_WINMOBILE;
		}
		
		if (t[0]=='9')
		{
			if (t[1]=='8')
			{
				if (t[2]==';' && (strncmp(t+3," Win 9x 4.90", 12)==0))
					return OS_WINME;
				else
					return OS_WIN98;
			}
			if (t[1]=='x'){
				return OS_WINME;
			}
			if (t[1]=='5')
				return OS_WIN95;
		}

		if (t[0]=='M' && t[1]=='E')
		{
			return OS_WINME;
		}

		if (t[0]=='C' && t[1]=='E')
		{
			return OS_WINMOBILE;
		}

		if (t[0]=='3')
		{
			if (t[1]=='.')
				return OS_WIN3X;
			else if (t[1]=='2')
				return OS_WIN95;
		}
	}

	if (strnstr(os_str, "Android", len))
	{
		return OS_ANDROID;
	}

	if (strnstr(os_str, "Linux", len))
	{
		return OS_LINUX;
	}
//	if (strstr(os_str,"FreeBSD"))
//	{
//		return OS_FBSD;
//	}

	if (strnstr(os_str, "iPhone", len) || strnstr(os_str, "iPad", len))
	{
		return OS_IOS;
	}

	if ((t = strnstr(os_str, "Mac", len)) != NULL)
	{
		t+=3;
		if(*t=='i' || *t=='_')
			return OS_MAC;
	}

	if (strnstr(os_str, "Symbian", len))
	{
		return OS_SYMBIAN;
	}

	if (strnstr(os_str, "/MIDP", len))
	{
		return OS_J2ME;
	}
	
//	if (strstr(os_str,"Sun"))
//	{
//		return OS_SUN;
//	}

	return 0;
}

bool HttpEventListener::_IsBot(char *userAgent, int len)
{
	if (strncasestr(userAgent, "Googlebot", len))
		return true;
	if (strncasestr(userAgent, "Yandex/", len))
		return true;
	if (strncasestr(userAgent, "StackRambler", len))
		return true;
	if (strncasestr(userAgent, "Slurp", len))
		return true;
	if (strncasestr(userAgent, "msnbot", len) || strncasestr(userAgent, "bingbot", len))
		return true;

	return false;
}

char *HttpEventListener::_GetNextParam(char *start)
{
	char *pquery = start;
	while (*pquery)
	{
		char next = *(pquery + 1);
		if ((next == '&') || !next)
			return (pquery + 1);
		else
			pquery++;
	}
	return NULL;
}

int HttpPostEvent::MAX_POST_SIZE = 1024 * 1024;

SocketClient *HttpPostEvent::ReleaseSocket()
{
	if (_client)
	{
		if (_buf)
			HttpSocket::Free(_buf);
		_buf = NULL;
		_op = EPOLL_CTL_DEL;
		if (!_thread->Ctrl(this))
		{
			L(LOG_ERROR, "Can't delete for ReleaseSocket\n");
		}
		SocketClient *client = _client;
		_client = NULL;
		_curState = END_WORK;
		return client;
	}
	return NULL;
}

void HttpPostEvent::Call(int events)
{
	if ((events & EPOLLHUP) == EPOLLHUP)
	{
		//LU(LOG_ERROR, "EPoll EPOLLHUP\n");
		EndWork();
		return;
	}
	if ((events & EPOLLERR) == EPOLLERR)
	{
		LU(LOG_ERROR, "EPoll EPOLLERR\n");
		EndWork();
		return;
	}

	if ((events & EPOLLIN) == EPOLLIN) // read user request
	{
		if (_curState == WAIT_ANSWER_TIMER)
		{
			close(_timeOutDescr);
			_timeOutDescr = 0;
			if (!_listener->callTimer(this))
			{
				SendErrorPage();
				return;
			}
		}
		else if ((_curState == WAIT_QUERY) || (_curState == WAIT_DATA))
		{
			_UpdateOpTime();
			if (_curState == WAIT_QUERY)
			{
				if (!_ReceiveQuery())
				{
					SendErrorPage();
					return;
				}
			}
			else if (_curState == WAIT_DATA)
			{
				if (!_ReceivePOST())
				{
					SendErrorPage();
					return;
				}
			}

			if (_curState == RECEIVE_QUERY)
			{
				HttpEventListener::EFormAnswer res =_listener->FormAnswer(*_buf, this);
				if (res == HttpEventListener::ANS_ERROR)
				{
					SendErrorPage();
					return;
				}
				if (_client)
				{
					if (res == HttpEventListener::ANS_SEND_N_CLOSE)
					{
						_curState = SEND_N_CLOSE;
						sendAnswer();
					}
					else if (res == HttpEventListener::ANS_WAIT_RESULT)
					{
						ClearEvent(EPOLLIN | EPOLLOUT);
						_thread->Ctrl(this);
						_curState = WAIT_ANSWER_RESULT;
					}
					else if (res != HttpEventListener::ANS_WAIT_TIMER)
					{
						sendAnswer();
					}
				}
				else // socket released
				{
					delete _listener;
					EndWork();
				}
			}
			else
			{
				AddEvent(EPOLLIN);
				_thread->Ctrl(this);
			}
			return;
		}
	}
	if ((events & EPOLLOUT) == EPOLLOUT)
	{
		sendAnswer();
	}
}

bool HttpPostEvent::_ReceiveQuery()
{
	if (!_buf)
		_buf = HttpSocket::Get(MAX_QUERY_CHUNK);

	HttpSocket::ESocketResult res = _buf->AddRead(_descr);
	if ((res == HttpSocket::ERROR) || (res == HttpSocket::CONNECTION_CLOSE))
		return false;

	_readChunk++;
	if (_readChunk > MAX_CHUNK_COUNT)
	{
		LU(LOG_WARNING, "Too many chunks %u\n", _readChunk);
		return false;
	}

	if (_buf->Length() > 1)
	{
		char *beginQueryBuf = *_buf;
		if (!_lastParseCharPos)
		{
			if ((beginQueryBuf[0] != 'G') && (beginQueryBuf[0] != 'P') && (beginQueryBuf[0] != 'H'))
			{
				LU(LOG_WARNING, "Bad red\n");
				return false;
			}
		}
		beginQueryBuf += _lastParseCharPos;

		
		for ( ; _lastParseCharPos < _buf->Length(); _lastParseCharPos++)
		{
			if ((*beginQueryBuf == *_curEndChar))
			{
				if (*_curEndChar == '\n')
				{
					char *endHeader = beginQueryBuf - 1; // - \r\n
					if (!_firstHeaderCharPos) // query
					{
						if (!_ParseURI(*_buf, endHeader, false))
							return false;
					}
					else if (!_ParseHeader(*_buf + _firstHeaderCharPos, endHeader))
						return false;
					_firstHeaderCharPos = _lastParseCharPos + 1;
				}
				_curEndChar++;
				if (!*_curEndChar) // \r\n\r\n
					break;
			}
			else
				_curEndChar = endChars;
			beginQueryBuf++;
		}
		if (!*_curEndChar) // find end query
		{
			_curState = RECEIVE_QUERY;
			if (_contentLength > 0)
			{
				_lastParseCharPos++;
				if ((_lastParseCharPos  + _contentLength) <=_buf->Length())
				{
					if (!_ParsePOST(_buf->Data() + _lastParseCharPos))
						return false;
				}
				else // post not ready now
				{
					//L(LOG_WARNING, "Need more post data %d\n", (_lastParseCharPos  + _contentLength) - _buf->Length());
					_curState = WAIT_DATA;
				}
			}
		}
	}
	return true;
}


bool HttpPostEvent::_ReceivePOST()
{
	HttpSocket::ESocketResult res = _buf->AddRead(_descr);
	if ((res == HttpSocket::ERROR) || (res == HttpSocket::CONNECTION_CLOSE))
		return false;
	if ((_lastParseCharPos  + _contentLength) <=_buf->Length())
	{
		if (_ParsePOST(_buf->Data() + _lastParseCharPos))
			_curState = RECEIVE_QUERY;
		else
			return false;
	}
	return true;
}

bool HttpPostEvent::_ParsePOST(const char *buf)
{
	//L(LOG_WARNING, "Recive post req [%s/%u]\n", postQuery.c_str(), postQuery.GetLength());
	return _listener->ParsePostQuery(buf, _contentLength);
}

bool HttpPostEvent::_ParseHeader(char *beginHeader, char *endHeader)
{
	char *beginHeaderName = beginHeader;
	while (beginHeader < endHeader)
	{
		if (*beginHeader == ':')
		{
			int nameLen = beginHeader - beginHeaderName;
			beginHeader++;
			while (isspace(*beginHeader) && (beginHeader < endHeader))
				beginHeader++;
			int valLen = endHeader - beginHeader;
			if (valLen <= 0)
				return true;
			if (_CheckContentLength(nameLen, beginHeaderName, beginHeader, valLen))
			{
				//L(LOG_WARNING, "Recive _contentLength=%u\n", _contentLength);
				if (_contentLength > MAX_POST_SIZE)
				{
					L(LOG_ERROR, "Content length(%u) > MAX_POST_SIZE(%u)\n", _contentLength, MAX_POST_SIZE);
					return false;
				}
			}
			return _listener->ParseHeader(nameLen, beginHeaderName, beginHeader, endHeader, valLen);
		}
		beginHeader++;
	}
	return true;
}

bool HttpPostEvent::_CheckContentLength(int nameLen, char *beginHeaderName, char *beginHeader, const int len)
{
	static const String CONTENT_LENGTH_HEADER_NAME("Content-Length");
	if ((nameLen == CONTENT_LENGTH_HEADER_NAME.GetLength()) && !strncasecmp(beginHeaderName, CONTENT_LENGTH_HEADER_NAME.c_str(), CONTENT_LENGTH_HEADER_NAME.GetLength()))
	{
		_contentLength =  atoi(beginHeader);
		return true;
	}
	return false;
}
