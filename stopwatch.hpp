#pragma once

#include "time.hpp"
#include "key.hpp"

namespace lb
{
	static Key<MicroTimer> mcKey;
	inline void StarTimer(const char *task)
	{
		MicroTimer *mc = mcKey;
		if (!mc)
		{
			mc = new MicroTimer();
			mcKey.Set(mc);
		}
		mc->Start();
		L(LOG_WARNING, "Begin %s\n", task);
	}
	inline void EndTimer(const char *task)
	{
		MicroTimer *mc = mcKey;
		if (mc)
		{
			Word32 endTime = mc->End();
			L(LOG_WARNING, "End %s (%2.6f)\n", task, ((double)endTime)/1000000);
		}
	}
};

