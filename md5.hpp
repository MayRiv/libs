#pragma once

#include <openssl/evp.h>
#include "string.hpp"

namespace lb
{
	class MD5
	{
	public:
		MD5();
		~MD5();
		bool CalcMD5(const char *data, const int length, RString &result);
	private:
		EVP_MD_CTX _mdctx;
		union MdValueUnion
		{
			struct 
			{
				Word64 b1;
				Word64 b2;
				unsigned char other[EVP_MAX_MD_SIZE - sizeof(Word64) * 2];
			} mdValue;
			unsigned char value[EVP_MAX_MD_SIZE];
		} _md_value;
	};
}
