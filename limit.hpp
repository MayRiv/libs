#pragma once

namespace lb
{
	class Limit
	{
	public:
		static int EnableCoreDump();
		static int SetMaxOpenFiles(const int maxOpenFiles);
		static int SetMaxProcess(const int maxProcess);
	};
};
