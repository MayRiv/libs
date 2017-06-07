#pragma once


#include <openssl/ssl.h>
#include <openssl/err.h>

#include <vector>

#include "eventpoller.hpp"
#include "httpevents.hpp"
#include "ehttpthread.hpp"
#include "string.hpp"

#include "httpsocket.hpp"

namespace lb
{

class HttpsSocket: public HttpSocket
{
public:

	enum EResult
	{
		RES_OK,
		RES_PROGRESS,
		RES_ERROR,
		RES_CONNECTION_CLOSE,
	};

	enum EProgressRequest
	{
		REQ_NONE,
		REQ_WANT_READ,
		REQ_WANT_WRITE
	};

	HttpsSocket(int size):
		HttpSocket(size), _requestIO(REQ_NONE)
	{
	}

	char* getAppendBuf(int &freeSpace)
	{
		freeSpace = maxSize - length;
		return data + length;
	}
	
	char* getEnd()
	{
		return data + length;
	}

	EProgressRequest getIORequest()
	{
		return _requestIO;
	}

	EResult send(SSL *ssl);
	EResult recv(SSL *ssl);
	EResult shutdown(SSL *ssl);
	EResult handshake(SSL *ssl);

	static HttpsSocket* get(int bufSize);
	static void free(HttpsSocket *buf);

	static Word32 MAX_FREE_BUFS;
	static t_RStringSize MAX_BUF_SIZE;

protected:
	bool _checkSSLIO(SSL* _ssl, int &res);

	typedef std::vector<HttpsSocket*> TBufferList;
	static TBufferList _freeBuffs;
	static MutexSyncData _buffLock;
	static int _buffsUse;

	EProgressRequest _requestIO;

	struct Stat
	{
		Word32 freeBuff;
		Word32 createNewBuff;
		Word32 getBuf;
		Word32 resize;
		Word32 upResize;
	};
	static Stat _stat;
};

class HttpsEvent : public HttpEvent
{
public:
	static bool initSSL(const char *pKeyFile, const char *certFile, const char *caFile, time_t timeout);
	static void freeSSL();
	static bool restartSSL(const char *pKeyFile, const char *certFile, const char *caFile, time_t timeout);

	HttpsEvent(SocketClient *client, class EHttpThread *thread, HttpEventListener *listener);
	virtual ~HttpsEvent(void);
	virtual void Call(int events);
	virtual void Info(RString &buf, Time &curTime);
	virtual void EndWork();
	
	virtual HttpSocket* getBuffer()
	{
		return _buf;
	}

	virtual void SendErrorPage();
	virtual void sendAnswer();
	static const Word32 ENDPOINT_BUF_SZ;
	static int MAX_CHUNK_COUNT;
	static const Word32 MAX_HANDSHAKE_RETRY_COUNT = 5;
protected:
	static SSL_CTX *_createCTX(const char *pKeyFile, const char *certFile, const char *caFile, time_t timeout);// timeout == 0 - will be used SSL default 
	static SSL_CTX* _sslContext;
	
	bool _proccesSSL();
	bool _ReceiveQuery();
	bool _TrySendAnswer();
	bool _Shutdown();
	bool _Handshake();

	void _checkIORequests();
	SSL* _ssl;

	Word32 _handshakeRetry;
	HttpsSocket *_buf;

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
};

class HttpsEventFactory : public HttpEventFactory
{
public:
	virtual HttpsEvent *Create(SocketClient *client, class EHttpThread *thread, SocketClient *server) = 0;
	virtual ThreadSpecData *CreateThreadSpecData() { return NULL; };
	virtual ~HttpsEventFactory() {};
};

}
