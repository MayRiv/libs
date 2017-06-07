#include <errno.h>
#include <unistd.h>

#include "log.hpp"
#include "storage.hpp"

#include "inotify.hpp"


const Word32 INOTIFY_BUF_SIZE = 1024 * sizeof(sizeof(struct inotify_event));

INotify::INotify() 
{
  _iFD = inotify_init();
  if (_iFD == -1) 
	{
		L(LOG_ERROR, "Can't init inotify system\n");
    throw Error();
  }
}

INotify::~INotify()
{
	close(_iFD);
}

bool INotify::Add(INotifyWatch* watch)
{
	AutoMutexSyncData autoSync(this);

	int watchDescr = inotify_add_watch(_iFD, watch->Path().c_str(), watch->Mask());
	if (watchDescr == -1)
	{
		L(LOG_ERROR, "Can't add watch to %s\n", watch->Path().c_str());
		return false;
	}
	watch->SetWatchDescr(watchDescr);
	_watches[watchDescr] = watch;
	return true;
}

bool INotify::Remove(INotifyWatch* watch)
{
	AutoMutexSyncData autoSync(this);
  if (watch->IsInited())
	{
    if (inotify_rm_watch(_iFD, watch->WatchDescr()) == -1) 
		{
			L(LOG_WARNING, "Can't remove watch %s\n", watch->Path().c_str());
			return false;
    }
    _watches.erase(watch->WatchDescr());
		delete watch;
  }
	return true;
}

bool INotify::ParseEvents(const bool noIntr) 
{
  ssize_t len = 0;
 
  do 
	{
		_readBuf.Start();
    len = read(_iFD, (char*)_readBuf.AddAttach(INOTIFY_BUF_SIZE), INOTIFY_BUF_SIZE);
  } while (noIntr && len == -1 && errno == EINTR);
  
 if (len == -1)
	{
		L(LOG_ERROR, "Read events files\n");
    return false;
	}

 
	_readBuf.Begin();
	 
  while (_readBuf.CurPos() < (Word32)len) 
	{
    struct inotify_event* evt = (struct inotify_event*) _readBuf.Attach(sizeof(inotify_event));
    INotifyWatch* watch = FindWatch(evt->wd);
		if (watch) 
		{
			bool notEnd = watch->Event(evt);
			if (!notEnd) // end watch
				Remove(watch);
		}
		_readBuf.Skip(evt->len);
  }
	return true;
}

void INotify::PrintEvent(const Word32 mask, class RString &buf)
{
	buf.Null();
	if ((mask & IN_ACCESS) == IN_ACCESS)
		buf.snaprintf("IN_ACCESS,");
	if ((mask & IN_MODIFY) == IN_MODIFY)
		buf.snaprintf("IN_MODIFY,");
	if ((mask & IN_ATTRIB) == IN_ATTRIB)
		buf.snaprintf("IN_ATTRIB,");
	if ((mask & IN_CREATE) == IN_CREATE)
		buf.snaprintf("IN_CREATE,");
	if ((mask & IN_DELETE) == IN_DELETE)
		buf.snaprintf("IN_DELETE,");
	if ((mask & IN_DELETE_SELF) == IN_DELETE_SELF)
		buf.snaprintf("IN_DELETE_SELF,");
	if ((mask & IN_OPEN) == IN_OPEN)
		buf.snaprintf("IN_OPEN,");

	if (mask & IN_CLOSE)
	{
		buf.snaprintf("IN_CLOSE (");
    if ((mask & IN_CLOSE_WRITE) == IN_CLOSE_WRITE)
			buf.snaprintf("IN_CLOSE_WRITE");
		else if ((mask & IN_CLOSE_NOWRITE) == IN_CLOSE_NOWRITE)
			buf.snaprintf("IN_CLOSE_NOWRITE");
		buf.snaprintf("),");
	}
	if ((mask & IN_MOVE_SELF) == IN_MOVE_SELF)
		buf.snaprintf("IN_MOVE_SELF,");

	if (mask & IN_MOVE)
	{
		buf.snaprintf("IN_MOVE (");
		if ((mask & IN_MOVED_FROM) == IN_MOVED_FROM)
			buf.snaprintf("IN_MOVED_FROM");
		if ((mask & IN_MOVED_TO) == IN_MOVED_TO)
			buf.snaprintf("IN_MOVED_TO");
		buf.snaprintf("),");
	}
	if ((mask & IN_ISDIR) == IN_ISDIR)
		buf.snaprintf("IN_ISDIR,");
	if ((mask & IN_UNMOUNT) == IN_UNMOUNT)
		buf.snaprintf("IN_UNMOUNT,");
	if ((mask & IN_IGNORED) == IN_IGNORED)
		buf.snaprintf("IN_IGNORED,");
		
	buf.BackSpace();
}


