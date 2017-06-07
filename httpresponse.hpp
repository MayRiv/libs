#pragma once

#include "xtypes.hpp"
#include "string.hpp"

namespace lb
{
	const char * const NO_CACHE_HEADERS = "Cache-Control: no-cache, no-store, must-revalidate, no-cache=Set-Cookie, max-age=0, proxy-revalidate\r\nExpires: Thu, 01 Jan 1970 00:00:00 GMT\r\nPragma: no-cache\r\n";
	class RequestPos
	{
	public:
		RequestPos()
			: _contentLengthStart(0), _headersEnd(0)
		{

		}
		const Word32 headersEnd() const
		{
			return _headersEnd;
		}
		void SetHeadersEnd(const Word32 headersEnd)
		{
			_headersEnd = headersEnd;
		}
	private:
		friend class HTTPResponse;
		Word32 _contentLengthStart;
		Word32 _headersEnd;
	};

	class HTTPResponse
	{
	public:
		static void FormHeaders(const char *contentType, RString &cookies, RequestPos &rp, RString &buf, bool isKeepAlive = false, const char *addHeaders = NULL)
		{
			buf.snprintf("HTTP/1.1 200 OK\r\n%s%sContent-Type: %s\r\n", NO_CACHE_HEADERS, cookies.Data(), contentType);
			if (addHeaders)
				buf.AddStr(addHeaders);
			rp._contentLengthStart = buf.Length();
			// add Server SHolder header for size manipulate
			buf.snaprintf("Content-Length: 00\r\nServer: 11210b/SHolder\r\n");
			if (isKeepAlive)
			{
				buf.snaprintf("Connection: Keep-Alive\r\n\r\n");
			}
			else
			{
				buf.snaprintf("Connection: close\r\n\r\n");
			}
			rp._headersEnd = buf.Length();
		}
		static void FormContentLength(RequestPos &rp, RString &buf)
		{
			if (!rp._contentLengthStart)
				return;

			int contentLen = buf.Length() - rp._headersEnd;
			char *contentLengthStart = (char*)buf + rp._contentLengthStart + sizeof("Content-Length:");
			int lengthRes = snprintf(contentLengthStart, 6, "%02u", contentLen);
			if (lengthRes > 2) // need move Server header
			{
				char *serverStart = contentLengthStart + lengthRes;
				static String SERVER_STRING("\r\nServer: ");
				memcpy(serverStart, SERVER_STRING.c_str(), SERVER_STRING.GetLength());
			}
			else if (lengthRes > 0) // set zero byte to \r
				*(contentLengthStart + lengthRes) = '\r';
		}
	};
};
