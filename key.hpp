#pragma once

#include <pthread.h>
#include "log.hpp"

namespace lb
{
	template <class D> class Key
	{
	public:
		Key()
		{
			if (pthread_key_create(&key, NULL) != 0)
			{
				L(LOG_CRITICAL, "pthread_create_key failed\n");
				throw true;
			}
		}
		void Set(D *data)
		{
			pthread_setspecific(key, data);
		}
		D *Get()
		{
			return (D*) pthread_getspecific(key);
		}
		operator	D *()
		{
			return (D*) pthread_getspecific(key);
		}
		D	* operator	->() const
		{
			return (D*) pthread_getspecific(key);	
		}

		operator	bool() const
		{
			D *data = pthread_getspecific(key);	
			return data;
		}
	protected:
		pthread_key_t key;
	};
}

