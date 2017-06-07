#include <unistd.h>
#include "httpkeepalive.hpp"
#include "ehttpthread.hpp"

using namespace lb;



const char* const HttpKeepAlivePostEvent::CONNECTION_HEADER_KEEPALIVE = "Connection: Keep-Alive";
const char* const HttpKeepAlivePostEvent::CONNECTION_HEADER_CLOSE = "Connection: Close";


RString HttpKeepAlivePostEvent::_errorPageClose;
RString HttpKeepAlivePostEvent::_errorPageKeep;

ErrorSetter::ErrorSetter()
{
	HttpKeepAlivePostEvent::_errorPageClose.snprintf("HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\nConnection: Close\r\n\r\n");
	HttpKeepAlivePostEvent::_errorPageKeep.snprintf("HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n");
}

static ErrorSetter setErrorPages;

void HttpKeepAlivePostEvent::Info(RString &buf, Time &curTime)
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
	buf.snaprintf("] %s %u (%u; cr: %u; isK: %u))", _STATE_NAMES[_curState], _descr, curTime.Unix() - _lastOpTime, _countRequests, _isKeepAlive);
	buf.snaprintf("\n<br>");
}

void HttpKeepAlivePostEvent::_reset()
{
	if (_curState == END_WORK)
		return;
	if (!_isKeepAlive || _curState == SEND_N_CLOSE)
	{
		EndWork();
		return;
	}
	if (_timeOutDescr != 0)
	{
		close(_timeOutDescr);
		_timeOutDescr = 0;
	}
	_countRequests++;
	_listener->reset();
	_lastOpTime = _thread->_curTime.Unix(); 
	_curState = WAIT_QUERY;
	_readChunk = 0;
	_curEndChar = endChars; 
	_firstHeaderCharPos = 0;
	_lastParseCharPos = 0;
	_isKeepAlive = false;
	if (_buf)
		_buf->Null();
	ClearEvent(EPOLLOUT);
	AddEvent(EPOLLIN);
	if (!_thread->Ctrl(this))
		EndWork();
}

HttpKeepAlivePostEvent::HttpKeepAlivePostEvent(SocketClient *client, class EHttpThread *thread, HttpEventListener *listener):
	HttpPostEvent(client, thread, listener), _isKeepAlive(false), _countRequests(1)
{
}


bool HttpKeepAlivePostEvent::_ParseHeader(char *beginHeader, char *endHeader)
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
			else
			{
				static const String CONNECTION("Connection");
				static const String KEEPALIVE("Keep-Alive");
				static const String CLOSE("Close");
				if (strncasecmp(beginHeaderName, CONNECTION.c_str(), CONNECTION.GetLength()) == 0)
				{
					if (strncasecmp(beginHeader, KEEPALIVE.c_str(), KEEPALIVE.GetLength()) == 0)
					{
						_isKeepAlive = true;
					}
					else if (strncasecmp(beginHeader, CLOSE.c_str(), CLOSE.GetLength()) == 0)
					{
						_isKeepAlive = false;
					}
				}
			}
			return _listener->ParseHeader(nameLen, beginHeaderName, beginHeader, endHeader, valLen);
		}
		beginHeader++;
	}
	return true;
}

bool HttpKeepAlivePostEvent::_TrySendAnswer()
{
	HttpSocket::ESocketResult res = _buf->Send(_descr);
	if (res == HttpSocket::OK)
	{
		_reset();
		return true;
	}
	else if (res != HttpSocket::PROGRESS)
	{
		LU(LOG_WARNING, "[UE] Send data error %u (%u-%u) %s\n", res, _curState, _isKeepAlive, _buf->Data());
		EndWork();
		return true;
	}
	return false;
}

bool HttpKeepAlivePostEvent::_ReceiveQuery()
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
				//LU(LOG_WARNING, "Bad red %c\n", beginQueryBuf[0]);
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


bool HttpKeepAlivePostEvent::_ParseURI(char *beginHeader, char *endHeader, bool usingSSL)
{
	const char* curEndHeader = endHeader;
	while (curEndHeader > beginHeader)
	{
		if (*curEndHeader == ' ')
			break;
		curEndHeader--;
	}

	++curEndHeader;
	static const String HTTP_1_1("HTTP/1.1");
	if (!strncasecmp(curEndHeader, HTTP_1_1.Data(), HTTP_1_1.GetLength()) && isspace( *(curEndHeader + HTTP_1_1.GetLength() + 1)) )
	{
		_isKeepAlive = true;
	}
	return HttpPostEvent::_ParseURI(beginHeader, endHeader, usingSSL);
}

				
void HttpKeepAlivePostEvent::Call(int events)
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
			if (_timeOutDescr != 0)
			{
				close(_timeOutDescr);
				_timeOutDescr = 0;
				if (!_listener->callTimer(this))
				{
					SendErrorPageAndClose();
					return;
				}
			}
		}
		else if ((_curState == WAIT_QUERY) || (_curState == WAIT_DATA))
		{
			_UpdateOpTime();
			if (_curState == WAIT_QUERY)
			{
				if (!_ReceiveQuery())
				{
					SendErrorPageAndClose();
					return;
				}
			}
			else if (_curState == WAIT_DATA)
			{
				if (!_ReceivePOST())
				{
					SendErrorPageAndClose();
					return;
				}
			}

			if (_curState == RECEIVE_QUERY)
			{
				HttpEventListener::EFormAnswer res =_listener->FormKeepAliveAnswer(*_buf, this, _isKeepAlive);
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
						_curState = WAIT_ANSWER_RESULT;
						ClearEvent(EPOLLIN | EPOLLOUT);
						_thread->Ctrl(this);
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


void HttpKeepAlivePostEvent::SendErrorPageAndClose()
{
	_isKeepAlive = false;
	SendErrorPage();
}

void HttpKeepAlivePostEvent::SendErrorPage()
{
	if (!_buf)
		return;

	// no need to close connection on error, if it is really needed, error must be generated separatly and use ANS_SEND_N_CLOSE
	_buf->Null();
	if (_isKeepAlive)
		_buf->Add(_errorPageKeep.Data(), _errorPageKeep.Length());
	else
		_buf->Add(_errorPageClose.Data(), _errorPageClose.Length());
	_buf->AddEndZero();
	sendAnswer();
}

void HttpKeepAlivePostEvent::sendAnswer()
{
	if (_curState == END_WORK)
		return;
	if (!_TrySendAnswer())
	{
		if (_curState != SEND_N_CLOSE)
			_curState = SEND_ANSWER;
		ClearEvent(EPOLLIN);
		AddEvent(EPOLLOUT);
		_thread->Ctrl(this);
	}
}

