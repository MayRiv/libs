#include "md5.hpp"

using namespace lb;

MD5::MD5()
{
	EVP_MD_CTX_init(&_mdctx);
}

MD5::~MD5()
{
	EVP_MD_CTX_cleanup(&_mdctx);
}

/*unsigned long Swap(unsigned long value)
{
    return ((value & 0xFF000000) >> 24) |
            (value & 0x00FF0000) >>  8) |
            (value & 0x0000FF00) <<  8) |
            (value & 0x000000FF) << 24);
}*/
inline Word64 Swap(Word64 value)
{
	return ( ((value & (Word64)0xFF00000000000000ll) >> 56) | 
					 ((value & (Word64)0x00FF000000000000ll) >> 40) | 	
					 ((value & (Word64)0x0000FF0000000000ll) >> 24) | 	
					 ((value & (Word64)0x000000FF00000000ll) >> 8) | 	
					 ((value & (Word64)0x00000000FF000000ll) << 8) | 	
					 ((value & (Word64)0x0000000000FF0000ll) << 24) | 	
					 ((value & (Word64)0x000000000000FF00ll) << 40) | 	
					 ((value & (Word64)0x00000000000000FFll) << 56)); 	
}

bool MD5::CalcMD5(const char *data, const int length, RString &result)
{
	
	if (!EVP_DigestInit_ex(&_mdctx, EVP_md5(), NULL))
		return false;

	if (!EVP_DigestUpdate(&_mdctx, data, length))
		return false;

	unsigned int md_len;
	if (!EVP_DigestFinal_ex(&_mdctx, (unsigned char*)_md_value.value, &md_len))
		return false;

	result.snaprintf("%016llx%016llx", Swap(_md_value.mdValue.b1), Swap(_md_value.mdValue.b2));
	return true;
}
