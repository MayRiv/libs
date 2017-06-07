
#include "httpsocket.hpp"

#include <sys/socket.h>
#include <errno.h>




HttpSocket::ESocketResult HttpSocket::AddRead(const Descriptor descr)
{
	_sended = 0;
	int chunkSize = maxSize - length - 1;
	if (chunkSize < (maxSize / 2))
		chunkSize = maxSize - 1;

	char *queryBuf = GetBuf(chunkSize);
	int res = recv(descr,  queryBuf,  chunkSize - 1, MSG_NOSIGNAL | MSG_DONTWAIT);
	SetLength(length - (chunkSize - (res > 0 ? res : 0)));
	if (res > 0)
		return OK;

	if (res == 0)
		return CONNECTION_CLOSE;

	if (errno == EAGAIN)
		return PROGRESS;
	else
	{
		Null();
		if (errno == ENOTSOCK)
				L(LOG_WARNING, "Not sock %d\n", descr);
		L(LOG_WARNING, "Error read %u (%s)\n", errno, strerror(errno));
		return ERROR;
	}
}


HttpSocket::ESocketResult HttpSocket::AddRead(const Descriptor descr, Word32 size)
{
	int needed = size - length;
	if (needed <= 0)
		return OK;
	_sended = 0;
	char *queryBuf = GetBuf(needed);
	int res = recv(descr,  queryBuf, needed, MSG_NOSIGNAL | MSG_DONTWAIT);
	SetLength(length - (needed - (res > 0 ? res : 0)));
	if (res > 0)
		return OK;

	if (res == 0)
		return CONNECTION_CLOSE;

	if (errno == EAGAIN)
		return PROGRESS;
	else
	{
		Null();
		if (errno == ENOTSOCK)
				L(LOG_WARNING, "Not sock %d\n", descr);
		L(LOG_WARNING, "Error read %u (%s)\n", errno, strerror(errno));
		return ERROR;
	}
}


HttpSocket::ESocketResult HttpSocket::Read(const Descriptor descr)
{
	if (length == 0) // can read more data
	{
		_sended = 0;
		int chunkSize = maxSize - 1;
		char *queryBuf = GetBuf(chunkSize);
		int res = recv(descr,  queryBuf,  chunkSize - 1, MSG_NOSIGNAL | MSG_DONTWAIT);
		if (res > 0)
		{
			SetLength(length - (chunkSize - res));
			return OK;
		}
		Null();

		if (res == 0)
			return CONNECTION_CLOSE;

		if (errno == EAGAIN)
			return PROGRESS;
		else
		{
			if (errno == ENOTSOCK)
					L(LOG_WARNING, "Not sock %d\n", descr);

			L(LOG_WARNING, "Error read %u (%s)\n", errno, strerror(errno));
			return ERROR;
		}
	}
	else
		return PROGRESS;
}

HttpSocket::ESocketResult HttpSocket::Send(const Descriptor descr)
{
	int lastSend = length - _sended;
	if (lastSend <= 0)
		return OK;

	int res = send(descr, data + _sended, lastSend, MSG_NOSIGNAL);
	if (res > 0)
	{
		_sended += res;
		if (res < lastSend)
			return PROGRESS;
		else
		{
			Null();
			return OK;
		}
	}
	if (errno == EAGAIN || errno == EINTR)
		return PROGRESS;
	else
	{
		if (errno == ENOTSOCK)
				L(LOG_WARNING, "Not sock %d\n", descr);

		L(LOG_WARNING, "Error send %u (%s)\n", errno, strerror(errno));
		return ERROR;
	}
}

HttpSocket::ESocketResult HttpSocket::SendWNull(const Descriptor descr)
{
	int lastSend = length - _sended;
	if (lastSend <= 0)
		return OK;

	int res = send(descr, data + _sended, lastSend, MSG_NOSIGNAL);
	if (res > 0)
	{
		_sended += res;
		if (res < lastSend)
			return PROGRESS;
		else
			return OK;
	}
	if (errno == EAGAIN)
		return PROGRESS;
	else
	{
		if (errno == ENOTSOCK)
				L(LOG_WARNING, "Not sock %d\n", descr);

		L(LOG_WARNING, "Error send %u (%s)\n", errno, strerror(errno));
		return ERROR;
	}
}

HttpSocket::TBufferList HttpSocket::_freeBuffs;
MutexSyncData HttpSocket::_buffLock;
int HttpSocket::_buffsUse = 0;

HttpSocket::Stat HttpSocket::_stat;

HttpSocket *HttpSocket::Get(const t_RStringSize bufSize)
{
	HttpSocket *buf = NULL;
	_buffLock.LockData();
	if (!_freeBuffs.empty())
	{
		buf = _freeBuffs.back();
		_freeBuffs.pop_back();
		buf->Null();
	}
	else
	{
		buf = new HttpSocket(bufSize);
		_stat.createNewBuff++;
	}
	_buffsUse++;
	_stat.getBuf++;
	_buffLock.UnLockData();

	if (buf->MaxSize() < bufSize)
	{
		buf->SetMaxSize(bufSize);
		_stat.upResize++;
	}
	return buf;
}


Word32 HttpSocket::MAX_FREE_BUFS = 5000;
t_RStringSize HttpSocket::MAX_BUF_SIZE = 15000;

void HttpSocket::Free(HttpSocket *buf)
{
	_buffLock.LockData();
	_buffsUse--;
	if (_freeBuffs.size() < MAX_FREE_BUFS)
	{
		if (buf->MaxSize() > MAX_BUF_SIZE)
		{
			buf->SetMaxSize(MAX_BUF_SIZE);
			_stat.resize++;
		}
		_freeBuffs.push_back(buf);
	}
	else
	{
		delete buf;
		_stat.freeBuff++;
	}
	_buffLock.UnLockData();
}

void HttpSocket::Info(RString &buf)
{
	buf.snaprintf("SocketBuff Use: %d, free %u, Stat: %u (%u/%u) r: %u, ur: %u<br>", _buffsUse, _freeBuffs.size(), _stat.getBuf, _stat.createNewBuff, _stat.freeBuff, _stat.resize, _stat.upResize);
	memset(&_stat, 0, sizeof(_stat));
}