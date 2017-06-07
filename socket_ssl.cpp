#include <sys/poll.h>
#include <signal.h>
#include <sys/socket.h>
//#include <openssl/err.h>
//#include <openssl/engine.h>
//#include <openssl/conf.h>

#include "socket_ssl.hpp"
#include "log.hpp"

using namespace lb;

bool SocketSSL::_inited = SocketSSL::_init();
pthread_mutex_t *SocketSSL::_lock_cs = NULL;

bool SocketSSL::_init()
{
	/* Load encryption & hashing algorithms for the SSL program */
	SSL_library_init();

	/* Load the error strings for SSL & CRYPTO APIs */
	SSL_load_error_strings();

	/* Ignore Broken Pipe signal */
	signal(SIGPIPE, SIG_IGN);

	/* Threads Safe */
	_lock_cs = (pthread_mutex_t *) OPENSSL_malloc(CRYPTO_num_locks() * sizeof(pthread_mutex_t));
	for (int i = 0; i < CRYPTO_num_locks(); i++)
		pthread_mutex_init(&(_lock_cs[i]), NULL);

	CRYPTO_set_id_callback(_pthreads_thread_id);
	CRYPTO_set_locking_callback(_pthreads_locking_callback);

	return true;
}

bool SocketSSL::_waitForSSL(const int sslErr, const int timeOut)
{
	int events = 0;
	if (sslErr == SSL_ERROR_WANT_READ) {
		events |= POLLIN;
	} else if (sslErr == SSL_ERROR_WANT_WRITE) {
		events |= POLLOUT;
	} else {
		return false;
	}

	return _waitFor(events, timeOut);
}

SSL_CTX *SocketSSL::createServerCTX(const char *pKeyFile, const char *certFile, const char *caFile)
{
	SSL_CTX *pSSLCtx = NULL;
	try {
		/* Create a SSL_METHOD structure (choose a SSL/TLS protocol version) */
		const SSL_METHOD *meth = SSLv23_server_method();

		/* Create a SSL_CTX structure */
		pSSLCtx = SSL_CTX_new(const_cast<SSL_METHOD *>(meth));
		if (pSSLCtx == NULL)
			throw "SSL_CTX_new()";

		SSL_CTX_set_session_cache_mode(pSSLCtx, SSL_SESS_CACHE_OFF);

		SSL_CTX_set_options(pSSLCtx, SSL_OP_NO_SSLv2);

		if (SSL_CTX_set_cipher_list(pSSLCtx, "RC4:AES128:AES256") != 1)
            throw "SSL_CTX_set_cipher_list()";

		if (pKeyFile && certFile) {
			/* Load the server certificate into the SSL_CTX structure */
			if (SSL_CTX_use_certificate_file(pSSLCtx, certFile, SSL_FILETYPE_PEM) != 1)
				throw "SSL_CTX_use_certificate_file()";

			/* Load the private-key corresponding to the server certificate */
			if (SSL_CTX_use_PrivateKey_file(pSSLCtx, pKeyFile, SSL_FILETYPE_PEM) != 1) 
				throw "SSL_CTX_use_PrivateKey_file()";

			if (caFile != NULL) {
				/* Load the server CA certificate into the SSL_CTX structure */
				if (SSL_CTX_load_verify_locations(pSSLCtx, caFile, NULL) != 1)
					throw "SSL_CTX_load_verify_locations()";
			}

			/* Check if the server certificate and private-key matches */
			if (SSL_CTX_check_private_key(pSSLCtx) != 1) 
				throw "SSL_CTX_check_private_key()";
		}
	} catch (const char *str) {
		L(LOG_ERROR, "SocketSSL::createServerCTX(): error: %s\n", str);
		if (pSSLCtx != NULL)
			SSL_CTX_free(pSSLCtx);
		return NULL;
	}

	return pSSLCtx;
}

int SocketSSL::open(const unsigned int ip, short int port, const int timeOut)
{
	if (!TransportBase::open(ip, port, timeOut))
		return 0;

	try {
		/* Create a SSL_METHOD structure (choose a SSL/TLS protocol version) */
		const SSL_METHOD *meth = SSLv23_client_method();

		/* Create a SSL_CTX structure */
		SSL_CTX *pSSLCtx = SSL_CTX_new(const_cast<SSL_METHOD *>(meth));
		if (pSSLCtx == NULL)
			throw "SSL_CTX_new()";

		_pSSL = SSL_new(pSSLCtx);
		
		SSL_CTX_free(pSSLCtx);

		if (_pSSL == NULL)
			throw "SSL_new()";
	
		/* Assign the socket into the SSL structure (SSL and socket without BIO) */
		if (SSL_set_fd(_pSSL, _descr) != 1)
			throw "SSL_set_fd()";

		/* Perform SSL Handshake */
		int ret;
		while ((ret = SSL_connect(_pSSL)) != 1) {
			int sslErr = SSL_get_error(_pSSL, ret);
			if (!_waitForSSL(sslErr, timeOut))
				throw "SSL_connect()";
			L(LOG_WARNING, "[SSL] SSL_connect Handshake; ret: %d, sslErr: %d\n", ret, sslErr);
		}
	} catch (const char *str) {
		L(LOG_ERROR, "SocketSSL::open(): error: %s\n", str);
		close();
		return 0;
	}

	return 1;
}

int SocketSSL::close()
{
	if (_pSSL != NULL) {
		size_t retry = 6;
		while (SSL_shutdown(_pSSL) == 0) {
			if (retry-- <= 0) {
				L(LOG_WARNING, "[SSL] SSL_shutdown Handshake limit excited\n");
				break;
			}
		}
		SSL_free(_pSSL);
		_pSSL = NULL;
		//CONF_modules_free();
		//ERR_remove_state(0);
		//ENGINE_cleanup();
		//CONF_modules_unload(1);
		////ERR_free_strings();
		////EVP_cleanup();
		////CRYPTO_cleanup_all_ex_data();
	}

	return TransportBase::close();
}

SSL *SocketSSL::createSSL(SSL_CTX *pSSLCtx)
{
	_pSSL = SSL_new(pSSLCtx);
	if (_pSSL == NULL)
		L(LOG_ERROR, "SocketSSL::createSSL(): error: SSL_new()\n");

	return _pSSL;
}

int SocketSSL::accept(const int timeOut)
{
	try {
		if (_pSSL == NULL)
			throw "SSL not created";

		struct timeval tv;
		tv.tv_sec = timeOut;
		tv.tv_usec = 0 ;
		if (setsockopt(_descr, SOL_SOCKET, SO_RCVTIMEO, (char *) &tv, sizeof tv) == -1)
			throw "setsockopt(SO_RCVTIMEO) error";
		if (setsockopt(_descr, SOL_SOCKET, SO_SNDTIMEO, (char *) &tv, sizeof tv) == -1)
			throw "setsockopt(SO_SNDTIMEO) error";
	
		/* Assign the socket into the SSL structure (SSL and socket without BIO) */
		if (SSL_set_fd(_pSSL, _descr) != 1)
			throw "SSL_set_fd()";

		/* Perform SSL Handshake */
		int ret;
		while ((ret = SSL_accept(_pSSL)) != 1) {
			int sslErr = SSL_get_error(_pSSL, ret);
			if (!_waitForSSL(sslErr, timeOut))
				throw "SSL_accept()";
			L(LOG_WARNING, "[SSL] SSL_accept Handshake; ret: %d, sslErr: %d\n", ret, sslErr);
		}

//		const SSL_CIPHER *cipher = SSL_get_current_cipher(_pSSL);
//		if (cipher != NULL) {
//			L(LOG_INFO, "[SSL] accept: protocol: %s, cipher: %s, version: %s\n", SSL_get_version(_pSSL), SSL_CIPHER_get_name(cipher), SSL_CIPHER_get_version(cipher));
//		} else {
//			L(LOG_ERROR, "[SSL] accept: protocol: %s, cipher: NULL\n", SSL_get_version(_pSSL));
//		}
	} catch (const char *str) {
		L(LOG_ERROR, "SocketSSL::accept(): error: %s\n", str);
		close();
		return 0;
	}

	return 1;
}

int SocketSSL::_read(char *buf, const size_t len)
{
	/* Perform SSL Handshake */
	int ret;
	while ((ret = SSL_read(_pSSL, buf, len)) <= 0) {
		int sslErr = SSL_get_error(_pSSL, ret);
		if (!_waitForSSL(sslErr, SOCK_OP_TIMEOUT))
			break;
//		L(LOG_WARNING, "[SSL] SSL_read Handshake; ret: %d, sslErr: %d\n", ret, sslErr);
	}
	
	return ret;
	//return SSL_read(_pSSL, buf, len);
}

int SocketSSL::_write(const char *buf, const size_t len)
{
	/* Perform SSL Handshake */
	int ret;
	while ((ret = SSL_write(_pSSL, buf, len)) <= 0) {
		int sslErr = SSL_get_error(_pSSL, ret);
		if (!_waitForSSL(sslErr, SOCK_OP_TIMEOUT))
			break;
		L(LOG_WARNING, "[SSL] SSL_write Handshake; ret: %d, sslErr: %d\n", ret, sslErr);
	}
	
	return ret;
//	return SSL_write(_pSSL, buf, len); 
}
