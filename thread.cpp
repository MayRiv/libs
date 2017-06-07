#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#include "thread.hpp"

using namespace lb;

Key<Thread> lb::curThread;

Thread::Thread()
{
	pthread_mutex_init(&lockKey, NULL);
	pthread_attr_init(&attr);
	status = "Init";
}


Thread::~Thread()
{
	curThread.Set(NULL);
	pthread_attr_destroy(&attr);
	pthread_mutex_destroy(&lockKey);
}

void *Thread::Action(void *arg)
{
	Thread *thr = (Thread *) arg;
	curThread.Set(thr);
	thr->run();
	pthread_exit(NULL);
	return 0;
}

void Thread::Lock()
{
	if (pthread_mutex_lock(&lockKey))
		throw true;
}

void Thread::UnLock()
{
	if (pthread_mutex_unlock(&lockKey))
		throw true;
}

void Thread::WaitMe()
{
	void *ret_val;
	pthread_join(tid, &ret_val);
}
void Thread::Cancel()
{
	pthread_cancel(tid);
}

