#pragma once

#include <sys/types.h>

namespace lb
{
	typedef __uint8_t Byte;
	typedef __uint16_t Word16;
	typedef __uint32_t Word32;
	typedef __uint64_t Word64;
	typedef __int64_t Int64;


	struct AssocPair
	{
		int key;
		const char *value;
	};
	struct RAssocPair
	{
		const char *value;
		int key;
	};
};

