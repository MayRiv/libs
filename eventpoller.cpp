#include <sys/epoll.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>


#include "log.hpp"
#include "eventpoller.hpp"

using namespace lb;

EventPoller::EventPoller(const int maxConn)
	: _maxConn(maxConn), _numActiveEvents(0)
{
	if ((_eventFD = epoll_create(_maxConn)) == -1) 
	{
		L(LOG_WARNING, "[ES] Can't init epoll\n");
		throw Error();
	}
	_events = new epoll_event[_maxConn];
};

EventPoller::~EventPoller()
{
	close(_eventFD);
	delete [] _events;
}

bool EventPoller::Ctrl(Event *event, const int descr, const Word32 op, const Word32 events)
{
	struct epoll_event ev;
	memset(&ev, 0, sizeof(ev));
	ev.data.ptr = event;
	ev.events = events;
	
	if (epoll_ctl(_eventFD, op, descr, &ev) == -1)
	{
		L(LOG_ERROR, "[ES] can't epoll_ctl %u %s (%u/%u)\n", errno, strerror(errno), event->_op, descr);
		return false;
	}
	
	return true;
}

bool EventPoller::Ctrl(Event *event)
{
	struct epoll_event ev;
	memset(&ev, 0, sizeof(ev));
	ev.data.ptr = event;
	ev.events = event->_events;
	
	if (epoll_ctl(_eventFD, event->_op, event->_descr, &ev) == -1)
	{
		L(LOG_ERROR, "[ES] can't epoll_ctl %u %s (%u/%u)\n", errno, strerror(errno), event->_op, event->_descr);
		return false;
	}

	event->_op = EPOLL_CTL_MOD;
	
	return true;
}

bool EventPoller::Dispatch(const int timeout, bool callActive)
{
	int res = epoll_wait(_eventFD, _events, _maxConn, timeout);

	if (res == -1) 
	{
		if (errno == EINTR || errno == EAGAIN)
			return true;
		L(LOG_ERROR, "[ES] signal com\n");
		return false;
	} 
	_numActiveEvents = res;

	//L(LOG_WARNING, "epoll_wait reports %d\n", res);

	if (callActive)
		CallActiveEvents();
	return true;
}

void EventPoller::CallActiveEvents()
{
	for (int i = 0; i < _numActiveEvents; i++) 
	{
		Event *ev = (Event *)_events[i].data.ptr;
		ev->Call(_events[i].events);
	}
}

Event::Event(const int descr)
		: _descr(descr), _events(0), _op(EPOLL_CTL_ADD)
{
}


/*
void EventSocket::Call(int events)
{
	if ((events & EPOLLHUP == EPOLLHUP) || (events & EPOLLERR == EPOLLERR))
	{
		L(LOG_ERROR, "EPoll error\n");
		return;
	}

	if ((events & EPOLLIN) && _needRead > 0)
	{
		while (true)
		{
			int readNum = recv(_descr,  _readBuf + _curRead, _needRead, MSG_NOSIGNAL);
			if (readNum == 0)  // connection break
			{
				ClearEvent(EPOLLIN);
				_ConnectionBreak();
				break;
			}
			else if (readNum >= _needRead) // end read
			{
				_needRead = 0;
				ClearEvent(EPOLLIN);
				_EndRead();
				break;
			}
			else if (errno == EAGAIN) // read again
			{
				_curRead += readNum;
				_needRead -= ReadNum;
				break;
			}
			else if (readNum < 0)
			{
				Error();
				ClearEvent(EPOLLIN);
				break;
			}
			_curRead += readNum;
			_needRead -= readNum;
		}
	}
	if (events & EPOLLOUT)
	{

	}
}
*/
