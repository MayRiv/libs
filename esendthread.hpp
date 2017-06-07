#pragma once

#include "thread.hpp"
#include "socket.hpp"
#include "eventpoller.hpp"
#include "mutexsync.hpp"
#include "hashmap.hpp"
#include <list>
#include "ips.hpp"

#include "httpevents.hpp"
#include "sendevent.hpp"

namespace lb
{
	class ESendThread : public Thread
	{
	public:
		ESendThread(const Word32 maxConns);
		static bool AddEvent(class SendEvent *sendEvent);
		static void RunESendThread(const Word32 maxConns);
		static Time _curTime;
		bool Ctrl(Event *ue)
		{
			return _poller.Ctrl(ue);
		}
		static void UpdateTime()
		{
			_curTime.Update();
		}
		static void Info(RString &buf);
	private:
		virtual void run();
		static ESendThread *_eSendThread;
		bool _AddEvent(class SendEvent *sendEvent);

		EventPoller _poller;
		typedef std::list<class SendEvent*> THttpEventList;
		THttpEventList _requestEvents;
		MutexSyncData _requestsSync;
	};
}
