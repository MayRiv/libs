#pragma once

#include <openssl/ssl.h>

#include "socket_raw.hpp"

namespace lb {

	class SocketSSL: public TransportBase {
	public:
		static SSL_CTX *createServerCTX(const char *pKeyFile, const char *certFile, const char *caFile = NULL);

		SocketSSL(TDescriptor descr = INVALID_SOCKET)
			: TransportBase(descr), _pSSL(NULL)
		{
		}
		virtual ~SocketSSL()
		{
			close();
		}

		virtual int close();
		virtual int open(const unsigned int ip, short int port, const int timeOut = SOCK_OP_TIMEOUT);
		SSL *createSSL(SSL_CTX *pSSLCtx);
		int accept(const int timeOut = SOCK_OP_TIMEOUT);
	private:
		static bool _inited;
		static pthread_mutex_t *_lock_cs;
		static unsigned long _pthreads_thread_id(void)
		{
			 return (unsigned long) pthread_self(); 
		}
		static void _pthreads_locking_callback(int mode, int type, const char *file, int line)
		{
			if (mode & CRYPTO_LOCK) {
				pthread_mutex_lock(&(_lock_cs[type]));
			} else {
				pthread_mutex_unlock(&(_lock_cs[type]));
			}
		}

		SSL *_pSSL;

		static bool _init();
		bool _waitForSSL(const int sslErr, const int timeOut);
		virtual int _read(char *buf, const size_t len);
		virtual int _write(const char *buf, const size_t len);
	};

};
