#pragma once

#include <list>
#include "thread.hpp"
#include "mutexsync.hpp"

namespace lb
{
	typedef std::list<Thread*> TThreadList;

	template<class D> class MutexThreadManager : public MutexSyncData
	{
	public:
		MutexThreadManager()
		{
			count = 0;
		}
		MutexThreadManager(class SocketClient *webServer, int MAX_THREAD)
		{
			count = 0;
			this->webServer = webServer;
			for (int i = 0; i < MAX_THREAD; i++)
				new D(this, this->webServer);
		}
		
	
		~MutexThreadManager()
		{
			for (TThreadList::iterator t = threads.begin(); t != threads.end(); t++)
				delete (*t);
		}

		void Newtask();

		void AddWaitThread()
		{
			LockData();
		}
		void RemWaitThread()
		{
			UnLockData();
		}
		
		void Remove(Thread *thread)
		{
			listSync.LockData();
			threads.remove(thread);
			count--;
			listSync.UnLockData();
		};
		void Add(Thread *thread)
		{
			listSync.LockData();
			threads.push_back(thread);
			count++;
			listSync.UnLockData();
		};
		TThreadList &ThrList()
		{
			return threads;
		}
	protected:
		class SocketClient *webServer;
		TThreadList threads;
		MutexSyncData listSync;
		int count;
	};

	template<class D> class DynMutexThreadManager : public MutexThreadManager<D>
	{
	public:
		DynMutexThreadManager(int MIN_WAIT_THREAD, int MAX_THREAD)
			: waitCount(0), minWaitThread(MIN_WAIT_THREAD),maxThread(MAX_THREAD)
		{
		}
		virtual ~DynMutexThreadManager()
		{
		}

		virtual void Newtask() = 0;

		static const double MIN_WAIT_KOEF = 1.1;

		int AddWaitThread()
		{
			if (waitCount >= (minWaitThread * MIN_WAIT_KOEF))
				return 0;
			waitCount++;
			MutexThreadManager<D>::LockData();
			return 1;
		}
		void RemWaitThread()
		{
			waitCount--;
			if (waitCount < minWaitThread && MutexThreadManager<D>::count < maxThread) // add new thread
				Newtask();
			MutexThreadManager<D>::UnLockData();
		}
		int Count()
		{
			return MutexThreadManager<D>::count;
		}
		int WaitCount()
		{
			return waitCount;
		}
	protected:
		int waitCount;
		int minWaitThread;
		int maxThread;
	};
};

