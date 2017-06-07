#pragma once

#include <sys/inotify.h>
#include "mutexsync.hpp"
#include "hashmap.hpp"

using namespace lb;

class INotifyWatch 
{
public:
	INotifyWatch(const Word32 mask)
		: _watchDescr(-1), _mask(mask)
	{
	}
	virtual ~INotifyWatch() {};

	
	const Word32 Mask() const
	{
		return _mask;
	}
	void SetWatchDescr(const int watchDescr)
	{
		_watchDescr = watchDescr;
	}
	const int WatchDescr() const
	{
		return _watchDescr;
	}
	bool IsInited()
	{
		return _watchDescr == -1;
	}
	virtual bool Event(struct inotify_event* evt) = 0;
	virtual const std::string &Path() = 0;
protected:
	int _watchDescr;
	Word32 _mask;
};

class INotify : public MutexSyncData
{
public:
	class Error {};
	INotify();
	~INotify();
	bool Add(INotifyWatch* watch);
	bool ParseEvents(const bool noIntr = false);
	INotifyWatch *FindWatch(const int watchDescr)
	{
		AutoMutexSyncData autoSync(this);
		TINotifyWatchHash::iterator f = _watches.find(watchDescr);
		if (f != _watches.end())
			return f->second;
		else
			return NULL;
	}
	bool Remove(INotifyWatch* watch);
	static void PrintEvent(const Word32 mask, class RString &buf);
private:

	int _iFD;
	typedef hash_map<int, INotifyWatch*> TINotifyWatchHash;
	TINotifyWatchHash _watches;
	Storage _readBuf;
};
