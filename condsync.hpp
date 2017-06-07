#ifndef __COND_SYNC_HPP__
#define __COND_SYNC_HPP__

#include <pthread.h>

class CondSync
{
public:
	CondSync()
	{
		 pthread_mutex_init(&mutex, NULL);
		 pthread_cond_init(&cond, NULL);
	}
	~CondSync()
	{
		pthread_mutex_destroy(&mutex);
		pthread_cond_destroy(&cond);
	}
	void Wait()
	{
		pthread_mutex_lock(&mutex);
		pthread_cond_wait(&cond, &mutex);
		pthread_mutex_unlock(&mutex);		
	}
	void Call()
	{
		pthread_cond_signal(&cond);
	}
	void CallAll()
	{
		pthread_cond_broadcast(&cond);
	}
protected:
	pthread_mutex_t mutex;
	pthread_cond_t cond;
};

#endif
