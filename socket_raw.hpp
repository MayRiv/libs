#pragma once

#include <cstddef>

#include "string.hpp"

namespace lb
{
	class TransportBase {
	public:
		typedef int TDescriptor;

		static const int SOCK_OP_TIMEOUT = 20; // socket operation time out in seconds
		static const int INVALID_SOCKET = -1;

		TransportBase(TDescriptor descr = INVALID_SOCKET)
			: _descr(descr)
		{
		}
		virtual ~TransportBase()
		{
			close();
		}

		virtual int close();
		virtual int open(const unsigned int ip, const short int port, const int timeOut = SOCK_OP_TIMEOUT);
		int read(char *buf, const size_t len, const int timeOut = SOCK_OP_TIMEOUT);
		int write(const char *buf, const size_t len, const int timeOut = SOCK_OP_TIMEOUT);
		int readImapAnswer(RString &buf, const char **cmdLabel, const int timeOut = SOCK_OP_TIMEOUT); // Read until find endTemple
		int readUntilStr(char *buf, int len, const char *endTemple, const int timeOut = SOCK_OP_TIMEOUT); // Read until find endTemple

	protected:
		TDescriptor _descr;

		bool _waitFor(const int events, const int timeOut);
		virtual int _read(char *buf, const size_t len) = 0;
		virtual int _write(const char *buf, const size_t len) = 0;
	};

	class Socket: public TransportBase {
	public:
		Socket(TDescriptor descr = INVALID_SOCKET)
			: TransportBase(descr)
		{
		}

	private:
		virtual int _read(char *buf, const size_t len);
		virtual int _write(const char *buf, const size_t len);
	};
};
