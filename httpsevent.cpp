#include <signal.h>
#include <errno.h>

#include "httpsevent.hpp"
#include "sslexcache.hpp"

namespace lb
{

HttpsSocket::TBufferList HttpsSocket::_freeBuffs;
MutexSyncData HttpsSocket::_buffLock;
int HttpsSocket::_buffsUse = 0;
HttpsSocket::Stat HttpsSocket::_stat;

Word32 HttpsSocket::MAX_FREE_BUFS = 5000;
t_RStringSize HttpsSocket::MAX_BUF_SIZE = 15000;
int HttpsEvent::MAX_CHUNK_COUNT = 20;

HttpsSocket* HttpsSocket::get(int bufSize)
{
	HttpsSocket *buf = NULL;
	AutoMutexSyncData autoMutex(&_buffLock);
	if (!_freeBuffs.empty())
	{
		buf = _freeBuffs.back();
		_freeBuffs.pop_back();
		buf->Null();
		buf->_requestIO = REQ_NONE;
	}
	else
	{
		buf = new HttpsSocket(bufSize);
		_stat.createNewBuff++;
	}
	_buffsUse++;
	_stat.getBuf++;

	if (buf->MaxSize() < bufSize)
	{
		buf->SetMaxSize(bufSize);
		_stat.upResize++;
	}
	return buf;
}

void HttpsSocket::free(HttpsSocket *buf)
{
	if (!buf)
		return;
	AutoMutexSyncData autoMutex(&_buffLock);
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
}

bool HttpsSocket::_checkSSLIO(SSL* _ssl, int &res)
{
	int err = SSL_get_error(_ssl, res);
	if (err != SSL_ERROR_NONE)
	{
		if (err == SSL_ERROR_WANT_READ)
		{
			if (errno == EAGAIN || errno == EINTR)
			{
				_requestIO = REQ_WANT_READ;
				return true;
			}
			res = 1; //repeat ssl func call
			return false;
		}
		if (err == SSL_ERROR_WANT_WRITE)
		{
			if (errno == EAGAIN || errno == EINTR)
			{
				_requestIO = REQ_WANT_WRITE;
				return true;
			}
			res = 1; //repeat ssl func call
			return false;
		}
		int err_s = ERR_get_error();
		if (err_s != 0)
		{
			L(LOG_ERROR, "SSL processing error: %d %d %s %s(errno %d)\n", err, err_s, ERR_func_error_string(err_s), ERR_reason_error_string(err_s), errno);
		}
		return false;
	}
	return false;
}

HttpsSocket::EResult HttpsSocket::shutdown(SSL *ssl)
{
	int res;
	size_t retry = 6;
	while( !(res = SSL_shutdown(ssl)) )
	{
		if (retry-- <= 0) {
			L(LOG_WARNING, "[SSL] SSL_shutdown Handshake limit excited\n");
			break;
		}
	}
	if (res < 0)
	{
		int err = SSL_get_error(ssl, res);
		if (err == SSL_ERROR_WANT_WRITE )
		{
			_requestIO = REQ_WANT_WRITE;
			return RES_PROGRESS;
		}
		if (err == SSL_ERROR_WANT_READ)
		{
			_requestIO = REQ_WANT_READ;
			return RES_PROGRESS;
		}

		if (err == SSL_ERROR_SYSCALL && (SSL_get_shutdown(ssl) & SSL_SENT_SHUTDOWN) != SSL_SENT_SHUTDOWN )
		{
			L(LOG_WARNING, "SSL connection was not closed gracefully\n");
		}
		int err_s = ERR_get_error();
		if (err_s != 0)
		{
			//L(LOG_ERROR, "SSL processing error: %d %d %s %s(errno %d)\n", err, err_s, ERR_func_error_string(err_s), ERR_reason_error_string(err_s), errno);
		}
		return RES_ERROR;
	}
	return RES_OK;
}

HttpsSocket::EResult HttpsSocket::handshake(SSL *ssl)
{
	int res = SSL_do_handshake(ssl);
	if (res == 0)
	{
		//L(LOG_ERROR, "Handshake Error\n");
		return RES_ERROR;
	}
	if (res < 0)
	{
		if (_checkSSLIO(ssl, res))
			return RES_PROGRESS;
		return RES_ERROR;
	}
	return RES_OK;
}

HttpsSocket::EResult HttpsSocket::send(SSL* ssl)
{
	int lastSend = length - _sended;
	if (lastSend <= 0)
		return RES_OK;
	int res;
	do {
		res = SSL_write(ssl, data + _sended, lastSend); //send(descr, data + _sended, lastSend, MSG_NOSIGNAL);
		if (res > 0)
		{
			_sended += res;
			if (res < lastSend)
				return RES_PROGRESS;
			else
			{
				Null();
				return RES_OK;
			}
		}

		if (_checkSSLIO(ssl, res))
		{
			return RES_PROGRESS;
		}
	} while (res > 0);
	
	if (res == 0)
		return RES_CONNECTION_CLOSE;

	L(LOG_WARNING, "Error send %u (%s)\n", errno, strerror(errno));
	return RES_ERROR;
}

HttpsSocket::EResult HttpsSocket::recv(SSL* ssl)
{
	_sended = 0;
	int res;
	do
	{
		int chunkSize = maxSize - length - 1;
		if (chunkSize < (maxSize / 2))
			chunkSize = maxSize - 1;

		char *queryBuf = GetBuf(chunkSize);
		res =  SSL_read(ssl, queryBuf, chunkSize - 1);       //recv(descr,  queryBuf,  chunkSize - 1, MSG_NOSIGNAL | MSG_DONTWAIT);
		if (res > 0)
		{
			SetLength(length - (chunkSize - res));
			//return RES_OK;
		}
		if (_checkSSLIO(ssl, res))
		{
			SetLength(length - chunkSize);
			return RES_PROGRESS;
		}
	} while (res > 0);

	Null();

	if (res == 0)
		return RES_CONNECTION_CLOSE;

	L(LOG_WARNING, "Error send %u (%s)\n", errno, strerror(errno));
	return RES_ERROR;
}


pthread_mutex_t *HttpsEvent::_lock_cs = NULL;
SSL_CTX* HttpsEvent::_sslContext = NULL;

const Word32 HttpsEvent::ENDPOINT_BUF_SZ = 2048;

SSL_CTX *HttpsEvent::_createCTX(const char *pKeyFile, const char *certFile, const char *caFile, time_t timeout)
{
	SSL_CTX *pSSLCtx = NULL;
	try {
		/* Create a SSL_METHOD structure (choose a SSL/TLS protocol version) */
		const SSL_METHOD *meth = SSLv23_server_method();

		/* Create a SSL_CTX structure */
		pSSLCtx = SSL_CTX_new(const_cast<SSL_METHOD *>(meth));
		if (pSSLCtx == NULL)
			throw "SSL_CTX_new()";

		SSL_CTX_set_options(pSSLCtx, SSL_OP_NO_SSLv2);

		if (SSL_CTX_set_cipher_list(pSSLCtx, "kEECDH+AESGCM+AES128:kEECDH+AES128:kRSA+AESGCM+AES128:kRSA+AES128:!RC4:!aNULL:!eNULL:!MD5:!EXPORT:!LOW:!SEED:!CAMELLIA:!IDEA:!PSK:!SRP:!SSLv2") != 1)
            throw "SSL_CTX_set_cipher_list()";

		if (pKeyFile && certFile) {
			if (SSL_CTX_use_certificate_file(pSSLCtx, certFile, SSL_FILETYPE_PEM) != 1)
				throw "SSL_CTX_use_certificate_file()";

			if (SSL_CTX_use_PrivateKey_file(pSSLCtx, pKeyFile, SSL_FILETYPE_PEM) != 1) 
				throw "SSL_CTX_use_PrivateKey_file()";

			if (caFile != NULL) {
				if (SSL_CTX_load_verify_locations(pSSLCtx, caFile, NULL) != 1)
					throw "SSL_CTX_load_verify_locations()";
			}
			if (SSL_CTX_check_private_key(pSSLCtx) != 1) 
				throw "SSL_CTX_check_private_key()";
		}
	} catch (const char *str) {
		L(LOG_ERROR, "SocketSSL::createServerCTX(): error: %s\n", str);
		if (pSSLCtx != NULL)
			SSL_CTX_free(pSSLCtx);
		return NULL;
	}


	SSL_CTX_set_mode(pSSLCtx, SSL_MODE_AUTO_RETRY);
	SSL_CTX_set_read_ahead(pSSLCtx, 1);
	
	if (timeout > 0)
		SSL_CTX_set_timeout(pSSLCtx, timeout);
	
	SSL_CTX_set_options(pSSLCtx, SSL_OP_MSIE_SSLV2_RSA_PADDING);

//	SSL_CTX_set_options(pSSLCtx, SSL_OP_SSLREF2_REUSE_CERT_TYPE_BUG);
//	SSL_CTX_set_options(pSSLCtx, SSL_OP_MICROSOFT_BIG_SSLV3_BUFFER);
//	SSL_CTX_set_options(pSSLCtx, SSL_OP_SSLEAY_080_CLIENT_DH_BUG);
//	SSL_CTX_set_options(pSSLCtx, SSL_OP_TLS_D5_BUG);
//	SSL_CTX_set_options(pSSLCtx, SSL_OP_TLS_BLOCK_PADDING_BUG);
	SSL_CTX_set_options(pSSLCtx, SSL_OP_ALL);
	SSL_CTX_set_options(pSSLCtx, SSL_OP_CIPHER_SERVER_PREFERENCE);

//	SSL_CTX_set_options(pSSLCtx, SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS);

	return pSSLCtx;
}

bool HttpsEvent::initSSL(const char *pKeyFile, const char *certFile, const char *caFile, time_t timeout)
{
	/* Load encryption & hashing algorithms for the SSL program */
	SSL_library_init();

	/* Load the error strings for SSL & CRYPTO APIs */
	SSL_load_error_strings();

	/* Ignore Broken Pipe signal */
	signal(SIGPIPE, SIG_IGN);

	_lock_cs = (pthread_mutex_t *) OPENSSL_malloc(CRYPTO_num_locks() * sizeof(pthread_mutex_t));
	for (int i = 0; i < CRYPTO_num_locks(); i++)
		pthread_mutex_init(&(_lock_cs[i]), NULL);

	CRYPTO_set_id_callback(_pthreads_thread_id);
	CRYPTO_set_locking_callback(_pthreads_locking_callback);

	_sslContext = _createCTX(pKeyFile, certFile, caFile, timeout);
	if (_sslContext)
		SSLCache::addSSLCtx(_sslContext);
	return _sslContext != NULL;
}

bool HttpsEvent::restartSSL(const char* pKeyFile, const char* certFile, const char* caFile, time_t timeout)
/* Have to reconfigure SSLCache too */
{
	SSL_CTX* newSSLContext = NULL;
	
	newSSLContext = _createCTX(pKeyFile, certFile, caFile, timeout);
	if (newSSLContext) {
		SSL_CTX* tempContext = _sslContext;
		_sslContext = newSSLContext;
		SSLCache::addSSLCtx(_sslContext);
		if (tempContext) {
			SSL_CTX_free(tempContext);
		}		
	}
	return newSSLContext != NULL;
}

bool HttpsEvent::_ReceiveQuery()
{
	HttpsSocket::EResult res = _buf->recv(_ssl);
	if ((res == HttpsSocket::RES_ERROR) || (res == HttpsSocket::RES_CONNECTION_CLOSE))
		return false;

	_readChunk++;
	if (_readChunk > MAX_CHUNK_COUNT)
	{
		LU(LOG_WARNING, "Too many chunks %u\n", _readChunk);
		return false;
	}

	if (_buf->Length() > 1)
	{
		char *beginQueryBuf = *_buf;
		if (!_lastParseCharPos)
		{
			if ((beginQueryBuf[0] != 'G') && (beginQueryBuf[0] != 'P') && (beginQueryBuf[0] != 'H'))
			{
				LU(LOG_WARNING, "Bad request\n");
				return false;
			}
		}
		beginQueryBuf += _lastParseCharPos;

		
		for ( ; _lastParseCharPos < _buf->Length(); _lastParseCharPos++)
		{
			if ((*beginQueryBuf == *_curEndChar))
			{
				if (*_curEndChar == '\n')
				{
					char *endHeader = beginQueryBuf - 1; // - \r\n
					if (!_firstHeaderCharPos) // query
					{
						if (!_ParseURI(*_buf, endHeader, true))
							return false;
					}
					else if (!_ParseHeader(*_buf + _firstHeaderCharPos, endHeader))
						return false;
					_firstHeaderCharPos = _lastParseCharPos + 1;
				}
				_curEndChar++;
				if (!*_curEndChar) // \r\n\r\n
					break;
			}
			else
				_curEndChar = endChars;
			beginQueryBuf++;
		}
		if (!*_curEndChar) // find end query
		{
			_curState = RECEIVE_QUERY;
			return true;
		}
	}

	if (!_buf->Length() && res != HttpsSocket::RES_PROGRESS) // dont receive anything, ex. Chrome's cert checking
	{
		L(LOG_WARNING, "Empty request\n");
		return false;
	}
	return true;
}

bool HttpsEvent::_TrySendAnswer()
{
	HttpsSocket::EResult res = _buf->send(_ssl);
	if (res == HttpsSocket::RES_OK)
	{
		_curState = SHUTDOWN;
		return true;
	}
	else if (res != HttpsSocket::RES_PROGRESS)
	{
		LU(LOG_WARNING, "[UE] Send data error\n");
		EndWork();
		return true;
	}
	return false;
}

void HttpsEvent::SendErrorPage()
{
	_buf->Null();
	_buf->Add(_errorPage.Data(), _errorPage.Length());
	_buf->AddEndZero();
	if (!_TrySendAnswer())
		_curState = SEND_ANSWER;
}

void HttpsEvent::freeSSL()
{
	if (_sslContext)
	{
		SSL_CTX_free(_sslContext);
	}
}

HttpsEvent::HttpsEvent(SocketClient *client, class EHttpThread *thread, HttpEventListener *listener):
	HttpEvent(client, thread, listener), _handshakeRetry(0), _buf(NULL)
{
	_curState = DO_HANDSHAKE;
	AddEvent(EPOLLIN | EPOLLERR | EPOLLHUP);
	_ssl = SSL_new(_sslContext);
	if (_ssl)
	{
		if (SSL_set_fd(_ssl, client->GetDescr()))
		{
			SSL_set_accept_state(_ssl);
			_buf = HttpsSocket::get(MAX_QUERY_CHUNK);
		}
		else
			_curState = END_WORK;
	}
	else
		_curState = END_WORK;
}

HttpsEvent::~HttpsEvent(void)
{
	EndWork();
}

void HttpsEvent::EndWork()
{
	_curState = END_WORK;
	if (_client)
	{
		delete _client;
		_client = NULL;
		delete _listener;
		HttpsSocket::free(_buf);
		_buf = NULL;
		SSL_free(_ssl);
		_ssl = NULL;
	}
}

void HttpsEvent::Info(RString &buf, Time &curTime)
{
	if (_client)
	{
		_client->GetIPStr(buf);
	}
	else
	{
		//buf.snaprintf("Ended");
		return;
	}
	buf.snaprintf("] %u %u (%u)", _curState, _descr, curTime.Unix() - _lastOpTime);
	buf.snaprintf("\n<br>");
}

bool HttpsEvent::_Shutdown()
{
	if (_handshakeRetry > MAX_HANDSHAKE_RETRY_COUNT)
		return false;
	++_handshakeRetry;
	if (_buf->shutdown(_ssl) == HttpsSocket::RES_PROGRESS)
		return true;
	return false;
}

bool HttpsEvent::_Handshake()
{
	if (_handshakeRetry > MAX_HANDSHAKE_RETRY_COUNT)
		return false;
	++_handshakeRetry;
	HttpsSocket::EResult res = _buf->handshake(_ssl);
	if (res == HttpsSocket::RES_PROGRESS)
		return true;
	if (res == HttpsSocket::RES_OK)
	{
		_handshakeRetry = 0;
		_curState = WAIT_QUERY;
		return true;
	}
	return false;
}

void HttpsEvent::_checkIORequests()
{
	int req = _buf->getIORequest();
	ClearEvent(EPOLLOUT);
	AddEvent(EPOLLIN);
	if (req == HttpsSocket::REQ_WANT_WRITE || _curState == SHUTDOWN)
	{
		AddEvent(EPOLLOUT);
	}
	_thread->Ctrl(this);
}

void HttpsEvent::sendAnswer()
{
	if (_curState == END_WORK)
		return;
	if (!_TrySendAnswer())
	{
		if (_curState != SEND_N_CLOSE)
			_curState = SEND_ANSWER;
		ClearEvent(EPOLLIN);
		AddEvent(EPOLLOUT);
		_thread->Ctrl(this);
	}
}

bool HttpsEvent::_proccesSSL()
{
	if (_curState == DO_HANDSHAKE)
	{
		if(!_Handshake())
		{
			EndWork();
			return false;
		}
		if (_curState != WAIT_QUERY)
			return true;
	}
	if (_curState == WAIT_QUERY)
	{
		_UpdateOpTime();
		if (!_ReceiveQuery())
		{
			SendErrorPage();
			return true;
		}
		if (_curState != RECEIVE_QUERY)
			return true;
	}
	if (_curState == RECEIVE_QUERY)
	{
		_UpdateOpTime();
		HttpEventListener::EFormAnswer res = _listener->FormAnswer(*_buf, this);
		if (res == HttpEventListener::ANS_ERROR)
		{
			SendErrorPage();
			return true;
		}
		else if (res == HttpEventListener::ANS_SEND_N_CLOSE)
		{
			_curState = SEND_N_CLOSE;
			sendAnswer();
			return true;
		}
		else if (res == HttpEventListener::ANS_WAIT_RESULT)
		{
			_curState = WAIT_ANSWER_RESULT;
			ClearEvent(EPOLLIN | EPOLLOUT);
			_thread->Ctrl(this);
			return true;
		}
		else if (res != HttpEventListener::ANS_WAIT_TIMER)
		{
			sendAnswer();
			return true;
		}
	}
	
	if (_curState == SEND_ANSWER)
	{
		if(!_TrySendAnswer())
			return true;
	}
	if (_curState == SHUTDOWN)
	{
		if (!_Shutdown())
		{
			EndWork();
			return false;
		}
	}
	return true;
}

void HttpsEvent::Call(int events)
{
	if (_curState == END_WORK)
		EndWork();
	if ((events & EPOLLHUP) == EPOLLHUP)
	{
		EndWork();
		return;
	}

	if ((events & EPOLLERR) == EPOLLERR)
	{
		EndWork();
		return;
	}

	if ( ((events & EPOLLIN) == EPOLLIN) || ((events & EPOLLOUT) == EPOLLOUT) ) // read user request
	{
		if (_curState == WAIT_ANSWER_TIMER)
		{
			if (_timeOutDescr != 0)
			{
				close(_timeOutDescr);
				_timeOutDescr = 0;
				if (!_listener->callTimer(this))
				{
					SendErrorPage();
					return;
				}
			}
		}
		else 
		{
			if (!_proccesSSL())
				return;
		}
	}

	if (_curState != END_WORK)
	{
		_checkIORequests();
	}
}

}

