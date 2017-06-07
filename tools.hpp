#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <set>
#include <ctype.h>
#include <fcntl.h>
#include <time.h>
#include "xtypes.hpp"

#define SWITCH_PTR(type, ptr1, ptr2) type *t; t=ptr1; ptr1=ptr2;ptr2=t;

namespace lb
{
	Word32 ToTStamp(Word32 &tDay, Word64 &tStamp);
	Word32 FileSize(FILE *file);
	off_t FileSize(const char *path);
	void ToTStamp(time_t currentTime, Word32 &tDay, Word64 &tStamp);
	void ToTStamp(struct tm *localTm, Word32 &tDay, Word64 &tStamp);
	void GetExpiresDate(Word32 secs, char *buffer_str);
	void FormExpiresDate(Word32 secs, class RString &buf);

	static const Word32 MAX_HTTP_DATE_SIZE = sizeof("Fri, 22 Oct 2010 19:31:20 GMT") + 2;
	void GetHTTPDate(const time_t t, RString &buf);
	char *FindDomain(char *email);
	bool GetDomainFromURL(const char *url, class String &domain);

	//Word16 GetOsBrws(char *userAgent);
	//int ParseOs(char *userAgent);
	char *GetAssocPairValue(int key, const AssocPair *pairs);
	int GetAssocPairKey(const char *value, const AssocPair *pairs);
	int GetAssocPairKey(const char *value, const int len, const AssocPair *pairs);
	int GetAssocPairZeroKey(const char *value, const AssocPair *pairs);

	int ParseHeader(const char *header, char *&value, const int len, char del, const AssocPair *pairs);
	int CheckHeader(const char *header, const char *reqHeader, char del, char *&value);

	void ParseEmail(char *email, char *name, char *address);
	int FileExists(const char *fileName);
	int Touch(const char *fileName, const int defaultFileMode = S_IRUSR | S_IWUSR);

	void dmybetween(const int& d2, const int& m2, const int& y2, const int& d1, const int& m1, const int& y1, int& d, int& m, int& y);

	inline Word32 GetFullMask(Byte mask)
	{
		Word32 fullMask = 1<<31;
		for (int i = 0; i < mask-1; i++)
			fullMask |= fullMask>>1;
		return fullMask;
	}
	inline int IpInNetFullMask(Word32 ip, Word32 net, Word32 fullMask)
	{
		if ((ip & fullMask) == net)
			return 1;
		else
			return 0;
	}

	inline int IpInNet(Word32 ip, Word32 net, Byte mask)
	{
		return IpInNetFullMask(ip, net, GetFullMask(mask));
	}

	template <class R> R StrToType(const char *src, char *&next_char, int base)
	{
	}

	template <>
	inline int StrToType<int>(const char *src, char *&next_char, int base)
	{
		return strtol(src, &next_char, base);
	}

	template <>
	inline Word32 StrToType<Word32>(const char *src, char *&next_char, int base)
	{
		return strtol(src, &next_char, base);
	}
	template <>
	inline Word64 StrToType<Word64>(const char *src, char *&next_char, int base)
	{
		return strtoll(src, &next_char, base);
	}
	template <>
	inline float StrToType<float>(const char *src, char *&next_char, int base)
	{
		return strtof(src, &next_char);
	}
	template <>
	inline double StrToType<double>(const char *src, char *&next_char, int base)
	{
		return strtod(src, &next_char);
	}

	template <class D> int explode(const char *str, std::vector<D> &list, char delimiter = ',', int base = 10)
	{
		char *nextChar = (char *) str;
		char *end = nextChar + strlen(str);
		D current;
		while (nextChar < end) {
			current = StrToType<D>(nextChar, nextChar, base);
			list.push_back(current);
			if(*nextChar++ != delimiter)
				break;
		}

		return list.size();
	}

	template <class D> int explode(const char *str, const int len, std::vector<D> &list, char delimiter = ',', int base = 10)
	{
		char *nextChar = (char *) str;
		char *end = nextChar + len;
		D current;
		while (nextChar < end) {
			current = StrToType<D>(nextChar, nextChar, base);
			list.push_back(current);
			if(*nextChar++ != delimiter)
				break;
		}

		return list.size();
	}

	template <class D> int explode(const char *str, std::set<D> &elemSet, char delimiter = ',', int base = 10)
	{
		char *nextChar = (char *) str;
		char *end = nextChar + strlen(str);
		D current;
		while (nextChar < end) {
			current = StrToType<D>(nextChar, nextChar, base);
			elemSet.insert(current);
			if(*nextChar++ != delimiter)
				break;
		}

		return elemSet.size();
	}



	template<class D>
	int explode(const char* src, const char delim, D *arr, const int maxElement, int base = 10)
	{
		int i;
		char* next_char = (char*)src;
		D currentInt;

		for(i = 0; '\0' != *next_char && i < maxElement; )
		{
			currentInt =  StrToType<D>(next_char, next_char, base); //strtoll(next_char, &next_char, 10);
			arr[i++] = (D)currentInt;
			if(*next_char != delim)
				break;
			else
				next_char++;
		}
		return i;
	}

	Word64 binImplode(char* src, char delim);

	template<class D> void SwitchPtr(D *&src, D *&dst)
	{
		D *t  = src;
		src = dst;
		dst = t;
	}


	/*
	implode test

	#include <stdio.h>
	#include <stdlib.h>
	int main()
	{
		char *imp[] = {"1976, 2278, 2550, 3363, 3434, 3494, 3496, 3664, 3714, 4181, 4334, 4778, 4784, 4890, 4925", ",asdf,23,354", ",0,2,3","363, 3434, 3494, 3496, 3664, 3714, 4181, 4"};
	#define INTCNT 8

		int arr[INTCNT];
	#define STRCNT sizeof(imp)/sizeof(imp[0])
		for (int i=0;i<STRCNT ;++i )
		{
			int cnt = implode(imp[i],',',arr,INTCNT);
			printf("\n%s\ncnt %d\n", imp[i], cnt);
			for (int j=0;j<cnt ;j++ )
			{
				printf("%d , ",arr[j]);
			}
		} 
		return 0;
	}
	*/

	inline char *strncasestr(const char *s, const char *find, size_t slen)
	{
		size_t paternLen = strlen(find);
		if (slen < paternLen)
			return 0;

		const char *end = s + slen - paternLen;
		const char *p = s;
		int res = -1;
		while (*p && p <= end && (res = strncasecmp(p, find, paternLen)))
			p++;

		return (char *) (res ? 0 : p);
	}

#ifndef BSD
	inline char *strnstr(const char *s, const char *find, size_t slen)
	{
		size_t paternLen = strlen(find);
		if (slen < paternLen)
			return 0;

		const char *end = s + slen - paternLen;
		const char *p = s;
		int res = -1;
		while (*p && p <= end && (res = memcmp(p, find, paternLen)))
			p++;

		return (char *) (res ? 0 : p);

		//size_t len = strlen(s);
		//if (slen < len)
		//	len = slen;
		//if (len < paternLen)
		//	return 0;

		//const char *end = s + len - paternLen;
		//const char *p = s;
		//int res;
		//do {
		//	res = memcmp(p, find, paternLen);
		//} while (res && p++ < end);

		//return (char *) (res ? 0 : p);

		//char c, sc;
		//size_t len;

		//if ((c = *find++) != '\0') {
		//				len = strlen(find);
		//				do {
		//								do {
		//												if ((sc = *s++) == '\0' || slen-- < 1)
		//																return (NULL);
		//								} while (sc != c);
		//								if (len > slen)
		//												return (NULL);
		//				} while (strncmp(s, find, len) != 0);
		//				s--;
		//}
		//return ((char *)s);
	}
#endif

	size_t strip_html_tags(char *text, const char *const *allowTags = 0, const char *const *denyContainer = 0);
	size_t compress_spaces(char *text);

	long CombinedLCGRandom();
	void SeedRandom( );

	inline size_t string_chsum(const char *str)
	{
		size_t h = 0;
		for (; *str; str++) {
			h = *str + (h << 5) + (h << 12) - h;
		}

		return h;
	}

	inline Word64 string_chsum_64(const char *str)
	{
		Word64 h = 0;
		for (; *str; str++) {
			h = *str + (h << 10) + (h << 24) - h;
		}
		return h;
	}
	inline Word64 string_chsum_64(const char *str, size_t len, Word64 h = 0)
	{
		for (register size_t i = 0; i < len; i++) {
			h = str[i] + (h << 10) + (h << 24) - h;
		}
		return h;
	}

	inline size_t string_chsum(const char *str, size_t len, size_t h = 0)
	{
		for (register size_t i = 0; i < len; i++) {
			h = str[i] + (h << 5) + (h << 12) - h;
		}

		return h;
	}
	inline size_t string_case_chsum(const char *str, size_t len, size_t h = 0)
	{
		for (register size_t i = 0; i < len; i++) {
			h = tolower((unsigned char)str[i]) + (h << 5) + (h << 12) - h;
		}

		return h;
	}
	inline int SpaceSpn(char *body)
	{
		int i = 0;
		while (isspace(*body))
		{
			body++;
			i++;
		}
		return i;
	}

	struct LoadAvgData
	{
		double avg1;
		double avg5;
		double avg15;
	};
	bool GetLoadAvg(LoadAvgData &avg);
	unsigned char *GetCharTableFromString(const char *str);
	inline int Word32BitCount(unsigned int n)
	{
// This is for 32 bit numbers. Need to adjust for 64 bits
		Word32 tmp;
		tmp = n - ((n >> 1) & 033333333333) - ((n >> 2) & 011111111111);
		return ((tmp + (tmp >> 3)) & 030707070707) % 63;
	}

#define PBC_TWO(c)     ((Word64)0x1u << (c))
#define PBC_MASK(c) \
  (((Word64)(-1)) / (PBC_TWO(PBC_TWO(c)) + 1u))
#define PBC_COUNT(x,c) \
  ((x) & PBC_MASK(c)) + (((x) >> (PBC_TWO(c))) & PBC_MASK(c))

	inline int ParalelWord64BitCount (Word64 n)  {
		 n = PBC_COUNT(n, 0) ;
		 n = PBC_COUNT(n, 1) ;
		 n = PBC_COUNT(n, 2) ;
		 n = PBC_COUNT(n, 3) ;
		 n = PBC_COUNT(n, 4) ;
		 n = PBC_COUNT(n, 5) ; 
		 return n ;
	}

	const char * const GetHostName();
	bool GetDiskSize(const char *path, Word64 &freeDiskSpace, Word64 &allDiskSpace);
};

