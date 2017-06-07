
#include "tools.hpp"
#include "sslexcache.hpp"

namespace lb
{

size_t SSLCache::_max_cache_size = 4096;
size_t SSLCache::_max_session_size = 4096;
const size_t SSLCache::AVERAGE_SESSION_SIZE = 256;
SSLCache::TFreeSessionList SSLCache::_freeSessions;
MutexSyncData SSLCache::_sync;

SSLCache::TSessionMap *SSLCache::_cache = NULL;

void SSLCache::initCache(size_t max_cache_size, size_t max_session_size)
{
	_max_session_size = max_session_size;
	_max_cache_size = max_cache_size;
	_cache = new TSessionMap(max_cache_size);
	//preallocate some sessions 
	while( _freeSessions.size() < max_cache_size >> 2)
	{
		_freeSessions.push_back(new SessionValue);
		_freeSessions.back()->session.resize(AVERAGE_SESSION_SIZE);
	}
}

bool SSLCache::addSSLCtx(SSL_CTX *ctx)
{
	if (!_cache)
		return false;
	Word32 session_ctx_id = rand();

	//Uncomment for force caching
	//SSL_CTX_set_options(ctx, SSL_OP_NO_TICKET);

	if (!SSL_CTX_set_session_id_context(ctx, (TSessionStorageBaseType*) &session_ctx_id, sizeof session_ctx_id))
		return false;
	SSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_SERVER | SSL_SESS_CACHE_NO_INTERNAL | SSL_SESS_CACHE_NO_AUTO_CLEAR);
	SSL_CTX_sess_set_cache_size(ctx, _max_cache_size);
	SSL_CTX_sess_set_new_cb(ctx, &SSLCache::_new_session);
	SSL_CTX_sess_set_remove_cb(ctx, &SSLCache::_remove_session);
	SSL_CTX_sess_set_get_cb(ctx, &SSLCache::_get_session);
	//SSL_CTX_set_info_callback(ctx, &_info);
	return true;
}
void SSLCache::destroyCache()
{
	clear();
	delete _cache;
	_cache = NULL;
}

void SSLCache::clear()
{
	//printf("clear all\n");
	AutoMutexSyncData amutex(&_sync);
	TSessionMap::iterator itSess = _cache->begin();
	for (; itSess != _cache->end(); ++itSess)
	{
		delete itSess->second;
	}
	_cache->clear();

	TFreeSessionList::iterator itLSess = _freeSessions.begin();
	for (; itLSess != _freeSessions.end(); ++itLSess)
	{
		delete *itLSess;
	}
	_freeSessions.clear();
}

void SSLCache::removeExpired()
{
	//printf("remove expired\n");
	AutoMutexSyncData amutex(&_sync);
	TSessionMap::iterator itSess = _cache->begin();
	time_t curTime = time(NULL);
	for (; itSess != _cache->end(); )
	{
		if (itSess->second->expiredAt <= curTime)
		{
			_freeSessions.push_back(itSess->second);
			itSess = _cache->erase(itSess);
		}
		else
			++itSess;
	}
}

void SSLCache::_info(const SSL *ssl, int where, int ret)
{	
}

SSL_SESSION *SSLCache::_get_session(SSL *ssl, TSessionStorageBaseType *data, int len, int *copy)
{
	//printf("get sess\n");
	SSL_SESSION *sessPtr = 0;
	Word64 key = _generateKey(ssl->ctx, data, len);
	*copy = 0;
	AutoMutexSyncData amutex(&_sync);
	TSessionMap::iterator itSess = _cache->find(key);
	if (itSess != _cache->end())
	{
		//printf("Session found\n");
		if (itSess->second->expiredAt < time(NULL))
		{
			//printf("Expired %d\n", itSess->second->expiredAt);
			// remove 
			_freeSessions.push_back(itSess->second);
			_cache->erase(key);
			return 0;
		}
		const TSessionStorageBaseType* dataPtr = &(itSess->second->session.front());
		sessPtr = d2i_SSL_SESSION(NULL, &dataPtr, itSess->second->session.size());
		if (!sessPtr)
		{
			L(LOG_ERROR, "SSL Session cache - can't convert saved session. Drop...\n");
			_freeSessions.push_back(itSess->second);
			_cache->erase(key);
			return 0;
		}
	}
	return sessPtr;
}

int SSLCache::_new_session(SSL *ssl, SSL_SESSION *_sess)
{
	//printf("new sess\n");
	AutoMutexSyncData amutex(&_sync);
	if (_cache->size() >= _max_cache_size)
	{
		L(LOG_ERROR, "SSL Session cache - cache is full\n");
		return 0;
	}
	size_t len = i2d_SSL_SESSION(_sess, 0);
	if (len >= _max_session_size || !len)
	{
		L(LOG_ERROR, "SSL Session cache - can't be stored: too large or session is invalid\n");
		return 0;
	}

	SessionValue* sess;
	if (_freeSessions.empty())
	{
		sess = new SessionValue;
	}
	else
	{
		sess = _freeSessions.front();
		_freeSessions.pop_front();
	}
	if (sess->session.size() < len)
		sess->session.resize(len);

	sess->expiredAt = _sess->timeout + time(NULL);
	TSessionStorageBaseType* dataPtr = &sess->session.front();
	if (!i2d_SSL_SESSION(_sess, &dataPtr))
	{
		L(LOG_ERROR, "SSL Session cache - can't be stored: conversion failed\n");
		_freeSessions.push_back(sess);
		return 0;
	}
	Word64 key = _generateKey(ssl->ctx, _sess->session_id, _sess->session_id_length);
        std::pair<Word64, SessionValue*> v(key, (SessionValue*)0);
	//std::pair< TSessionMap::iterator, bool> itSess = _cache->insert(TSessionMap::value_type(key, 0));  
	std::pair< TSessionMap::iterator, bool> itSess = _cache->insert(v);  
	if (!itSess.second)
	{
		L(LOG_WARNING, "SSL Session cache - already has session with such key. Overwrite...");
		_freeSessions.push_back(itSess.first->second);
	}
	itSess.first->second = sess;
	return 1;
}

void SSLCache::_remove_session(SSL_CTX *ctx, SSL_SESSION *sess)
{
	//L(LOG_ERROR,"remove sess\n");
	AutoMutexSyncData amutex(&_sync);
	Word64 key = _generateKey(ctx, sess->session_id, sess->session_id_length);
	TSessionMap::iterator itSess = _cache->find(key);
	if (itSess != _cache->end())
	{
		_freeSessions.push_back(itSess->second);
		_cache->erase(itSess);
	}
}

}

