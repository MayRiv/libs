#pragma once

#include <string>
#include <vector>
#include "hashmap.hpp"
#include "tools.hpp"
#include "icharset.hpp"


namespace lb
{
	class DetectCharset
	{
	public:
		static void Init();
		static ECharset CheckCharset(const char *buf, const int size);
	private:
		static const RAssocPair _statsKoi[];
		static const RAssocPair _statsUtf8[];
		static const RAssocPair _statsWin[];

		class Charset
		{
		public:
			Charset(const ECharset charset, const RAssocPair *statsTable, char *charsetLocale);
			Word32 CheckCharset(const char *buf, const int size);
			ECharset GetCharset() const
			{
				return _charset;
			}
		private:
			Word32 _CheckSum(const char *buf, int len) // plain checksum
			{
				Word32 sum = 0;
				for (int i = 0; i < len -1; i++)
				{
					sum |= (Byte)buf[i];
					sum <<= 8;
				}
				sum |= (Byte)buf[len - 1];
				return sum;
			}

			typedef unordered_map<Word32, Word32> TKeyList;
			ECharset _charset;
			Word32 _curMask;
			TKeyList _hashTable;
			char *_charsetLocale;
		};
		typedef std::vector<Charset*> TCharsetList;
		static TCharsetList _charsets;
	};
};
