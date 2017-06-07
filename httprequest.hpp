#pragma once

#include "hashmap.hpp"
namespace lb
{
	class HttpRequest
	{
	public:
		HttpRequest(const Word32 reqTime);
		bool Parse(RString &buf);
		const String &Host() const
		{
			return _reqHost;
		}
		const String &ReqURI() const
		{
			return _reqURI;
		}
		const Word32 XRealIP() const
		{
			return _xRealIP;
		}
		const Word32 Fwrd() const
		{
			return _fwrd;
		}
		const Word32 ReqTime() const
		{
			return _reqTime;
		}
		const String &Cookies() const
		{
			return _cookies;
		}
		const String &PostData() const
		{
			return _postData;
		}
		const bool GetCook(const String &name, String &value) const;
	private:
		bool _ParseQuery(char *begin, char *end);
		bool _ParseHeader(char *beginHeader, char *endHeader);
		bool _CheckHeader(const char *beginHeaderName, const int nameLen, char *beginHeader, char *endHeader, const String &header, String &value);

		enum ERequestType
		{
			UNKNOWN,
			GET,
			POST,
			HEAD,
		};
		ERequestType _type;
		Word32 _reqTime;
		String _reqURI;
		String _reqHost;
		String _cookies;
		String _postData;
		Word32 _xRealIP;
		Word32 _fwrd;
	};
}
