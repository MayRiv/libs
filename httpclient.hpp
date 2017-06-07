#pragma once

#include <stdio.h>

#include "xtypes.hpp"
#include "string.hpp"
#include "icharset.hpp"


namespace lb
{
	const int ER_HTTP_CANNOT_CONNECT 	= -1;
	const int ER_HTTP_CANNOT_SEND 		= -2;
	const int ER_HTTP_BAD_ANSWER  		= -3;
	const int ER_HTTP_BAD_CODE 				= -4;
	const int ER_HTTP_REDIRECT				= -5;
	const int ER_HTTP_NOT_MODIF				= -6;
	const int ER_HTTP_CANNOT_SAVE			= -7;

	const int HEADER_CONTENT_TYPE = 1;
	const int HEADER_LOCATION			= 2;
	const int HEADER_LAST_MOD			= 3;


	const int HTTP_CODE_OK 								= 200; // "200"  OK
	const int HTTP_CODE_MULTIPLE_REDIRECT = 300; // "300"  Multiple Choices
	const int HTTP_CODE_MOVED_PERMANENTLY = 301; // "301"  Moved Permanently
	const int HTTP_CODE_FOUND 						= 302; // "302"  Found
	const int HTTP_CODE_SEE_OTHER 				= 303; // "303"  See Other
	const int HTTP_CODE_NOT_MODIF 				= 304; // "304"  Not Modified
	const int HTTP_CODE_USE_PROXY 				= 305; // "305"  Use Proxy
	const int HTTP_CODE_TEMP_REDIRECT 		= 307; // "307"  Temporary Redirect
	const int HTTP_CODE_BAD_REQUEST				= 400; // "400"  Bad Request
	const int HTTP_CODE_UNAUTHORIZED			= 401; // "401"  Unauthorized
	const int HTTP_CODE_PAYMENT_REQ				= 402; // "402"  Payment Required
	const int HTTP_CODE_FORBIDDEN					= 403; // "403"  Forbidden
	const int HTTP_CODE_NOTFOUND					= 404; // "404"  Not Found
	const int HTTP_CODE_METHOD_NOT_ALLOWED	= 405; // "405"  Method Not Allowed
	const int HTTP_CODE_MOT_ACCEPTABLE		= 406; // "406"  Not Acceptable

	const int MAX_HTTP_STR = 255;

	const int CONTENT_TYPE_TEXT_HTML		= 1;
	const int CONTENT_TYPE_TEXT_PLAIN		= 2;

	const int CONTENT_PARAM_CHARSET = 1;


	const int MAX_WAIT_TIME = 20;
	const int MAX_CONNECTION_WAIT_TIME = 5;

	class HttpClient
	{
	public:
		HttpClient(const char *userAgent, const char *bindOnIp = NULL, const int bindOnPort = 0)
			: _bindOnIp(bindOnIp), _bindOnPort(bindOnPort)
		{
			this->userAgent.Set(userAgent);
			lastModify = 0;
		}
		int Get(const class String &host, const class String &file, const Word32 ip, const Byte port, class RString &buf, time_t lastTime = 0, FILE *toFile = NULL);
		int GetLocal(const class String &file, RString &buf);
		void SetBuf(class RString &buf, ECharset charset, Byte contentType)
		{
			pbody = (char*)buf.Data();
			this->length = buf.Length();
			this->charset = charset;
			this->contentType = contentType;
		}
		char *Body() { return pbody; };
		String &Location() { return location; }
		Word32 Length() { return length; }
		ECharset Charset() 
		{ 
			return charset; 
		};
		Byte Code() { return httpCode; };
		Byte ContentType() { return contentType; }
		Word32 LastModify() { return lastModify; }
	protected:
		static const struct AssocPair headers[];
		static const struct AssocPair contentTypes[];
		static const struct AssocPair contentParams[];
		String _bindOnIp;
		int _bindOnPort;
		Word32 length;
		String location;
		String userAgent;
		Byte httpVersion;
		Word16 httpCode;
		Byte contentType;
		ECharset charset;
		Word32 lastModify;
		char *pbody;
	};
};

