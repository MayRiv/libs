#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "xtypes.hpp"
#include "log.hpp"
#include "httpclient.hpp"
#include "socket.hpp"
#include "string.hpp"
#include "tools.hpp"
#include "time.hpp"
#include "icharset.hpp"

using namespace lb;

int HttpClient::Get(const String &host, const String &file, const Word32 ip, const Byte port, RString &buf, const time_t lastTime, FILE *toFile)
{
	pbody = NULL;
	charset = CONTENT_CHARSET_ENGLISH;
	location.Set("");
	
	SocketClient sock;
	if (_bindOnIp && !sock.BindOnIp(_bindOnIp.c_str(), _bindOnPort))
	{
		L(LOG_ERROR, "[HC] Cannot bind on %s:%u\n", _bindOnIp.c_str(), _bindOnPort);
		return ER_HTTP_CANNOT_CONNECT;
	}

	if (!sock.Connect(ip,  port, MAX_CONNECTION_WAIT_TIME))
		return ER_HTTP_CANNOT_CONNECT;

	buf.snprintf("GET /%s HTTP/1.0\r\nAccept: */*\r\nHost: %s\r\nReferer: http://%s/%s\r\n", file.Data(), host.c_str(), host.c_str(), file.Data());
	if (lastTime)
	{
    struct tm t;
    gmtime_r(&lastTime, &t);
    char dataBuf[27];
    asctime_r(&t, dataBuf);
		buf.snaprintf("If-Modified-Since: %s GMT\r\n", dataBuf);
	}
	buf.snaprintf("User-Agent: %s\r\n\r\n", userAgent.c_str());
	if (sock.Send(buf, buf.Length(), MAX_WAIT_TIME) !=  buf.Length())
		return ER_HTTP_CANNOT_SEND;

	int len = 0;
	if ((len = sock.ReadBin(buf, buf.MaxSize()-1, MAX_WAIT_TIME, 1)) <= 0)
		return ER_HTTP_CANNOT_SEND;

	length = len;
	buf.SetLength(len);
	char *pstr = buf;
	if (*pstr != 'H' || *(++pstr) != 'T' || *(++pstr) != 'T' || *(++pstr) != 'P' || *(++pstr) != '/' || *(++pstr) != '1' || *(++pstr) != '.')
		return ER_HTTP_BAD_ANSWER;

	if (*(++pstr) == '1')
		httpVersion = 1;
	else
		httpVersion = 0;
	if (*(++pstr) != ' ')
		return ER_HTTP_BAD_ANSWER;

	pstr++;
	char *tstr = NULL;
	httpCode = strtol(pstr, &tstr, 10);
	switch (httpCode)
	{
		case HTTP_CODE_OK:
			break;
		case HTTP_CODE_MULTIPLE_REDIRECT:
		case HTTP_CODE_MOVED_PERMANENTLY:
		case HTTP_CODE_FOUND:
		case HTTP_CODE_SEE_OTHER:
			break;
		case HTTP_CODE_NOT_MODIF:
			return ER_HTTP_NOT_MODIF;
		default:
			return ER_HTTP_BAD_CODE;
	}
	pstr = tstr;

	tstr = strchr(pstr, '\r');
	if (!tstr || !(*(++tstr) == '\n'))
		return ER_HTTP_BAD_ANSWER;

	pstr = tstr + 1;
	
	int id = 0;
	int j = 0;
	int paramID = 0;
	char *vstr = NULL;
	
	// Begin parse headers
  while (*pstr)
  {
    tstr = strchr(pstr, ':');
    len = tstr - pstr;
    if ((len <= 0) || (len > MAX_HTTP_STR))
    {
     	return -5;
    }
  	*tstr = 0;
  	id = GetAssocPairKey(pstr, headers);
  	pstr=tstr+1;

  	tstr = strchr(pstr, '\r');
  	if (!tstr || !(*(++tstr) == '\n'))
  		return -3;

		len = tstr-pstr;
  	
  	if (id && len > 0 && len < MAX_HTTP_STR)
  	{
  		j=strspn(pstr, " \t");
  		pstr += j;
  		len -= j+1;//remove spaces && '\r\n' from len
  		*(pstr+len) = 0;// zero '\r'	
  		switch (id)	
  		{
  			case HEADER_CONTENT_TYPE:
  			{
  				vstr = strchr(pstr, ';');
  				if (vstr)
  					*vstr =  0;
  				contentType = GetAssocPairKey(pstr, contentTypes);
  				if (vstr)
  					pstr = vstr + 1;
  				else
  					break;
  				if (*pstr == ' ')
  					pstr++;
  				do  // parse paramters (RFC media-type = type "/" subtype *( ";" parameter )
					{
						vstr = strchr(pstr, '=');
						if (!vstr)
							break;
						*vstr = 0;
						paramID = GetAssocPairKey(pstr, contentParams);
						switch (paramID)
						{
							case CONTENT_PARAM_CHARSET:
							{
								char q = 0;
								char *qend = NULL;

								if (IsQuote(*(vstr+1)))
								{
									char q = *(vstr+1);
									vstr++;
									char *qend = strchr(vstr+1, q);
									if (qend)
										*qend = 0;
								}
								else // skip ';' and spaces
								{
									char *qend = strchr(vstr+1, ';');
									if (qend)
										*qend = 0;
									else if ((qend = strchr(vstr+1, ' ')) != NULL)
										*qend = 0;
								}
								

								charset = (ECharset)GetAssocPairKey(vstr+1, charsets);
								if (!charset)
								{
									//L(LOG_ERROR, "Bad charset %s on %s/%s\n", vstr+1, host.c_str(), file.Data());
									charset = CONTENT_CHARSET_UNKNOWN;
								}
								if (qend)
									*qend = q;
								break;
							}
						}
					} while ((vstr = strchr(pstr, ';')) != NULL);
  			}
  			break;
  			case HEADER_LOCATION:
  				location.SetUnescapeChars(pstr, len);
	 			break;
	 			case HEADER_LAST_MOD:
	 				lastModify = Time::Str2Unix(pstr);
	 			break;
	 				
  		}
  	}
  	pstr=tstr+1;
  	if (*pstr == '\r')
  		break;
  }
	if (*pstr != '\r' || *(++pstr) != '\n' || !*(++pstr))
 		return ER_HTTP_BAD_ANSWER;

	if (location)
		return ER_HTTP_REDIRECT;
	pbody = pstr;
	
	len = length;
	length -=  pbody - buf.Data();
	if (toFile)
	{
		if (!fwrite(pbody, length, 1, toFile))
		{
			L(LOG_ERROR, "Cannot save to file\n");
			return ER_HTTP_CANNOT_SAVE;
		}
		while (len > buf.MaxSize()-2)
		{
			if ((len = sock.ReadBin(buf, buf.MaxSize()-1, MAX_WAIT_TIME, 1)) > 0)
			{
				if (!fwrite(buf, len, 1, toFile))
				{
					L(LOG_ERROR, "Cannot save to file\n");
					return ER_HTTP_CANNOT_SAVE;	
				}
				length += len;
			}
		}
	}
	return 1;
}

const AssocPair HttpClient::headers[] =
{
	{HEADER_CONTENT_TYPE, "content-type"},
	{HEADER_LOCATION, "location"},
	{HEADER_LAST_MOD, "last-modified"},
	{0, NULL}
	
};

const AssocPair HttpClient::contentTypes[] = 
{
	{CONTENT_TYPE_TEXT_HTML, "text/html"},
	{CONTENT_TYPE_TEXT_PLAIN, "text/plain"},
	{0, NULL}
};

const AssocPair HttpClient::contentParams[] = 
{
	{CONTENT_PARAM_CHARSET, "charset"},
	{0, NULL}
};

   


int HttpClient::GetLocal(const class String &file, RString &buf)
{
	FILE *fp = fopen(file.c_str(), "r");
	if (!fp)
		return 0;
	struct stat sb;
  fstat(fileno(fp), &sb);
  fread((char*)buf, 1, sb.st_size, fp);
  buf.SetLength(sb.st_size);
  pbody = buf;
	fclose(fp);
	return 1;
}
