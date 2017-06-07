#include "httpevents.hpp"
#include "sendevent.hpp"
using namespace lb;

SendEvent::SendEvent(SocketClient *client, HttpSocket *buf, const Word32 addTime)
	: Event(client->GetDescr()), _client(client), _buf(buf), _addTime(addTime)
{
	AddEvent(EPOLLOUT | EPOLLERR | EPOLLHUP);
}

SendEvent::~SendEvent()
{
	EndWork();
}

bool SendEvent::IsEnded(const Word32 curTime)
{
	static const Word32 MAX_WAIT_SEND = 60*2;

	if (_addTime < (curTime - MAX_WAIT_SEND))
		return true;
	else
		return false;
}

void SendEvent::EndWork()
{
	if (_client)
	{
		delete _client;
		_client = NULL;
	}
	if (_buf)
	{
		HttpSocket::Free(_buf);
		_buf = NULL;
	}
}

void SendEvent::Call(int events)
{
	if ((events & EPOLLHUP) == EPOLLHUP)
	{
		//LU(LOG_ERROR, "EPoll EPOLLHUP\n");
		EndWork();
		return;
	}
	if ((events & EPOLLERR) == EPOLLERR)
	{
		L(LOG_ERROR, "EPoll EPOLLERR\n");
		EndWork();
		return;
	}

	if ((events & EPOLLOUT) == EPOLLOUT)
	{
		HttpSocket::ESocketResult res = _buf->Send(_descr);
		if (res == HttpSocket::OK)
		{
			EndWork();
		}
		else if (res != HttpSocket::PROGRESS)
		{
			L(LOG_WARNING, "[UE] Send data error\n");
			EndWork();
		}
	}
}
