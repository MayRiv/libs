#pragma once

extern const char wordDelimeters[];
extern const char quotes[];
extern const char alphanum[];

inline int IsWordDelimiter(unsigned char c)
{
	return wordDelimeters[c];
}

inline int IsQuote(unsigned char c)
{
	return quotes[c];
}

inline int IsNoCyr(unsigned char c)
{
	return alphanum[c];
}


