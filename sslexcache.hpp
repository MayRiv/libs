#pragma once

#include <boost/unordered_map.hpp>
#include <openssl/ssl.h>

#include "mutexsync.hpp"
#include "log.hpp"
#include "time.hpp"
#include <list>


namespace lb
{

class SSLCache
{
public:
	static void initCache(size_t max_cache_size, size_t max_session_size);
	static bool addSSLCtx(SSL_CTX *ctx);
	static void destroyCache();// after method call, cache can be initialized by initCache again
	static void clear();
	static void removeExpired();

protected:

	static size_t _max_cache_size;
	static size_t _max_session_size;
	static const size_t AVERAGE_SESSION_SIZE;

	typedef unsigned char TSessionStorageBaseType;
	struct SessionValue
	{
		std::vector<TSessionStorageBaseType> session;
		time_t expiredAt;
	};

	inline static Word64 _generateKey(SSL_CTX* ctx, TSessionStorageBaseType* data, int len)
	{
		Word64 key = string_chsum_64((const char*)data, len);
		key = string_chsum_64((const char*)ctx->sid_ctx, ctx->sid_ctx_length, key);
		return key;
	}


	static int _new_session(SSL *ssl, SSL_SESSION *sess);
	static void _remove_session(SSL_CTX *ctx, SSL_SESSION *sess);
	static SSL_SESSION *_get_session(SSL *ssl, TSessionStorageBaseType *data, int len, int *copy);
	static void _info(const SSL *ssl, int where, int ret);


	typedef std::list<SessionValue*> TFreeSessionList;
	static  TFreeSessionList _freeSessions;
	typedef boost::unordered_map<Word64, SessionValue*> TSessionMap; 
	static TSessionMap *_cache;
	static MutexSyncData _sync;
};

}


