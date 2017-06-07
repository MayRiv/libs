#pragma once

#include <pthread.h>
namespace lb
{
	class MutexSyncData
	{
	public:
		MutexSyncData();
		~MutexSyncData();
		void LockData();
		int TryLockData();
		void UnLockData();
	protected:
		pthread_mutex_t mutex;
	};

	class AutoMutexSyncData
	{
	public:
		AutoMutexSyncData()
			: sync(NULL)
		{

		}
		AutoMutexSyncData(MutexSyncData *sync)
			: sync(sync)
		{
			sync->LockData();
		}
		AutoMutexSyncData(MutexSyncData *sync, bool lock)
			: sync(sync)
		{
			if (lock)
				sync->LockData();
		}

		~AutoMutexSyncData()
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
		void LockData(MutexSyncData *sync)
		{
			this->sync = sync;
			sync->LockData();
		}
		bool TryLockData(MutexSyncData *sync)
		{
			if (sync->TryLockData())
			{
				this->sync = sync;
				return true;
			}
			else
				return false;
		}
	protected:
		MutexSyncData *sync;
	};
}


