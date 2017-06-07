#pragma once

#include <string>
#include <map>

namespace lb
{
	char *base64_decode_str(char *result, const char *in, size_t len);
	char *base64_decode(char *str_out, const char *str_in, size_t length/*, bool strict = false*/);
	char *convert_cyr_string(char *str, int length, char from, char to);
	char convert_cyr_char(char c, char from, char to);
	//char encodeHeader(char *result, char *header, int len);
	//char * quoted_printable_decode(char *str_out, const char *str_in);
	char * quoted_printable_decode(char *str_out, const char *str_in, const int len, const char ch = '=');
	
	bool checkXlatChars(const char *str, size_t len, char xlat[]);
	size_t decode_mime_header(const char *from, class RString &to, size_t len, const char *additionCharset = NULL, const char *escapeChars = NULL);


	void InitDetectCode();
	int DetectCharset(char *str, int len, char from = 0);
	extern const unsigned char koi8_tolower[256];
	extern char *decodeTable[256];

	void transliterate(const char *in, int inlen, char *out, int outlen);

	void decode_html_entities(char *buf, const char *charset = 0);
	typedef std::map<std::string, unsigned int> THtmlEntitiesTable;
	const THtmlEntitiesTable *get_html_entities_table();
};

