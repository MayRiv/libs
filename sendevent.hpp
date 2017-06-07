#pragma once

#include "eventpoller.hpp"
#include "string.hpp"
#include "log.hpp"
#include "time.hpp"
#include "socket.hpp"
#include "mutexsync.hpp"

namespace lb
{
	class SendEvent : public Event
	{
	public:
		SendEvent(class SocketClient *client, class HttpSocket *buf, const Word32 addTime);
		virtual ~SendEvent();
		void EndWork();
		bool IsEnded(const Word32 curTime);
	private:
		virtual void Call(int events);
		class SocketClient *_client;
		class HttpSocket *_buf;
		Word32 _addTime;
	};
};
