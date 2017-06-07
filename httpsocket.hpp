#pragma once
#include <vector>

#include "string.hpp"
#include "log.hpp"
#include "socket.hpp"
#include "mutexsync.hpp"

using namespace lb;

class HttpSocket : public RString
{
public:
	HttpSocket(const Word32 size)
		: RString(size), _sended(0)
	{
	}
	enum ESocketResult
	{
		OK,
		PROGRESS,
		ERROR,
		CONNECTION_CLOSE,
	};
	void Null()
	{
		RString::Null();
		_sended = 0;
	}
	void ZeroSended()
	{
		_sended = 0;
	}
	void SkipSend(const Word32 size)
	{
		_sended += size;
	}
	const int Sended() const
	{
		return _sended;
	}

	ESocketResult Send(const Descriptor descr);
	ESocketResult Read(const Descriptor descr);
	ESocketResult SendWNull(const Descriptor descr);
	ESocketResult AddRead(const Descriptor descr);
	ESocketResult AddRead(const Descriptor descr, Word32 size);
	static HttpSocket *Get(const t_RStringSize bufSize);
	static void Free(HttpSocket *buf);
	static void Info(RString &buf);

	static Word32 MAX_FREE_BUFS;
	static t_RStringSize MAX_BUF_SIZE;
protected:
	typedef std::vector<HttpSocket*> TBufferList;
	static TBufferList _freeBuffs;
	static MutexSyncData _buffLock;
	static int _buffsUse;

	struct Stat
	{
		Word32 freeBuff;
		Word32 createNewBuff;
		Word32 getBuf;
		Word32 resize;
		Word32 upResize;
	};
	static Stat _stat;
	int _sended;
};

