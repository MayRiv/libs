#pragma once

#include <pthread.h>
#include "key.hpp"

namespace lb
{
	typedef void* (*FNPTR) ( void *);

	class Thread
	{
	public:
		Thread();
		virtual ~Thread();
		void SetStack(unsigned int steckSize)
		{
			pthread_attr_setstacksize(&attr, steckSize);
					
		};
		void SetDetachState()
		{
			pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		}
		int Create()
		{
			return pthread_create (&tid, &attr, Action, (void *) this);
		};
		void WaitMe();
		void Lock();
		void UnLock();
		void SetStatus(const char *str)
		{
			status = str;
		};
		void Cancel();
		const char *Status() const 
		{ 
			return status; 
		}
		pthread_t Tid() { return tid;}
		class Error {};
	protected:
		static void* Action(void *);
		virtual void run() = 0;
		const char *status;
		pthread_t tid;
		pthread_attr_t attr;
		pthread_mutex_t lockKey;
	};


	extern Key<Thread> curThread;
};

