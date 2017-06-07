#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "socket.hpp"
#include "thread.hpp"
#include "log.hpp"
#include "tools.hpp"
#include "time.hpp"
#include "ips.hpp"

#include "ehttpthread.hpp"

using namespace lb;
const Word32 EVENT_THREAD_STACK_SIZE = 100 * 1024;

HttpAcceptThread::HttpAcceptThread(SocketClient *listenSocket, HttpEventFactory* factory)
	: _webserver(listenSocket), _eventFactory(factory), _threadRunning(true)
{
	SetDetachState();
	SetStack(EVENT_THREAD_STACK_SIZE);
	if (Create())
		throw Error();
}

void HttpAcceptThread::run()
{
	static const int MAX_DEFER_TIMEOUT = 60;
	_webserver->SetDeferAccept(MAX_DEFER_TIMEOUT);
	while (_threadRunning)
	{
		SocketClient *client = _webserver->GetConnection();
		if (client)
		{
			client->SetNonBlock();
			if (!EHttpThread::AddConnection(_eventFactory->Create(client, 0, _webserver), _webserver))
				delete client;
		}
	}
	_webserver->Shutdown();
}


Time EHttpThread::_curTime;
EHttpThread::TEHttpThreadList EHttpThread::_threads;



void EHttpThread::RunEHttpThread(const Word32 maxThreads, const Word32 maxConns, HttpEventFactory* factory)
{
	for (Word32 i = 0; i < maxThreads; i++)
		_threads.push_back(new EHttpThread(maxConns, factory->CreateThreadSpecData()));
	L(LOG_WARNING, "Run %u event threads (%u)\n", _threads.size(), maxConns);
}


EHttpThread::EHttpThread(const Word32 maxConns, ThreadSpecData* threadData)
	: _poller(maxConns), _threadSpecData(threadData)
{
	SetDetachState();
	SetStack(EVENT_THREAD_STACK_SIZE);
	if (Create())
		throw Error();
}


static const Word32 POLLER_WAIT_TIME  = 60;

void EHttpThread::run()
{
	time_t lastCheckTime = 0;
	while (1)
	{
		_poller.Dispatch(POLLER_WAIT_TIME * 1000, false /*not call active*/);
		//_curTime.Update();

		_requestsSync.LockData();
		_poller.CallActiveEvents();

		if (lastCheckTime != _curTime.Unix())
		{
			THttpEventList::iterator reqEvent = _requestEvents.begin();
			while (reqEvent != _requestEvents.end())
			{
				HttpEvent *he = (*reqEvent);
				if (he->IsEnded(_curTime.Unix()))
				{
				#ifdef USE_SELFDESTRUCT
					he->free();
				#else
					delete he;
				#endif
					reqEvent = _requestEvents.erase(reqEvent);
				}
				else
					++reqEvent;
			}
			lastCheckTime = _curTime.Unix();
		}
		_requestsSync.UnLockData();
	}
}

bool EHttpThread::AddConnection(HttpEvent* ev, SocketClient *server)
{
	EHttpThread *httpThread = _threads[rand() % _threads.size()];
	if (httpThread->_AddConnection(ev, server))
		return true;
	else
	{
		for (TEHttpThreadList::iterator e = _threads.begin(); e != _threads.end(); ++e)
			if ((*e)->_AddConnection(ev, server))
				return true;
		return false;
	}
}

bool EHttpThread::_AddConnection(HttpEvent* ev, SocketClient *server)
{
	AutoMutexSyncData autoSync(&_requestsSync);

	ev->setThread(this);
	if (!Ctrl(ev))
	{
		delete ev;
		L(LOG_WARNING, "Can't add connection\n");
		return false;
	}
	_requestEvents.push_back(ev);
	return true;
}

bool EHttpThread::AddTimeoutNL(HttpEvent* ev)
{
	return _poller.Ctrl(ev, ev->getTimerDescr(), EPOLL_CTL_ADD, EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLONESHOT);
}

bool EHttpThread::pushEventNL(HttpEvent* ev)
{
	/*if (pthread_self() != tid) 
	{
		L(LOG_ERROR, "EThread: Failed to push event - wrong thread\n");
		return false;
	}*/
	bool res = _poller.Ctrl(ev);
	if (res)
	{
		_requestEvents.push_back(ev);
		return true;
	}
	return false;
}


void EHttpThread::Info(RString &buf, const bool fullMode)
{
	buf.snaprintf("Events threads %u\n<br>", _threads.size());
	for (TEHttpThreadList::iterator eThread = _threads.begin(); eThread != _threads.end(); ++eThread)
		buf.snaprintf("Thread %x (%u events)\n<br>", (*eThread)->tid, (*eThread)->_requestEvents.size());
	if(fullMode)
	{
		for (TEHttpThreadList::iterator eThread = _threads.begin(); eThread != _threads.end(); ++eThread)
			(*eThread)->_Info(buf);
	}
}

void EHttpThread::_Info(RString &buf)
{
	_curTime.Update();
	Word32 c = 0;
	_requestsSync.LockData();
	for (THttpEventList::iterator e = _requestEvents.begin(); e != _requestEvents.end(); ++e)
	{
		(*e)->Info(buf, _curTime);
		c++;
	}
	_requestsSync.UnLockData();
	buf.snaprintf("Thread sumary: %x (%u events)\n<br>", tid, c);

	if (_threadSpecData)
		_threadSpecData->Info(buf);
}
