#include "httprequest.hpp"
#include "log.hpp"
#include "socket.hpp"

using namespace lb;

HttpRequest::HttpRequest(const Word32 reqTime)
	: _type(UNKNOWN), _reqTime(reqTime), _xRealIP(0), _fwrd(0)
{
}

const int MIN_HTTP_REQ = 10;

bool HttpRequest::Parse(RString &buf)
{
	if (buf.Length() < MIN_HTTP_REQ)
		return false;
	char *pBuf = buf;
	if (*pBuf == 'G')
		_type = GET;
	else if (*pBuf == 'P')
		_type = POST;
	else if (*pBuf == 'H')
		_type = HEAD;
	else
	{
		L(LOG_WARNING, "Bad http req %s\n", buf.Data());
		return false;
	}
	char *endBuf = buf + buf.Length();
	static const char *endChars = "\r\n\r\n";
	const char *curEndChar = endChars;

	char *beginHeader = pBuf;
	while (pBuf != endBuf)
	{
		if ((*pBuf == *curEndChar))
		{
			if (*curEndChar == '\n')
			{
				char *endHeader = pBuf - 1;
				if (beginHeader == buf) // begin
				{
					if (!_ParseQuery(beginHeader, endHeader))
						return false;
				}
				else
				{
					if (beginHeader != endHeader)
					{
						if (!_ParseHeader(beginHeader, endHeader)) // pBuf - \r\n
							return false;
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
	if (_type == POST)
	{
		pBuf++; // skip last \n 
		int postLen = endBuf - pBuf;
		if (postLen > 0)
			_postData.Set(pBuf, postLen);
	}
	//L(LOG_WARNING, "Recive httpreq: %s (%s) - %s (P: %s (%u)\n", _reqURI.c_str(), _reqHost.c_str(), _cookies.c_str(), _postData.c_str(), _postData.GetLength());
	return true;
}

bool HttpRequest::_ParseQuery(char *begin, char *end)
{
	while (begin != end)
	{
		if (*begin == ' ')
		{
			begin++;
			char *beginURI = begin;
			while (begin != end)
			{
				if (*begin == ' ') // end URI
				{
					int len = begin - beginURI;
					if (len > 0)
					{
						_reqURI.Set(beginURI, len);
						static const String HTTP_STR("http://");
						if (_reqURI.PartOf(HTTP_STR.c_str(), HTTP_STR.GetLength()))
						{
							char *beginHost = _reqURI.c_str() + HTTP_STR.GetLength();
							char *pHost = beginHost;
							char *endUri = _reqURI.c_str() + _reqURI.GetLength();
							while (pHost != endUri)
							{
								if (*pHost == '/' || *pHost == ':')
								{
									int hostLen = pHost - beginHost;
									if (hostLen > 0)
									{
										_reqHost.Set(beginHost,  hostLen);
										while (pHost != endUri)
										{
											if (*pHost == '/')
											{
												int resLen = endUri - pHost;
												if (resLen > 0)
													_reqURI.Set(pHost, resLen);
												else
													_reqURI.Set("/");
												break;
											}
											pHost++;
										}
									}
									break;
								}
								pHost++;
							}
						}
						return true;
					}
					else
					{
						L(LOG_ERROR, "Recive empty URI\n");
						return false;
					}
				}
				begin++;
			}
			L(LOG_ERROR, "Can't find end URI\n");
			return false;
		}
		begin++;
	}
	L(LOG_ERROR, "Recive bad requestURI\n");
	return false;
}
inline bool HttpRequest::_CheckHeader(const char *beginHeaderName, const int nameLen, char *beginHeader, char *endHeader, const String &header, String &value)
{
	if ((nameLen == header.GetLength()) && !strncasecmp(beginHeaderName, header.c_str(), header.GetLength())) // header host
	{
		while (beginHeader != endHeader)
		{
			if (isspace(*beginHeader))
				beginHeader++;
			else
				break;
		}

		int len = endHeader - beginHeader;
		if (len > 0)
		{
			value.Set(beginHeader, len);
			return true;
		}
		else
		{
			L(LOG_ERROR, "Recive empty %s header\n", header.c_str());
			return false;
		}
	}
	return false;
}

bool HttpRequest::_ParseHeader(char *beginHeader, char *endHeader)
{
	char *beginHeaderName = beginHeader;
	while (beginHeader != endHeader)
	{
		if (*beginHeader == ':')
		{
			int nameLen = beginHeader - beginHeaderName;
			beginHeader++;
			
			if (!_reqHost.GetLength())
			{
				static const String HOST_HEADER_NAME("HOST");
				if (_CheckHeader(beginHeaderName, nameLen, beginHeader, endHeader, HOST_HEADER_NAME, _reqHost))
				{
					char *pPort = strchr(_reqHost.c_str(), ':');
					if (pPort)
						_reqHost.Trim(pPort - _reqHost.c_str());
					return true;
				}
			}
			static const String COOKIE_HEADER_NAME("COOKIE");
			if (_CheckHeader(beginHeaderName, nameLen, beginHeader, endHeader, COOKIE_HEADER_NAME, _cookies))
				return true;

			static const String XREALIP_HEADER_NAME("X-REAL-IP");
			String xRealIp;
			if (_CheckHeader(beginHeaderName, nameLen, beginHeader, endHeader, XREALIP_HEADER_NAME, xRealIp))
			{
				_xRealIP = SocketClient::IpToLong(xRealIp.c_str());
				return true;
			}

			static const String XFWRD_HEADER_NAME("X_FORWARDED_FOR");
			String fwrd;
			if (_CheckHeader(beginHeaderName, nameLen, beginHeader, endHeader, XFWRD_HEADER_NAME, fwrd))
			{
				_fwrd = SocketClient::IpToLong(xRealIp.c_str());
				return true;
			}

			return true;
		}
		beginHeader++;
	}
	L(LOG_ERROR, "Recive bad header\n");
	return false;
}

const bool HttpRequest::GetCook(const String &name, String &value) const
{
	const char *p = _cookies.c_str() - 1;
	const char *pEnd = _cookies.c_str() + _cookies.GetLength();
	const char *brk;
	while (p) {
		while (*++p == ' ');
		brk = (const char *) memchr(p, ';', pEnd - p);

		if (!memcmp(p, name.c_str(), name.GetLength()) && p[name.GetLength()] == '=') {
			p += name.GetLength() + 1;
			if (!brk)
				brk = pEnd;
			value.Set(p, brk - p);
			return true;
		}

		p = brk;
	}

	return false;
}
