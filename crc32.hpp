#pragma once
/*
 * This code implements the AUTODIN II polynomial
 * The variable corresponding to the macro argument "crc" should
 * be an unsigned long.
 * Oroginal code  by Spencer Garrett <srg@quick.com>
 */

/* generated using the AUTODIN II polynomial
 *	x^32 + x^26 + x^23 + x^22 + x^16 +
 *	x^12 + x^11 + x^10 + x^8 + x^7 + x^5 + x^4 + x^2 + x^1 + 1
 */

#include "xtypes.hpp"

namespace lb
{
	extern Word32 GetCRC32(const char *str, const Word32 len);
	extern Word32 GetCRC32C(Word32 crc, const char *str, const Word32 len);
};
