#pragma once


#include <vector>
#include "xtypes.hpp"
#include <sys/epoll.h>

namespace lb
{

class Event 
{
public:
	Event(const int descr);
	friend class EventPoller;
	virtual void Call(int events) = 0;
	virtual ~Event() {};

	void ClearEvent(int mask)
	{
		_events &= ~(mask);
	}	
	void AddEvent(int mask)
	{
		_events |= mask;
	}
	void Close()
	{
		_events = 0;
		_op = EPOLL_CTL_DEL;
	}
	int Op()
	{
		return _op;
	}
	Word32 events() const
	{
		return _events;
	}
protected:
	int _descr;
	Word32 _events;
	int _op;
};

class EventPoller
{
public:
	class Error {};
	EventPoller(const int maxConn);
	~EventPoller();
	bool Ctrl(Event *event);
	bool Ctrl(Event *event, const int descr, const Word32 op, const Word32 events);
	bool Dispatch(const int timeout, bool callActive);
	void CallActiveEvents();
	const int MaxConn() const
	{
		return _maxConn;
	}
private:
	int _eventFD;
	int _maxConn;
	struct epoll_event *_events;
	int _numActiveEvents;
};

};
