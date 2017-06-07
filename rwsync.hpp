#pragma once

#include <pthread.h>

namespace lb
{
	class RWSyncData
	{
	public:
		RWSyncData();
		~RWSyncData();
		void RLockData();
		void WLockData();
		void UnLockData();
	protected:
		pthread_rwlock_t rw;
	};

	class AutoRWSyncDataR
	{
	public:
		AutoRWSyncDataR(RWSyncData *sync)
			: sync(sync)
		{
			sync->RLockData();
		}
		~AutoRWSyncDataR()
		{
			if (sync)
				sync->UnLockData();
		}
		void UnLockData()
		{
			if (sync)
			{
				sync->UnLockData();
				sync = NULL;
			}
		}
		void LockData(RWSyncData *sync)
		{
			this->sync = sync;
			sync->RLockData();
		}
	protected:
		RWSyncData *sync;
	};

	class AutoRWSyncDataW
	{
	public:
		AutoRWSyncDataW()
			: sync(NULL)
		{
		}
		AutoRWSyncDataW(RWSyncData *sync)
			: sync(sync)
		{
			sync->WLockData();
		}
		~AutoRWSyncDataW()
		{
			if (sync)
				sync->UnLockData();
		}
		void UnLockData()
		{
			if (sync)
			{
				sync->UnLockData();
				sync = NULL;
			}
		}
		void LockData(RWSyncData *sync)
		{
			this->sync = sync;
			sync->WLockData();
		}
		void LockDataR(RWSyncData *sync)
		{
			this->sync = sync;
			sync->RLockData();
		}
	protected:
		RWSyncData *sync;
	};
};


