#pragma once

#include <stdlib.h>
#include "string.hpp"
#include "xtypes.hpp"
#include "exception.hpp"

namespace lb
{
	enum EHttpError
	{
		HTTP_ERROR_BAD_METHOD = 1,
		HTTP_ERROR_BAD_HEADER,
		HTTP_ERROR_MAX_STR_REACH,
		HTTP_ERROR_EMPTY_GET_LINE,
		HTTP_ERROR_UNKOWN_PARAM,
		HTTP_ERROR_REQUEST_SMALL,
		HTTP_ERROR_RN,
	};

	const int MAX_HTTP_STR				= 1024;
	const int MAX_HTTP_USERAGENT	=	64;
	const int MAX_HTTP_QUERY			= 1024;

	const int MAX_PARAM_VALUE_LENGTH = 1024;

	enum EHttpHeader
	{
		HEADER_GET	= 1,
		HEADER_HOST,
		HEADER_USERAGENT,
		HEADER_XFORWARDED,
		HEADER_REFERER,
		HEADER_COOKIE,
	};

	#ifdef _HTTP_WITH_POST
	const int HEADER_CONTENT_LENGTH = 7;
	#endif

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
	static const int  BR_OPERA9   = 28;
	static const int  BR_OPERA10   = 20;
	static const int  BR_OPERA11   = 27;
	static const int  BR_OPERA12   = 29;
	static const int  BR_OPERAMOBILE= 22;

	static const int  BR_MSIE		= 2;
//	const int  BR_MSIE5		= 1;
	static const int  BR_MSIE6		= 5;
	static const int  BR_MSIE7		= 6;
	static const int  BR_MSIE8		= 3;
	static const int  BR_MSIE9		= 7;
	static const int  BR_MSIE10		= 14;

	static const int  BR_MSIEMOBILE	= 30;

	static const int  BR_NETSCAPE  = 12;

	static const int  BR_MOZILLA  = 23;
	static const int  BR_FIREFOX  = 24;
//	static const int  BR_FIREFOX2  = 13;
	static const int  BR_FIREFOX3  = 4;
	static const int  BR_FIREFOX4  = 8;
	static const int  BR_FIREFOX5  = 10;
	static const int  BR_FIREFOX6  = 11;
	static const int  BR_FIREFOX7 = 12;
	static const int  BR_FIREFOX8  = 13;
	static const int  BR_FIREFOX9  = 15;




	//	const int  BR_KONQUEROR = 25;
		static const int  BR_SAFARI	= 26;
		static const int  BR_CHROME	= 9;

		static const int  NOT_PRESENT_BROWSER	= 255;


	struct HttpHeader
	{
		int id;
		const char *header;
	};

	struct HttpParam
	{
		int indx;
		char *param;
	};


	class HttpError : public Exception 
	{
	public:
		HttpError(int type) : Exception(type) {};
		virtual const char* GetStr() { return strs[type];};
	protected:
		static const char* strs[];
			
	};


	class Http
	{
	public:
		Http(char *buf, unsigned int size);
		~Http();
		int IsError() { return isError; }
		Word32 XForwardIP() { return xForwardIP; }
		String &What() { return what; }
		String &Referer() { return referer; }
		Byte Os() { return os; }
		Byte Browser() { return brws; }
		Byte IsCookies() { return isCookies; }
	protected:
		void NullData() {};
		void GetOsBrws(char *userAgent, int len);
		int ParseOs(char * os_str);
		int GetHeaderId(char *header);
		void ParseQuery(String &query);
		bool _IsBot(char *userAgent);
	#ifdef _HTTP_WITH_POST
		int ParsePost(char *pquery);
		int ContentLength() { return contentLength; }
	#endif

		static const HttpHeader headers[];
		String referer;
		String what;
		String host;
	  
		Byte  brws;
		Byte  os;
			
		Byte  isCookies;
		int isError;
		Word32 xForwardIP;
		
	#ifdef _HTTP_WITH_POST
		int contentLength;
	#endif
		// Include project specific parse...
		#include "spechttp.hpp"
	};
};


