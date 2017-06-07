#pragma once

#include <list>

#include "condsync.hpp"
#include "thread.hpp"
#include "xtypes.hpp"
#include "socket.hpp"
#include "eventpoller.hpp"
#include "mutexsync.hpp"
#include "hashmap.hpp"
#include "ips.hpp"

#include "httpevents.hpp"

namespace lb
{
	class EHttpThread : public Thread
	{
	public:
		EHttpThread(const Word32 maxConns, ThreadSpecData* threadData);
		bool Ctrl(Event *ue)
		{
			return _poller.Ctrl(ue);
		}
		static bool AddConnection(HttpEvent* ev, SocketClient *server);

		static void RunEHttpThread(const Word32 maxThreads, const Word32 maxConns, HttpEventFactory* factory);
		RString &ThreadBuf()
		{
			return _threadBuf;
		}
		static void UpdateTime()
		{
			_curTime.Update();
		}
		ThreadSpecData *GetThreadSpecData()
		{
			return _threadSpecData;
		}

		bool pushEventNL(HttpEvent* ev);

		bool AddTimeoutNL(HttpEvent* ev);

		static Time _curTime;
		static int InitIps(class Mysql &sql);
		static void Info(RString &buf, const bool fullMode = true);
	protected:
		virtual void run();
	
		typedef std::vector<EHttpThread*> TEHttpThreadList;
		static TEHttpThreadList _threads;
		bool _AddConnection(HttpEvent* ev, SocketClient *server);
		void _Info(RString &buf);
	
		EventPoller _poller;
		typedef std::list<HttpEvent*> THttpEventList;
		THttpEventList _requestEvents;
		MutexSyncData _requestsSync;
		RString _threadBuf;

		ThreadSpecData *_threadSpecData;
		MutexSyncData _sync;
	};

	class HttpAcceptThread : public Thread
	{
	public:
		HttpAcceptThread(SocketClient *listenSocket, HttpEventFactory* factory);
		void ShutdownThread()
		{
			_threadRunning = false;
			return;
		};
	private:
		virtual void run();
		SocketClient *_webserver;
		HttpEventFactory* _eventFactory;
		bool _threadRunning;
	};
}
