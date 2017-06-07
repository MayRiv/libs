#include <pthread.h>
#include "mutexsync.hpp"

using namespace lb;


MutexSyncData::MutexSyncData()
{
	pthread_mutex_init(&mutex, NULL);
}

MutexSyncData::~MutexSyncData()
{
	pthread_mutex_destroy(&mutex);
}

void MutexSyncData::LockData()
{
	pthread_mutex_lock(&mutex);
}

int MutexSyncData::TryLockData()
{
	if (pthread_mutex_trylock(&mutex)) // mute busy
		return 0;
	return 1;
}

void MutexSyncData::UnLockData()
{
	  pthread_mutex_unlock(&mutex);
}



