#pragma once

#include "string.hpp"

namespace lb
{
	class ClearGif
	{
	public:
		static void Form(RString &buf, bool isKeepAlive = false);
		static void Form(RString &buf, RString &cookies, bool isKeepAlive = false);
	};
};

