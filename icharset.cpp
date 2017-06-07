#include <stdlib.h>
#include <string.h>
#include <iconv.h>
#include <ctype.h>
#include <errno.h>

#include "string.hpp"
#include "xtypes.hpp"
#include "tools.hpp"
#include "icharset.hpp"

namespace lb
{

const char *charsets_name[MAX_ENCODE] = 
{
	"",
	"WINDOWS-1251",
	"KOI8-U",
	"KOI8-R",
	"KOI8-RU",
	"UTF-8",
	"MACCYRILLIC",
	"US",
	"ISO-8859-5",
	"ISO-8859-1",
};

const AssocPair charsets[] = 
{
	{CONTENT_CHARSET_WIN1251, "win1251"},
	{CONTENT_CHARSET_WIN1251, "win-1251"},
	{CONTENT_CHARSET_WIN1251, "Windows1251"},
	{CONTENT_CHARSET_WIN1251, "windows-1251"},
	{CONTENT_CHARSET_WIN1251, "CP1251"},
	{CONTENT_CHARSET_WIN1251, "cp-1251"},
	{CONTENT_CHARSET_WIN1251, "MS-CYRL"},
	{CONTENT_CHARSET_KOI8U, 	"koi8-u"},
	{CONTENT_CHARSET_KOI8R, 	"koi8-r"},
	{CONTENT_CHARSET_KOI8RU,	"koi8-ru"},
	{CONTENT_CHARSET_UTF8, 		"UTF-8"},
	{CONTENT_CHARSET_UTF8, 		"UTF8"},
	{CONTENT_CHARSET_US,  		"US"},
	{CONTENT_CHARSET_US, 			"US-ASCII"},
	{CONTENT_CHARSET_US, 			"ANSI_X3.4-1968"},
	{CONTENT_CHARSET_US, 			"ASCII"},
	{CONTENT_CHARSET_US,			"CP367"},
	{CONTENT_CHARSET_US,			"IBM367"},
	{CONTENT_CHARSET_US,			"CSASCII"},
	{CONTENT_CHARSET_MAC, 		"MAC"},
	{CONTENT_CHARSET_MAC,			"MACINTOSH"},
	{CONTENT_CHARSET_MAC, 		"MACROMAN"},
	{CONTENT_CHARSET_MAC, 		"CSMACINTOSH"},
	{CONTENT_CHARSET_ISO_8859_5, "ISO-8859-5"},
	{CONTENT_CHARSET_ISO_8859_1, "ISO-8859-1"},
	{0, NULL}
};

iconv_t iconv_tables[MAX_ENCODE];
const int MAX_LETER	= 3;

int IConvert(char *str, int len, char *outbuf, int outbuflen, int from, int to)
{
	iconv_t from_table = iconv_open (charsets_name[to], charsets_name[from]);
	if (from_table == (iconv_t)-1)
	{
		printf("Cannot open iconv from %s to %s\n", charsets_name[CONTENT_CHARSET_KOI8U], charsets_name[from]); 
		return 0;
	}

#ifndef LINUX
	const int argument = 1;
	iconvctl (from_table, ICONV_SET_DISCARD_ILSEQ, (void*) &argument);
#endif


 	size_t inlen = len;
 	size_t outlen = outbuflen;
//#ifndef BSD
 	char *ibuf = str;
//#else
//	const char *ibuf = str;
//#endif

  char *obuf = outbuf;
  while (inlen > 0)
	{
		int res = iconv (from_table, &ibuf, &inlen, &obuf, &outlen);
		if (res == 0)
			break;
		else if (res == -1 && (errno != EILSEQ && errno != EINVAL))
		{
			if (res == E2BIG)
				printf("Erorr outbuf is too small %s (%s)\n", strerror(errno), charsets_name[from]);
			else
				printf("Erorr %s (%s)\n", strerror(errno), charsets_name[from]);
  		iconv_close(from_table);
  		return 0;
		}
		if (inlen > 0)
		{
			ibuf++;
			inlen--;
		}
	}
  *obuf = 0;
	iconv_close(from_table);
	return obuf - outbuf;
}

int IConvert(char *str, int len, int from, int to)
{
	iconv_t from_table = iconv_open (charsets_name[to], charsets_name[from]);
	if (from_table == (iconv_t)-1)
	{
		printf("Cannot open iconv from %s to %s\n", charsets_name[CONTENT_CHARSET_KOI8U], charsets_name[from]); 
		return 0;
	}

#ifndef LINUX
//	const int argument = 1;	
//	iconvctl (from_table, ICONV_SET_DISCARD_ILSEQ, (void*) &argument);
#endif


	int res = 0;
	char *outstr = str;
 	size_t inlen = len;
 	size_t outlen = len;
//#ifndef BSD
  char *ibuf = str;
//#else
//	const char *ibuf = str;
//#endif
	char *obuf = outstr;
	while (inlen > 0)
	{
		res = iconv (from_table, &ibuf, &inlen, &obuf, &outlen);
		if (res == 0)
			break;
		else if (res == -1 && (errno != EILSEQ && errno != EINVAL))
		{
  		iconv_close(from_table);
			printf("Erorr %s (%s)\n", strerror(errno), charsets_name[from]);
  		return 0;
		}
		if (inlen > 0)
		{
			ibuf++;
			inlen--;
		}
	}
	*obuf = 0;
	iconv_close(from_table);
	return obuf - str;
}
int IConvertToCP1251Translit(char *buf, size_t len, const char *from)
{
    if (len==0)
            return 1;
    iconv_t iconv_cd;
    if ((iconv_cd = iconv_open("CP1251//TRANSLIT", from )) == (iconv_t) -1) {
        printf("Cannot open iconv from %s to CP1251//TRANSLIT\n", from);
        return 0;
    }
//#ifdef BSD
//	const char *inbuf = buf;
//#else
	char *inbuf = buf;
//#endif
        RString outBuf;
        
        char *tmpBuf = outBuf.GetBuf(len);
        char *outbuf = tmpBuf;
	size_t inlen = len;
        size_t outlen = len;
	size_t res = 0;

	while (inlen > 0 && outlen > 0) {
            res = iconv(iconv_cd, &inbuf, &inlen, &outbuf, &outlen);
                if (res == 0)
                    break;
		if (res == (size_t) (-1)) {
                    if (errno != EILSEQ && errno != EINVAL) {
                        if (errno == E2BIG) // not enough space
                        {
                            iconv_close(iconv_cd);
                            outbuf = tmpBuf;
                            inbuf = buf;
                            if ((iconv_cd = iconv_open("CP1251", from )) == (iconv_t) -1) {
                                printf("Cannot open iconv from %s to CP1251 \n", from);
                                return 0;
                            }
                            inlen = len;
                            outlen = len;
                            continue;
                        }
                        iconv_close(iconv_cd);
                        printf("Error %s (%s)\n", strerror(errno), from);
                        return 0;
                    } else if (errno == EILSEQ || errno == EINVAL) { //couldn't convert character
                        inbuf+=1;
                        inlen-=1;
                    } else{
                        iconv_close(iconv_cd);
			printf("Error %s \n", strerror(errno));
			return 0;
                    }
                }
	}
	iconv_close(iconv_cd);
	*outbuf = '\0';
        strcpy(buf,tmpBuf);
	return 1;
}
int IConvert(char *buf, size_t len, const char *from, const char *to)
{
	iconv_t iconv_cd;
	if ((iconv_cd = iconv_open(to, from)) == (iconv_t) -1) {
		printf("Cannot open iconv from %s to %s\n", from, to);
		return 0;
	}
        
//#ifdef BSD
//	const char *inbuf = buf;
//#else
	char *inbuf = buf;
//#endif
        
        char *outbuf = buf;
	size_t inlen = len;
        size_t outlen = len;
	size_t res = 0;

	while (inlen > 0 && outlen > 0) {
		res = iconv(iconv_cd, &inbuf, &inlen, &outbuf, &outlen);
		if (res == 0)
			break;

		if (res == (size_t) (-1)) {
			if (errno != EILSEQ && errno != EINVAL) {
				iconv_close(iconv_cd);
				*outbuf = '\0';
				printf("Erorr %s (%s)\n", strerror(errno), from);
				return 0;
			} else if (inbuf < outbuf) {
				iconv_close(iconv_cd);
				*outbuf = '\0';
				printf("Erorr %s (inbuf < outbuf)\n", strerror(errno));
				return 0;
			}
		}
		if (inlen > 0 && outlen > 0) {
			*outbuf++ = *inbuf++;
			inlen--;
			outlen--;
		}
	}
	iconv_close(iconv_cd);
	*outbuf = '\0';
	return 1;
}

};
