#pragma once

#include <ctype.h>
#include "xtypes.hpp"
#include "charset.hpp"
#include "chars.hpp"
#include <string.h>
#include <strings.h>


namespace lb
{
	enum EProtocol
	{
		PROTOCOL_HTTP = 1,
		PROTOCOL_HTTPS,
		PROTOCOL_FTP,
		PROTOCOL_MAIL,
		PROTOCOL_JAVASCRIPT
	};

	const int MIN_HOST_NAME_LENGTH = 4;

	class String
	{
	public:
		String()
		{
			data = NULL;
			length = 0;
		}
		String(const char *str);
		String(const char *str, int leng);
		String(const String &str);
		String(const class RString &str);
		~String();
		void Set(const char *str);
		void Set(const char *str, const int leng);
		void Set(const String &str);
		void SetEscapeChars(const char *str);
		void SetEscapeChars(const char *str, const int len);
		void operator=(const String &src)
		{
			Set(src);
		}
		void SetNull() // empty string
		{
			delete[] data;
			data = NULL;
			length = 0;
		}
		int PartOf(const String &s, int leng) const
		{
			if (length >= leng)
				if (!memcmp(data, s.data, leng))
					return 1;
			return 0;
		}
		int PartOf(const char *str, int leng)
		{
			if (length >= leng)
				if (!memcmp(data, str, leng))
					return 1;
			return 0;
		}

		int PartIOf(const String &s, int leng) const
		{
			if (length >= leng)
				if (!strncasecmp(data, s.data, leng))
					return 1;
			return 0;
		}

		int PartIOf(const char *str, int leng) const
		{
			if (length >= leng)
				if (!strncasecmp(data, str, leng))
					return 1;
			return 0;
		}

		int CopyTo(char *dst, int maxLen) const // copy string object to dst if reach maxLen cat str
		{
			Word16 len = length;
			if (len)
			{
				if (len >= maxLen)
					len = maxLen - 1;
				memcpy(dst, data, len);
				dst[len] = 0;
			}
			else
				dst[0] = 0;
			return len;
		}

		int CopyPartTo(String &dst, int from, int len)
		{
			if (len)
			{
			 if (from+len > length)
				len = length-from;
			 if (len <= 0) // if from larger then length of source
			 {
				 delete[] dst.data;
				 dst.data = NULL;
				 dst.length = 0;
				 return 0;
			 }
			 delete[] dst.data;
			 dst.data = new char[len+1]; // 1 byte to terminate symbol
			 dst.length = len;
			 memcpy(dst.data, data+from, len);
			 dst.data[len] = 0;
			}
			else
			{
				delete[] dst.data;
				dst.data = NULL;
				dst.length = 0;
			}
			return len;
		}
		void SetUnescapeChars(const char *str, int leng); // Set string with unescape special chars
		void UnescapeChars();
		Word16 GetLength() const {
			return length;
		}
		char *c_str()	const
		{
			if (data)
				return data;
			else
				return (char *)"";
		}

		char *Data() const
		{
			if (data)
				return data;
			else
				return (char *)"";
		}
		int In(const char *str) const; // String include template (str)
		int InCase(const char *templ) const;
		int GetNextWord(int from, char *word, int maxWordLength);

		int GetWord32(const char *param, int len, Word32 &val); // convert structure param=Word32 to value

		int ICmp(const char *str) const;
		int ICmp(const char *str, const int len) const;
		int ICmp(const String &str) const;

		int Cmp(const char *str)
		{
			if (length != strlen(str))
				return 0;
			if (data)
				if (!memcmp(data, str, length))
					return 1;
			return 0;
		}

		int Cmp(String *str) const
		{
			if (!length || length != str->length)
				return 0;
			if (!memcmp(data, str->data, length))
				return 1;
			return 0;
		}
		int Cmp(const String &str) const
		{
			if (/*!length || */length != str.length)
				return 0;
			if (!memcmp(data, str.data, length))
				return 1;
			return 0;
		}
		int convert_cyr_string(char from, char to)
		{
			if (data && length && (tolower(from) != tolower(to)))
			{
				lb::convert_cyr_string(data, length, from, to);
				return 1;
			}
			return 0;
		}
		int DetectCharset(char from = 0)
		{
			if (data && length)
				return lb::DetectCharset(data, length, from);
			else
				return 0;
		}
		int IsCyrText()
		{
			for (int i = 0; i < length; i++)
				if (!IsNoCyr(data[i]))
					return 1;
			return 0;
		}

		int IsValidHost() const;
		int IsValidUserName() const;
		int IsAlphaNum() const;
		int IsNumber() const
		{
			for (int i = 0; i < length; i++)
				if (!isdigit(data[i]))
					return 0;
			return 1;
		}
		const char operator[](int index) const
		{
			if (index < length)
				return data[index];
			return 0;
		}


		void Replace(const char from, const char to);
		void Replace(const char *from, const char *to);
		int StrReplace(const char *from, const char *to);
		Word32 Hash32() const;
		operator bool() const
		{
			return data && length;
		}
		int ToInt() const;
		int ToOInt() const;
	/*	operator int() const
		{
			if (length)
					return atoi(data);
			else
					return 0;
		}
	*/
		void operator ^=(const char *str); // insert string in the begining
		void operator +=(const char *str); // append string to the end
		bool operator !=(const String &str)
		{
			return !str.Cmp(this);
		}
		int Trim(int len);
		void WordWrap(int len, char braker);
		void StripTags();
		void StripHtmlSpecial();
		void RemoveDublicate(int maxlen, char badsymbol, char braker = '\n');
		void ToLower();
		void ToUpper();
		const String &ToCData();
		int ParseUrl(const String &parent, String &host, String &file, String &anhor, int &port) const;
		void ReplaceWrongXmlChars(const char replace = ' ');
		void ReplaceNotAlphaNum(const char replace = ' ');
	protected:
		static int UnescapeChars(const char *src, int srcLen, char *dst);
		char *data;
		Word16 length;
		static const char HexValue[256]; // static table for unescape special chars
	};


	typedef int t_RStringSize;
	const t_RStringSize MIN_RSTRING_SIZE = 128;

	class RString
	{
		friend class String;
	public:
		RString()
		{
			limitSize = 0;
			maxSize = MIN_RSTRING_SIZE;
			Init();
		}
		void UnAttach()
		{
			data = NULL;
		}
		RString(t_RStringSize startSize, t_RStringSize limitSize = 0);
		~RString();
		t_RStringSize snaprintf(const char *fmt, ...);
		t_RStringSize snprintf(const char *fmt, ...);
		void Add(const char *str, t_RStringSize len);
		void AddSlashed(const char *str, t_RStringSize len);
		void AddStr(const char *str, int len);
		void AddStr(const char *str)
		{
			return AddStr(str, strlen(str));
		}
		void Add(char c);
		void AddEndZero()
		{
			data[length] = 0;
		}
		void Set(t_RStringSize index, char c);
		void BackSpace()
		{
			if (length)
				length--;
			data[length] = 0;
		}
		void Null()
		{
			length = 0;
			data[length] = 0;
		}
		const t_RStringSize Length() const 
		{ 
			return length; 
		};
		t_RStringSize MaxSize() { return maxSize; };
		void SetLength(t_RStringSize length)
		{
			if (length < maxSize)
			{
				data[length] = 0;
				this->length = length;
			}
		}
		char *GetBuf(t_RStringSize len);
		void SetMaxSize(t_RStringSize newSize);
		int AddFile(const char *fileName);
		void ToLower();
		const RString &ToCData();
		operator char*() const { return data; };
		const char *Data() const { return data; };
		char operator[](int index)
		{
			if (index < length)
				return data[index];
			return 0;
		}
		class Error {};
		class ErrorLimit : public Error {};

		void addEscapedJSONStr(const char *str, const int len);
		void addEscapedJSStr(const char *str, const int len, const char quoteChar);
	protected:
		void Init();
		t_RStringSize CheckRes(t_RStringSize res, int size);

		t_RStringSize maxSize;
		t_RStringSize length;
		t_RStringSize limitSize;
		char *data;
	private:
		RString(const RString &s)
		{
		}
	};

	#ifdef LINUX
	extern char *strnstr(const char *s, const char *find, size_t slen);
	#endif
};

