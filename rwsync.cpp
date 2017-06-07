#include "rwsync.hpp"

namespace lb
{
	RWSyncData::RWSyncData()
	{
		pthread_rwlock_init(&rw, NULL);
	}

	RWSyncData::~RWSyncData()
	{
		pthread_rwlock_destroy(&rw);
	}

	void RWSyncData::RLockData()
	{
			pthread_rwlock_rdlock(&rw);
	}

	void RWSyncData::WLockData()
	{
			pthread_rwlock_wrlock(&rw);
	}

	void RWSyncData::UnLockData()
	{
			pthread_rwlock_unlock(&rw);
	}
};


