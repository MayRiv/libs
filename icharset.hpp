#pragma once

namespace lb
{
	extern const struct AssocPair charsets[];
	extern const char *charsets_name[];

	enum ECharset
	{
		CONTENT_CHARSET_ENGLISH			= 0,
		CONTENT_CHARSET_WIN1251 		= 1,
		CONTENT_CHARSET_KOI8U,
		CONTENT_CHARSET_KOI8R,
		CONTENT_CHARSET_KOI8RU,
		CONTENT_CHARSET_UTF8,
		CONTENT_CHARSET_MAC,
		CONTENT_CHARSET_US,
		CONTENT_CHARSET_ISO_8859_5,
		CONTENT_CHARSET_ISO_8859_1,
		CONTENT_CHARSET_UNKNOWN = 255,
	};

	const int MAX_ENCODE								=	10;

	int IConvert(char *str, int len, int from, int to);
	int IConvert(char *str, int len, char *outbuf, int outlen, int from, int to);
	int IConvert(char *buf, size_t len, const char *from, const char *to);
        int IConvertToCP1251Translit(char *buf, size_t len, const char *from);
};

