#pragma once

#include <limits.h>
#include "string.hpp"

namespace lb
{
bool ConvertPunyCode(String &url);

#if UINT_MAX >= (1 << 26) - 1
typedef unsigned int punycode_uint;
#else
typedef unsigned long punycode_uint;
#endif

enum punycode_status {
  punycode_success,
  punycode_bad_input,   /* Input is invalid.                       */
  punycode_big_output,  /* Output would exceed the space provided. */
  punycode_overflow     /* Input needs wider integers to process.  */
};

enum punycode_status punycode_encode(
  punycode_uint input_length,
  const punycode_uint input[],
  const unsigned char case_flags[],
	RString &outputBuf);

enum punycode_status punycode_encode(
  punycode_uint input_length,
  const punycode_uint input[],
  const unsigned char case_flags[],
  punycode_uint *output_length,
  char output[] );

};
