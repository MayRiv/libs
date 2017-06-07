#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "exception.hpp"
#include "http.hpp"
#include "tools.hpp"

namespace lb
{

Http::Http(char *buf, unsigned int size)
{
	char *pbuf = buf;
	char *tbuf = 0, *rbuf = 0;
	int len, id;
	xForwardIP = 0;
	isCookies = 0;
	isError = 0;
	brws = NOT_PRESENT_BROWSER;
	os = NOT_PRESENT_BROWSER;
	InitData();	
	unsigned int j = 0;
	if (size < 32)
	{
		isError = HTTP_ERROR_REQUEST_SMALL;
		return;
		//throw HttpError(HTTP_ERROR_BAD_METHOD);
	}
	
	if (
			(*pbuf == 'G' &&  *(++pbuf) == 'E' &&  *(++pbuf) == 'T' &&  (*(++pbuf) == ' ' || *pbuf== '\t')) || 
			(*pbuf == 'P' &&  *(++pbuf) == 'O' &&  *(++pbuf) == 'S' &&  *(++pbuf) == 'T' && (*(++pbuf) == ' ' || *pbuf== '\t'))
		 )
	{
		pbuf++;
		len = strcspn(pbuf, " ?");

		if ((len <= 0) || (len > (int)MAX_HTTP_URL))
		{
    	isError = HTTP_ERROR_EMPTY_GET_LINE;
			return;
		}
		else
		{
			CheckFile(pbuf, len);
			pbuf += len;
			if (*pbuf == '?')
			{
				pbuf++;
				tbuf = strchr(pbuf, ' ');
				len = tbuf ? (tbuf - pbuf) : -1;
				if (len > 0)
				{
					int oldLen = len;
					if (len >= MAX_HTTP_QUERY)
						len = MAX_HTTP_QUERY - 1;

					what.Set(pbuf, len);
					ParseQuery(what);
					pbuf += oldLen;
				}
				else if (len == 0)
				{
				}
				else
				{
					isError = HTTP_ERROR_EMPTY_GET_LINE;
					return;
				}
			}
		}
		rbuf = strchr(pbuf, '\r');
		if (!rbuf || *(++rbuf) != '\n')
		{
			isError = HTTP_ERROR_RN;
			return;
		}

		pbuf = rbuf + 1;
		while (*pbuf)
		{
			tbuf = strchr(pbuf, ':');
			len = tbuf - pbuf;
			if ((len <= 0) || (len > MAX_HTTP_STR))
			{
        isError = HTTP_ERROR_MAX_STR_REACH;
				return;
				//throw HttpError(HTTP_ERROR_MAX_STR_REACH);
			}
			else
			{
				*tbuf = 0;
				id = GetHeaderId(pbuf);
				pbuf=tbuf+1;
				rbuf = strchr(pbuf, '\r');
				if (!rbuf || *(++rbuf) != '\n')
				{
					isError =HTTP_ERROR_RN;
					return;
		//				throw HttpError(HTTP_ERROR_BAD_METHOD);
				}
				len = rbuf-pbuf;
				if (id && len > 0 && len < MAX_HTTP_STR)
				{
					j=strspn(pbuf, " \t");
					pbuf += j;
					len -= j+1;//remove spaces && '\r\n' from len
					*(pbuf+len) = 0;// zero '\r'	
					switch (id)	
					{
						case HEADER_HOST:
							host.Set(pbuf, len);
							break;		
						case HEADER_USERAGENT:
							GetOsBrws(pbuf, len);
							break;
						case HEADER_XFORWARDED:
							*(rbuf-1) = 0;

							tbuf = strchr(pbuf, ',');
							if (!tbuf)
							{
								if ((xForwardIP = inet_network(pbuf)) == INADDR_NONE)
									xForwardIP = 0;
							}
							else
							{
								*tbuf = 0;
							  if ((xForwardIP = inet_network(pbuf)) == INADDR_NONE)
							    xForwardIP = 0;
							}
							break;
						case HEADER_REFERER:
							if ((len > 11) && !memcmp(pbuf, "http://", 7))
							{
								referer.SetUnescapeChars(pbuf+7, len-7); // skip http:// from pbuf
							}
							break;
						case HEADER_COOKIE:
							//*(rbuf-1) = 0;
							ParseCookie(pbuf);
							break;
#ifdef _HTTP_WITH_POST					
						case HEADER_CONTENT_LENGTH:
							contentLength = strtol(pbuf, (char **)NULL, 10);
							break;
#endif
					}
				}
				pbuf=rbuf+1;
				if (*pbuf == '\r')
					break;
			}
		}
	}
	else
	{
		isError = HTTP_ERROR_BAD_METHOD;
		return;
		//throw HttpError(HTTP_ERROR_BAD_METHOD);
	}
#ifdef _HTTP_WITH_POST					
	if (buf[0] != 'P')
		return;
	if (!contentLength)
	{
		isError = HTTP_ERROR_EMPTY_GET_LINE;
		return;
	}
	if (*pbuf == '\r' && (*(++pbuf) == '\n') && *(++pbuf))
	{
		ParsePost(pbuf);
	}
#endif

//	if (what)
//		ParseQuery(what);
		
}

#ifdef _HTTP_WITH_POST
int Http::ParsePost(char *pquery)
{
	int i = 0;
	
	char param;
	while (*pquery)
	{
		param = *pquery;
		++pquery;
		if (*(pquery) != '=')
		{
			isError = HTTP_ERROR_UNKOWN_PARAM;
			return 0;
		}
		char *pq  = pquery;
 		while (*(++pq) && *pq != '&')
 				i++;
 		pquery++;
 		if (i)
 		{
 			String paramValueBuf(pquery, i);
 			if (!SetParam(param, paramValueBuf.c_str(), i))
 				return 0;
 			pquery += i;
   		i = 0;
 		}
 		if (!*pquery)
 				break;
		pquery++;
  }			
  return 1;
}
#endif

void Http::ParseQuery(String &query)
{
	char *pquery = query.c_str();
	char paramValueBuf[MAX_PARAM_VALUE_LENGTH];
	int i = 0;
	
#ifdef _NO_HTTP_PARAM
	int count = 0;
#else
	char param;
#endif

	while (*pquery)
	{
#ifndef _NO_HTTP_PARAM
		param = *pquery;
		while (*(++pquery) && *pquery != '&')
#else
		count++;
		while (*(pquery) && *pquery != '&')
#endif 
		{
			if (i < MAX_PARAM_VALUE_LENGTH-1)
			{
#ifndef _NO_HTTP_PARAM
				paramValueBuf[i] = *pquery;
#else
				paramValueBuf[i] = *(pquery++);
#endif
				i++;
			}
		}
		if (i)
		{
			paramValueBuf[i] = 0;
#ifdef _NO_HTTP_PARAM
			if (!SetParam(count, paramValueBuf, i))
				return;		
#else
			if (!SetParam(param, paramValueBuf, i))
				return;		
#endif
  		i = 0;
		}
		if (!*pquery)
				break;
		pquery++;
  }			
}


Http::~Http()
{
	/*
	delete what;
	delete userAgent;
	delete host;
	for (int i = 0; i <= PARAMETER_COUNT; i++)
				delete params[i];
			*/
}



int Http::ParseOs(char * os_str)
{
	register char * t;
	//printf("\nPARSE_OS %.10s\n", os_str);
	if ((t = strstr(os_str,"Win"))!=NULL)
	{
		t += 3;
		//printf("%s\n", t);
		if(memcmp(t,"dows",4)==0)
			t += 4;
		if(*t==' '||*t=='_')
			t++;
		else if (*t==';')	{//Windows; U; Windows NT 5.0
			register char * p;
			if ((p = strstr(t, "Win"))!=NULL) {
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
		if ( (t[0]=='N' && t[1]=='T')) {
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
					return OS_WINVISTA;
				}
				if (t[3]=='4')
					return OS_WINNT;
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

	if (strstr(os_str,"Android"))
	{
		return OS_ANDROID;
	}

	if (strstr(os_str,"Linux"))
	{
		return OS_LINUX;
	}
//	if (strstr(os_str,"FreeBSD"))
//	{
//		return OS_FBSD;
//	}

	if (strstr(os_str,"iPhone") || strstr(os_str,"iPad"))
	{
		return OS_IOS;
	}

	if ((t = strstr(os_str,"Mac"))!=NULL)
	{
		t+=3;
		if(*t=='i' || *t=='_')
			return OS_MAC;
	}

	if (strstr(os_str,"Symbian"))
	{
		return OS_SYMBIAN;
	}
	if (strstr(os_str,"/MIDP"))
	{
		return OS_J2ME;
	}
	
//	if (strstr(os_str,"Sun"))
//	{
//		return OS_SUN;
//	}

	return 0;
}

void Http::GetOsBrws(char *userAgent, int len){
								   // 0 1  2  3  4  5  6  7   8  9
	static const Byte opera[10]=     {0,20, 19,19,19,19,19,19, 27, 28};
	static const Byte msie[10]=	     {0,2, 2, 2, 2, 1, 5, 6,  3, 7};
	static const Byte netscape[10] = {0,0,12,12,12,12,12,12, 12, 0};
	register char *p=userAgent,*s;
	Byte version = 0;
	char ch;
	os = 0;
	brws = 0;

	if (_IsBot(userAgent))
		return;

	if (len > 18) //magic lenght to ensure that strstr are safe
	{
		if (!memcmp("Mozilla", userAgent, 7))
		{
			ch = userAgent[8];
			if (ch >= '0' && ch <= '9')
			{
				version = ch - '0';
				brws = netscape[version];
			}
			p+=9;
			//printf("M %.4s V%c ",p,ch);
			if ( (s = strchr(p,'('))!=NULL) {//MSIE || Netscape && OS
				if ((p = strstr(s,"MSIE"))!=NULL ) {
//				printf("Msie %.4s V%d ",p,version);
					p+=5;
					os = ParseOs(p+2);
					ch = p[0];
					if (ch>='0' && ch<='9')
					{
						version = ch - '0';
						if ((version == 4 || version == 6) && strstr(p+2, "Windows CE") != NULL)
						{
							brws = BR_MSIEMOBILE;
						}
						else
							brws = msie[version];
					}
					if ((p = strstr(s, "Opera"))!=NULL) {//search after MSIE string
						brws = BR_OPERA;
						ch = p[6];
						if(ch>='0' && ch <='9') {
							version = ch - '0';
							brws = opera[version];
							//printf("\nOpr %.4s V%c ",p,ch);
						}
					}
				}//User-Agent: Mozilla/5.0 (Windows; U; Windows NT 5.0; en-US; rv:1.6) Gecko/20040206 Firefox/0.8
				else if ( (p = strstr(s, "Gecko/"))!=NULL ) {
					char *pGecko = p + 7;
					brws = BR_MOZILLA;
					if ((p = strstr(pGecko, "Firefox")) != NULL)
					{
						brws = BR_FIREFOX;
						ch = p[8];
						if (ch == '3')
						{
							brws = BR_FIREFOX3;
						}
						else if (ch == '4')
						{
							brws = BR_FIREFOX4;
						}
						else if (ch == '5')
						{
								brws = BR_FIREFOX5;
						}
						else if (ch == '6')
						{
								brws = BR_FIREFOX6;
						}
						else if (ch == '7')
						{
								brws = BR_FIREFOX7;
						}
						else if (ch == '8')
						{
								brws = BR_FIREFOX8;
						}
						else if (ch == '9')
						{
								brws = BR_FIREFOX9;
						}
					}
					else if ( (p = strstr(pGecko, "Netscape"))!=NULL )
					{	//Gecko/20020823 Netscape/7.0
						ch = p[9];
						if (ch >= '0' && ch <= '9') {
							brws = netscape[ch - '0'];
						}
						else {
							ch = p[10];
							if (ch>='0' && ch <='9') {
								brws = netscape[ch - '0'];
							}
						}
					}
				}
				//else if ((p = strstr(s, "Konqueror")) != NULL) {//Mozilla/5.0 (compatible; Konqueror/3.3; Linux; en_US) (KHTML, like Gecko)
				//	brws = BR_KONQUEROR;
				//}
				else if ((p = strstr(s, "Chrome")) != NULL)//check before safari because may contain string Safari at the end.
					brws = BR_CHROME;
				else if ((p = strstr(s, "Safari")) != NULL) {//Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US) AppleWebKit/525.19 (KHTML, like Gecko) Chrome/1.0.154.36 Safari/525.19
					brws = BR_SAFARI;
				}

				if (!os) {
//					printf("%s\n",s);
					os = ParseOs(s);
				}
			}
		}
		else if (!memcmp("Opera",userAgent, 5)) {// Opera Mobile Opera Mini Opera/9 
			brws = BR_OPERA;
			ch = userAgent[6];
			if (ch>='0' && ch <='9') {
				version = ch - '0';
				brws = opera[version];
				if (version == 9)//trick to parse 10.00 and higher
				{
					if ((p = strstr(userAgent + 8, "Version/")) != NULL)
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
			} else if ( ch == 'M')
			{
				brws = BR_OPERAMOBILE;
			}
			os = ParseOs( userAgent + 8);
			//printf("\nO %.4s V%c ",p,ch);
		}
	}//len smaller than 18
	
	if(!os)
		os = ParseOs(userAgent);
	//if(!brws)
	//printf("br %d os %d %s\n", brws, os, userAgent);
}

const HttpHeader Http::headers[] =
{
//	{HEADER_GET, "GET"},
	{HEADER_HOST, "HOST"},
	{HEADER_USERAGENT, "USER-AGENT"},
	{HEADER_XFORWARDED, "X-Forwarded-For"},
	{HEADER_REFERER, "Referer"},
	{HEADER_COOKIE, "Cookie"},
#ifdef _HTTP_WITH_POST
	{HEADER_CONTENT_LENGTH, "Content-length"},
#endif
	{0, NULL}
	
};

bool Http::_IsBot(char *userAgent)
{
	if (strcasestr(userAgent, "Googlebot"))
		return true;
	if (strcasestr(userAgent, "Yandex/"))
		return true;
	if (strcasestr(userAgent, "StackRambler"))
		return true;
	if (strcasestr(userAgent, "Slurp"))
		return true;
	if (strcasestr(userAgent, "msnbot") || strcasestr(userAgent, "bingbot"))
		return true;

	return false;
}

/*
int Http::GetParamIndex(char *str)
{
	for (int i = 0; i < PARAMETER_COUNT; i++)
		if (!strcmp(str, acceptParams[i].param))
			return acceptParams[i].indx;
	return 0;
};
*/
int Http::GetHeaderId(char *str)
{
	int i = 0;
	while (headers[i].id)
	{
		if (!strcasecmp(str, headers[i].header))
			return headers[i].id;
		i++;
	}
	return 0;	
}

const char* HttpError::strs[] = 
{
	"Unknown Error",
	"Method must be GET",
	"Empty header detected",
	"Header max length reach",
	"Empty query line",
	"Unknown query param",
	"Request smaller then 35b",
	"Str not end with n/r"
};

};
