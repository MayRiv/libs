#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "string.hpp"
#include "tools.hpp"

using namespace lb;

const char String::HexValue[256] = {
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0, 0,
10,11,12,13,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
10,11,12,13,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

String::String(const char *str, int leng)
{
	if (leng)
	{
		data = new char[leng+1];
		memcpy(data, str, leng);
		data[leng] = 0;
		length = leng;
	}
	else
	{
		length = 0;
		data = NULL;
	}
}

String::String(const char *str)
	: data(NULL), length(0)
{
	if (str)
	{
		length = strlen(str);
		data = new char[length+1];
		memcpy(data, str, length);
		data[length] = 0;
	}
}

String::String(const RString &str)
{
	if (str.length)
	{
		length = str.length;
		data = new char[length+1];
		length = str.length;
		memcpy(data, str.data, length);
		data[length] = 0;
	}
	else
	{
		length = 0;
		data = NULL;
	}
}

String::String(const String &str)
	: data(NULL), length(0)
{
	if (str.length)
	{
		length = str.length;
		data = new char[length+1];
		memcpy(data, str.data, length);
		data[length] = 0;
	}
}

String::~String()
{
	if (data)
		delete[] data;
}

int String::ToInt() const
{
	if (length)
			return atoi(data);
	else
		return 0;
}

int String::ToOInt() const
{
	if (length)
			return  strtoul(data, NULL, 10);
	else
		return 0;
}

void String::Replace(const char from, const char to)
{
	for (int i = 0; i  < length; i++)
		if (data[i] == from)
			data[i] = to;
}
void String::Set(const String &str)
{
	char *oldData = data;
	if (str.length)
	{
		length = str.length;
		data = new char[length+1];
		memcpy(data, str.data, length+1);
	}
	else
	{
		data = NULL;
		length = 0;
	}
	if (oldData)
		delete[] oldData;
}

int String::InCase(const char *templ) const
{
	char *pstr = strcasestr(data, templ);
	if (pstr)
		return 1;
	else
		return 0;
}

int String::GetNextWord(int from, char *word, int maxWordLength)
{
	int i = from;
	int c = 0;
	for (; i < length && c < maxWordLength-1; i++, c++)
	{
		if (IsWordDelimiter(data[i]))
			break;
		else
			word[c] = data[i];
	}
	word[c] = 0;
	for (; i < length; i++)
		if (!IsWordDelimiter(data[i]))
			break;
	return i;
}

int String::In(const char *str) const
{
	char *pstr = strstr(data, str);
	if (pstr)
		return 1;
	else
		return 0;
}

int String::GetWord32(const char *param, int len, Word32 &val)
{
  if (len < length)
  {
  	char *pstr = strstr(data, param);
	  if (pstr)
  	{
	  	pstr += len;
		  val=strtoul(pstr, NULL, 10);
  		return 1;
	  }
  }
  val=0;
	return 0;
}

int String::UnescapeChars(const char *src, int srcLen, char *dst)
{
	char ch;
	int dst_len = 0;
	const char *end = src + srcLen;
	while (src < end)
	{
		ch = *src;
		if (ch == '%')
		{
			if( (*(src + 1) == 'u' || *(src + 1) == 'U') && (src + 3 < end) )
			{
				if ( (src + 5 < end) && isxdigit(*(src + 2)) && isxdigit(*(src + 3)) && isxdigit(*(src + 4)) && isxdigit(*(src + 5)) )
				{
						dst[dst_len++] = (HexValue[(unsigned char)*(src + 2)] << 4) + HexValue[(unsigned char)*(src + 3)];
						dst[dst_len++] = (HexValue[(unsigned char)*(src + 4)] << 4) + HexValue[(unsigned char)*(src + 5)];
						src += 6;
						continue;
				}
				else if ((src + 3 < end) && isxdigit(*(src + 2)) && isxdigit(*(src + 3)) )
				{
						dst[dst_len++] = (HexValue[(unsigned char)*(src + 2)] << 4) + HexValue[(unsigned char)*(src + 3)];
						src += 4;
						continue;
				}
			}
			else
			{
				if ( (src + 2 < end) && isxdigit(*(src + 1)) && isxdigit(*(src + 2)) )
				{
					dst[dst_len++] = (HexValue[(unsigned char)*(src + 1)] << 4) + HexValue[(unsigned char)*(src + 2)];
					src += 3;
					continue;
				}
			}
		}
		else if (ch == '+')
		{
			dst[dst_len++] = ' ';
			src++;
			continue;
		}

		dst[dst_len++] = ch;
		src++;
	}
	dst[dst_len] = 0;
	return dst_len;
}

void  String::UnescapeChars()
{
	if (data)
	{
		char *ddata = new char[length+1];
		length = UnescapeChars(data, length, ddata);
		delete[] data;
		data = ddata;
	}
}

void String::Set(const char *str, const int leng)
{
	char *oldData = data;
	if (leng)
	{
		data = new char[leng+1];
		memcpy(data, str, leng);
		length = leng;
		data[leng] = 0;
	}
	else
	{
		data = NULL;
		length = 0;
	}
	if (oldData)
		delete[] oldData;
}

void String::Set(const char *str)
{
	char *oldData = data;
	length = strlen(str);
	if (length)
	{
		data = new char[length+1];
		memcpy(data, str, length);
		data[length] = 0;
	}
	else
	{
		data = NULL;
		length = 0;
	}
	if (oldData)
		delete[] oldData;
}

void String::SetUnescapeChars(const char *str, int leng)
{
	if (data)
		delete[] data;

	data = new char[leng + 1];
	length = UnescapeChars(str, leng, data);
}

void String::SetEscapeChars(const char *str, const int len)
{
	if (data)
		delete[] data;
	length = len*3;
	data = new char[length+1];
	int res = 0;
	char *pnew = data;
	unsigned char c;
	for (int i = 0; i < len; i++)
	{
		c = str[i];
		if (isspace(c)) //Space
		{
		 *(pnew++) = '+';
		}
		else	if (isascii(c) && (!ispunct(c) || c == '.' || c == '-'))
		{
			*(pnew++) = c;
		}
		else // else chars
		{
			res=sprintf(pnew, "%%%02x", c);
			pnew += res;
		}
	}
	*pnew = 0;
	length = pnew-data;
}




void String::SetEscapeChars(const char *str)
{
	SetEscapeChars(str, strlen(str));
}



Word32 String::Hash32() const
{
	Word32 hash = 0;

	if (data)
	{
		for (int i = 0; i < length && i < 3; i++)
		hash += data[i]<<(i*8);
	}
	hash += length<<3*8;
	return hash;
}

int String::Trim(int len)
{
	if (len < length)
	{
		char* trimmed = new char[len+1];
		memcpy(trimmed, data, len);
		trimmed[len] = 0;
		delete [] data;
		data = trimmed;
		length = len;
		return 1;
	}
	return 0;
}

void String::RemoveDublicate(int maxlen, char badsymbol, char braker)
{
	int count = 0;
	for (int j = 0; j < length; j++)
	{
		if (data[j] == badsymbol)
		{
			count++;
			if (count >= maxlen)
				data[j] = braker;
		}
	}
}

void String::ReplaceNotAlphaNum(const char replace)
{
	for (int j = 0; j < length; j++)
	{
		unsigned char c = data[j];
		if (!isalpha(c) && !isdigit(c))
			data[j] = replace;
	}
}

void String::ReplaceWrongXmlChars(const char replace)
{
	static unsigned char *WRONG_XML_CHAR = GetCharTableFromString("\x01\x02\x03\x04\x05\x06\x07\x08\x0b\x0c\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f\x98");
	for (int j = 0; j < length; j++)
	{
		if (WRONG_XML_CHAR[(unsigned char)data[j]])
			data[j] = replace;
	}
}

void String::StripHtmlSpecial()
{
	for (int j = 0; j < length-1; j++)
	{
		if (data[j] == '&' && data[j+1] == '#')
		{
			int in = 0;
			int c = 0;
			for (int i = j+2; i < length; i++, c++)
			{
				if (isdigit(data[i]))
				{
					data[i] = ' ';
					in = 1;
				}
				else
				{
					if (in && data[i] == ';')
					{
						data[i] = ' ';
						c++;
					}
					break;
				}
			}
			if (in)
			{
				data[j] = ' ';
				data[j+1] = ' ';
				j += c+1;
			}
		}
	}
}

void String::StripTags()
{
	int count = 0;
	for (int j = 0; j < length; j++)
		switch (data[j])
		{
			case '<': //	&lt;
			case '>': //&gt;
				count += 3;
				break;
			case '"': // &quot;
				count += 5;
				break;
		}
	if (count)
	{
		char* strip = new char[length + count + 1];
		int c  = 0;
		for (int j = 0; j < length; j++)
			switch (data[j])
			{
				case '<': //	&lt;
					strip[c++] = '&';  strip[c++] = 'l'; strip[c++] = 't'; strip[c++] = ';';
					break;
				case '>': //&gt;
					strip[c++] = '&';  strip[c++] = 'g'; strip[c++] = 't'; strip[c++] = ';';
					break;
				case '"': // &quot;
					strip[c++] = '&';  strip[c++] = 'q'; strip[c++] = 'u'; strip[c++] = 'o'; strip[c++] = 't'; strip[c++] = ';';
					break;
				default:
					strip[c++] = data[j];
			}
		strip[c] = 0;
		delete[] data;
		data = strip;
		length = length + count;
	}
}

// I want put 'braker' after each 'len' characters
void String::WordWrap(int len, char braker = '\n')
{
	if (len >= length)
		return;

	int nonspace = 0;
	int dilator =  0;
	for (int i = 0; i < length; i++)
	{
		if (data[i] != ' ')
			nonspace++;
		else
			nonspace = 0;
		if (nonspace >= len)
		{
			dilator++;
			nonspace = 0;
		}
	}
	if (dilator)
	{
		char* wraped = new char[length + dilator + 1];
		nonspace = 0;
		int wlen = 0;
		for (int i = 0; i < length; i++)
		{
    	if (data[i] != ' ')
     		nonspace++;
     	else
     		nonspace = 0;
     	wraped[wlen++] = data[i];
     	if (nonspace >= len)
     	{
     		wraped[wlen++] = braker;
     		nonspace = 0;
     	}
		}
		if (wlen < length + dilator + 1)
		{
			wraped[wlen] = 0;
			delete[] data;
			data = wraped;
			length = wlen;
		}
	}
}

void String::Replace(const char *from, const char *to)
{
	if (!data || !length)
		return;

	int fromLen = strlen(from);
	int toLen = strlen(to);
	if (fromLen == toLen)
	{
		char *pchar = data;
		while ((pchar = strstr(pchar, from)) != NULL)
		{
			memcpy(pchar, to, toLen);
			pchar = pchar + toLen;
		}
	}
	else if (fromLen > toLen)
	{
		char *dst = data;
		char *pchar = data;
		char *lchar = NULL;
		while ((pchar = strstr(pchar, from)) != NULL)
		{
			if (lchar)
			{
				int len = pchar - lchar;
				if (len)
				{
					memcpy(dst, lchar, len);
					dst += len;
				}
			}
			else
				dst = pchar;

			memcpy(dst, to, toLen);
			dst += toLen;
			pchar = pchar + fromLen;
			lchar = pchar;
		}
		if (lchar)
		{
			int len = length - (lchar - data);
			if (len)
			{
				memcpy(dst, lchar, len);
				dst += len;
			}
			length = dst - data;
			data[length] = 0;
		}
	}
	else
	{
		RString rdata(length + toLen);
		char *pstr = data;
		char *lstr = data;
		while ((pstr = strstr(pstr, from)) != NULL)
		{
			rdata.Add(lstr, pstr - lstr);
			rdata.Add(to, toLen);
			pstr = pstr + fromLen;
			lstr = pstr;
		}
		if (lstr != data)
		{
			int len = length - (lstr - data);
			if (len)
				rdata.Add(lstr, len);
			Set(rdata.Data(), rdata.Length());
		}
	}
}

int String::StrReplace(const char *from, const char *to)
{
	unsigned int fromlen = strlen(from);
	if (fromlen > strlen(to))
		return 0;
	for (int i = 0; i < length; i++)
		for (unsigned int j = 0; j < fromlen; j++)
			if (data[i] == from[j])
			{
				data[i] = to[j];
				break;
			}
	return 1;
}

void String::operator ^=(const char *str) // insert string in the begining
{
	int len = strlen(str);
	char *appended = new char[length + len + 1];
	memcpy(appended, str, len);
	memcpy(appended+len, data, length);
	delete[] data;
	length += len;
	appended[length] = 0;
  data = appended;
}

void String::operator +=(const char *str) // append string to the end
{
	int len = strlen(str);
	char *appended = new char[length + len + 1];
	memcpy(appended, data, length);
	memcpy(appended + length, str, len);
	delete[] data;
	length += len;
	appended[length] = 0;
  data = appended;
}

int String::ParseUrl(const String &parent, String &host, String &file, String &anhor, int &port) const
{
	char *pdata = data;
	int from = 0;
	int protocol = PROTOCOL_HTTP;

	//strip space
	for (int i = 0; i < length; i++)
	{
		unsigned char c = data[i];
		if (isspace(c))
		{
			for (int j = i; j < length-1; j++)
				data[j] = data[j+1];
		}
		else if (!isdigit(c) && !isalpha(c) && !isprint(c))
		{
			//printf("Bad url %s\n", data);
			return 0;
		}
	}

	if (!memcmp(data, "http://", 7))
	{
		from = 7;
		protocol = PROTOCOL_HTTP;
	}
	else if (!memcmp(data, "ftp://", 6))
	{
		protocol = PROTOCOL_FTP;
		from = 6;
	}
	else if (!memcmp(data, "https://", 8))
	{
		protocol = PROTOCOL_HTTPS;
		from = 8;
	}
	else if (!memcmp(data, "mailto:", 7))
	{
		protocol = PROTOCOL_MAIL;
		from = 5;
	}
	else if (length > 11 && !memcmp(data, "javascript:", 11))
	{
		protocol = PROTOCOL_JAVASCRIPT;
		from = 11;
	}
	if (from)
	{
		int to = from;
		for (; to < length; to++)
		{
			if (data[to] == '/' || data[to] == ':') // end host
				break;
		}

		int hTo = to-1;
		while (hTo>from && data[hTo] == '.')
			hTo--;
		hTo++;
		host.Set(data+from, hTo-from);
		if (data[from] == ':')
		{
			port = strtol(data+from, &pdata, 10);
			from = pdata - data;
		}
		else
			from = to;
		host.ToLower();
	}
  if (length - from > 0)
  {
  	char *pstr = data+from;
  	if (*pstr != '/' || strstr(pstr, "./")) // find relevant links
  	{
  		int len = length - from;
  		char *tdata;
			if (*pstr != '/' && parent) // if start of link not '/' adding parent page
			{
				int plen = parent.length;
				if (parent.data[plen-1] != '/') // if end of parent not '/' then find direcotry begining
				{
					while (plen > 0 && parent.data[plen-1] != '/')
						plen--;
				}
				tdata = new char[len+plen+1];
				memcpy(tdata, parent.data, plen);
				memcpy(tdata+plen, pstr, len);
				len += plen;
			}
			else // else simply copy page;
			{
				tdata = new char[len+1];
				memcpy(tdata, pstr, len);
			}

			int i = len-1;
			int k = len-1;
			int level = 0;
			while (i >= 0)
			{
 				if (i >= 2 && tdata[i-2] == '.' && tdata[i-1] == '.' && tdata[i] == '/') // if level up
 				{
 					i -= 3;
 					level++;
 					continue;
 				}
 				else if (i >= 1 && tdata[i-1] == '.' && tdata[i] == '/') // if this level
 				{
 					i-=2;
 					continue;
				}
				if (level)
				{
 					int j = i-1;
 					while (j >= 0 && tdata[j] != '/') // skip one level
 						j--;
 					i = j;
 					level--;
 					continue;
 				}

				tdata[k--] = tdata[i];
				i--;
			}
			if (level) // if level of link relevant more then url then this is bad url
			{
 					delete[] tdata;
 					return 0;
			}

			k++;
			if (tdata[k] == '/') // if url start from '/' skip '/'
			{
				k++;
				len--;
			}
			len -= k;
			if (len <= 0)
			{
				delete[] tdata;
				return 0;
			}

			pstr = tdata+k;
			*(pstr+len) = 0;

			if ((pdata = strchr(pstr, '#')) != NULL) // find anchor
			{
				file.Set(pstr, pdata - pstr);
				anhor.Set(pstr+1, len -  (pdata - pstr));
			}
			else
				file.Set(pstr, len);
			delete[] tdata;
			return protocol;
		}
		else
//			if (*(data+from) == '/') // if file start from '/' skip '/'
				from++;

		if ((pdata = strchr(data+from, '#')) != NULL) // find anchor
		{
			file.Set(data+from, pdata - data - from);
			anhor.Set(pdata+1, length -  (pdata - data) - 1);
		}
		else
			file.Set(data+from, length - from);

	}
	else
		file.SetNull();
	return protocol;
}

void String::ToLower()
{
	for (int i = 0; i < length; i++)
		data[i] = tolower((unsigned char)data[i]);
}

void String::ToUpper()
{
	for (int i = 0; i < length; i++)
		data[i] = toupper((unsigned char)data[i]);
}

const String &String::ToCData()
{
//	return *this;

	char *bufEnd = data + length;

	size_t count = 0;
	const char *p = data;
	while ((p = strnstr(p, "]]>", bufEnd - p))) {
		count++;
		p += 3;
	}

	if (count < 1)
		return *this;

	RString buf(length + count * 12 + 1);
	p = data;
	const char *p1 = data;
	while ((p = strnstr(p1, "]]>", bufEnd - p1))) {
		p += 2;
		buf.Add(p1, p - p1);
		buf.Add("]]><![CDATA[", 12);
		p1 = p;
	}
	buf.Add(p1, bufEnd - p1);
	Set(buf.Data(), buf.Length());

	return *this;
}

int String::IsAlphaNum() const
{
	for (int i = 0; i < length; i++)
		if (!isdigit(data[i]) && tolower((unsigned char)data[i]) == toupper((unsigned char)data[i]))
		{
			if ((data[i]=='\'' || data[i]=='-') && i+1 < length && i > 0) // catch word'word and word-word
				if (tolower((unsigned char)data[i-1]) != toupper((unsigned char)data[i-1]) &&
						tolower((unsigned char)data[i+1]) != toupper((unsigned char)data[i+1]))
						return 1;

			return 0;
		}
	return 1;
}

int String::ICmp(const char *str, const int len) const
{
	if (length != len)
		return 0;
	if (str)
		if (!strncasecmp(data, str, length))
			return 1;
	return 0;
}

int String::ICmp(const String &str) const
{
	if (length != str.GetLength())
		return 0;
	if (data)
		if (!strncasecmp(data, str.Data(), length))
			return 1;
	return 0;
}

int String::ICmp(const char *str) const
{
	if (length != strlen(str))
		return 0;
	if (data)
		if (!strncasecmp(data, str, length))
			return 1;
	return 0;
}

int String::IsValidHost() const
{
	if (!length || length < MIN_HOST_NAME_LENGTH)
		return 0;
	for (int i = 0; i < length; i++)
		if (data[i] != '.' && data[i] != '-' && !isalpha(data[i]) && !isdigit(data[i]))
			return 0;
	return 1;
}

int String::IsValidUserName() const
{
	for (int i = 0; i < length; i++)
		if (data[i] != '.' && data[i] != '-' && data[i] != '_'  && !isalpha(data[i]) && !isdigit(data[i]))
			return 0;
	return 1;
}

RString::RString(t_RStringSize startSize, t_RStringSize limitSize)
{
	maxSize = startSize;
	this->limitSize = limitSize;
	Init();
}

void RString::Init()
{
	length = 0;
	data = (char*)malloc(maxSize+1);
	data[0] = 0;
}

RString::~RString()
{
	free(data);
}

t_RStringSize RString::snaprintf(const char *fmt, ...)
{
	va_list ap;
  while (1)
  {
	  va_start(ap, fmt);
	  int size = maxSize - length;
    int res = vsnprintf (data + length, size, fmt, ap);
    if (CheckRes(res, size))
		{
			va_end(ap);
	   	return res;
		}
		va_end(ap);
  }
}

t_RStringSize RString::snprintf(const char *fmt, ...)
{
	length = 0;
	va_list ap;
  while (1)
  {
	  va_start(ap, fmt);
	  int size = maxSize - length;
    int res = vsnprintf (data + length, size, fmt, ap);
    if (CheckRes(res, size))
		{
			va_end(ap);
	   	return res;
		}
		va_end(ap);
  }
}
void RString::Add(char c)
{
	t_RStringSize oldlength = length;
	while (!CheckRes(1, maxSize-length))
		;
	data[oldlength] = c;
}
void RString::Set(t_RStringSize index, char c)
{
	if (index < length)
		data[index] = c;
	else
		Add(c);
}

char *RString::GetBuf(t_RStringSize len)
{
	if (!len)
		return NULL;
	t_RStringSize oldlength = length;
	while (!CheckRes(len, maxSize - length))
		;
	return data + oldlength;
}

void RString::Add(const char *str, t_RStringSize len)
{
	t_RStringSize oldlength = length;
	if (CheckRes(len, maxSize-length) || CheckRes(len, maxSize-length))
		memcpy(data + oldlength, str, len);
}

void RString::AddSlashed(const char *str, t_RStringSize strLen)
{
	if (strLen <= 0)
		return;

	t_RStringSize len = strLen * 2 + 1;
	t_RStringSize oldlength = length;
	if (CheckRes(len, maxSize-length) || CheckRes(len, maxSize-length))
	{
		char *curPos = data + oldlength;
		const char *pStr = str;
		for (t_RStringSize i = 0; i < strLen; i++)
		{
			if (*pStr == '"' || *pStr == '\''  || *pStr == '\\')
			{
				*curPos = '\\';
				curPos++;
			}
			*curPos = *pStr;
			curPos++;
			pStr++;
		}
		SetLength(curPos - data);
	}
}

void RString::AddStr(const char *str, int len)
{
	t_RStringSize oldlength = length;
	if (CheckRes(len, maxSize-length) || CheckRes(len, maxSize-length))
	{
		memcpy(data + oldlength, str, len);
		data[length] = 0;
	}
}


t_RStringSize RString::CheckRes(t_RStringSize res, int size)
{
	if (res > -1 && res < size)
  {
  	length += res;
   	return 1;
  }

	int newSize = maxSize;
  if (res > -1)    //* glibc 2.1
		newSize += res + 1; /* precisely what is needed */
	else    /* glibc 2.0 */
    newSize *= 2;  /* twice the old size */

  if (limitSize && (newSize >= limitSize-1))
  {
  	data[length] = 0;
  	throw ErrorLimit();
  }

	char *newdata =(char*)malloc(newSize+1);
	if (newdata)
	{
		memcpy(newdata, data, length);
		free(data);
		data = newdata;
		maxSize = newSize;
	}
	else
		throw Error();
  return 0;
}

void RString::SetMaxSize(t_RStringSize newSize)
{
	if (newSize != maxSize)
	{
		char *newdata =(char*)malloc(newSize+1);
		if (newdata == NULL)
  		throw Error();
		maxSize = newSize;
		if (length > maxSize)
			length = maxSize;
		memcpy(newdata, data, length);
		free(data);
 		data = newdata;
		data[length] = 0;
	}
}


void RString::addEscapedJSStr(const char *str, const int len, const char quoteChar)
{
	int startLength = length;
	char *pstart = GetBuf(len * 2 + 1);
	char *pnew = pstart;
	const char* pstr = str;
	const char* estr = str + len;
	unsigned char c;
	bool tag = false;
	static const char *SCR_BYTES = "scr";
	const char *curSCR = SCR_BYTES;
	while (pstr < estr)
	{
		c = *(pstr++);
		if (c == '<') // tag start
		{
			tag = true;
		}
		else if (c == '>')
			tag = false;

		if (tag && (tolower(c) == *curSCR))
		{
			*(pnew++) = c;

			curSCR++;
			if (!*curSCR) // find <scr
			{
				*(pnew++) = quoteChar;
				*(pnew++) = '+';
				*(pnew++) = quoteChar;
				curSCR = SCR_BYTES;
				tag = false;
			}
		}
		else
		{
			curSCR = SCR_BYTES;
			switch (c)
			{
				case '\n':
					*(pnew++) = '\\';
					*(pnew++) = 'n';
					break;
				case '\t':
					*(pnew++) = '\\';
					*(pnew++) = 't';
					break;
				case '\r':
					*(pnew++) = '\\';
					*(pnew++) = 'r';
					break;
				case '\"':
					*(pnew++) = '\\';
					*(pnew++) = '\"';
					break;
				case '\'':
					*(pnew++) = '\\';
					*(pnew++) = '\'';
					break;
				case '\\':
					*(pnew++) = '\\';
					*(pnew++) = '\\';
				break;
				default:
					*(pnew++) = c;
			};
		}
	}
	SetLength(startLength + (pnew - pstart));
}

void RString::addEscapedJSONStr(const char *str, const int len)
{
	int startLength = length;
	char *pstart = GetBuf(len * 2 + 1);
	char *pnew = pstart;
	const char* pstr = str;
	const char* estr = str + len;
	unsigned char c;
	while (pstr < estr)
	{
		c = *(pstr++);
		switch (c)
		{
			case '\n':
				*(pnew++) = '\\';
				*(pnew++) = 'n';
				break;
			case '\t':
				*(pnew++) = '\\';
				*(pnew++) = 't';
				break;
			case '\r':
				*(pnew++) = '\\';
				*(pnew++) = 'r';
				break;
			case '\"':
				*(pnew++) = '\\';
				*(pnew++) = '\"';
				break;
			case '\\':
				*(pnew++) = '\\';
				*(pnew++) = '\\';
			break;
			default:
				*(pnew++) = c;
		};
	}
	SetLength(startLength + (pnew - pstart));
}

int RString::AddFile(const char *fileName)
{
	FILE *fp = fopen(fileName, "r");
	if (!fp)
		return 0;

	fseek(fp, 0, SEEK_END);
	long len = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if (len)
	{
		t_RStringSize oldlength = length;
		while (!CheckRes(len, maxSize-length))
			;
		if (!fread(data + oldlength, len, 1, fp))
		{
			fclose(fp);
			length = oldlength;
			return 0;
		}
		fclose(fp);
		return 1;
	}
	fclose(fp);
	return 0;
}

void RString::ToLower()
{
	for (int i = 0; i < length; i++)
		data[i] = tolower((unsigned char)data[i]);
}

const RString &RString::ToCData()
{
//	return *this;

	char *bufEnd = data + length;

	size_t count = 0;
	const char *p = data;
	while ((p = strnstr(p, "]]>", bufEnd - p))) {
		count++;
		p += 3;
	}

	if (count < 1)
		return *this;

	t_RStringSize newSize = length + count * 12;
	char *newdata = (char *) malloc(newSize + 1);
	if (newdata == NULL)
		throw Error();

	p = data;
	const char *p1 = data;
	char *p2 = newdata;
	while ((p = strnstr(p1, "]]>", bufEnd - p1))) {
		p += 2;
		length = p - p1;
		memcpy(p2, p1, length);
		p2 += length;
		memcpy(p2, "]]><![CDATA[", 12);
		p2 += 12;
		p1 = p;
	}
	memcpy(p2, p1, bufEnd - p1);

	maxSize = newSize;
	length = maxSize;
	free(data);
	data = newdata;
	data[length] = 0;

	return *this;
}

#ifdef LINUX
char *strnstr(const char *s, const char *find, size_t slen)
{
  char c, sc;
  size_t len;

  if ((c = *find++) != '\0')
	{
		len = strlen(find);
		do
		{
			do
			{
				if ((sc = *s++) == '\0' || slen-- < 1)
					return (NULL);
			} while (sc != c);
      if (len > slen)
        return (NULL);
    } while (strncmp(s, find, len) != 0);
    s--;
  }
	return ((char *)s);
}
#endif
