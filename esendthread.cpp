#include "esendthread.hpp"

using namespace lb;
const Word32 EVENT_THREAD_STACK_SIZE = 100 * 1024;

ESendThread *ESendThread::_eSendThread = NULL;
Time ESendThread::_curTime;

void ESendThread::RunESendThread(const Word32 maxConns)
{
	_eSendThread = new ESendThread(maxConns);
	L(LOG_WARNING, "Run send event thread (%u)\n", maxConns);
}


ESendThread::ESendThread(const Word32 maxConns)
	: _poller(maxConns)
{
	SetDetachState();
	SetStack(EVENT_THREAD_STACK_SIZE);
	if (Create())
		throw Error();
}


static const Word32 POLLER_WAIT_TIME  = 60;

void ESendThread::run()
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
				SendEvent *he = (*reqEvent);
				if (he->IsEnded(_curTime.Unix()))
				{
					delete he;
					reqEvent = _requestEvents.erase(reqEvent);
				}
				else
					reqEvent++;
			}
			lastCheckTime = _curTime.Unix();
		}
		_requestsSync.UnLockData();
	}
}

bool ESendThread::AddEvent(SendEvent *sendEvent)
{
	return _eSendThread->_AddEvent(sendEvent);
}

bool ESendThread::_AddEvent(SendEvent *sendEvent)
{
	AutoMutexSyncData autoSync(&_requestsSync);
	if (!Ctrl(sendEvent))
	{
		delete sendEvent;
		L(LOG_WARNING, "Can't add connection\n");
		return false;
	}
	_requestEvents.push_back(sendEvent);
	return true;
}

void ESendThread::Info(RString &buf)
{
	_eSendThread->_requestsSync.LockData();
	buf.snaprintf("ESendThread: %u\n", _eSendThread->_requestEvents.size());
	_eSendThread->_requestsSync.UnLockData();
}
