#pragma once
/* (C) Tera & Draal
 *  Class for mutex Mysql accesss
 *  MysqlSyncManager msm;
 *  if (!msm.Init(DB_HOST, DB_USER, DB_PASSWORD, DB_BASE, DB_PORT, 5))
 *  {
 *   printf("Can't run mysql\n");
 *   return -1;
 *  }
 *  MysqlSyncManager::TSharedMysqlSync sharedMysql = msm.get(1);
 *  Mysql &sql = sharedMysql->Sql();
 *  RString &buf = sharedMysql->Buf();
 *  buf.snprintf("SELECT id FROM videos WHERE id=1");
 *  sql.Q(buf);
*/


#include <vector>
#include <set>

#include "tr1.hpp"
#include "string.hpp"
#include "mysql.hpp"
#include "mutexsync.hpp"

namespace lb
{
	class MysqlSyncManager {
	private:
		class SyncMysql;

	public:
		class AutoMysqlSync {
		public:
			AutoMysqlSync(SyncMysql *ms)
				: _ms(ms)
			{
				_ms->LockData();
				_ms->_buf.Null();
			}

			~AutoMysqlSync()
			{
				if (_ms)
					_ms->UnLockData();
			}	

			void UnLockData()
			{
				if (_ms)
				{
					_ms->UnLockData();
					_ms = NULL;
				}
			}

			RString &Buf()
			{
				return _ms->_buf;
			}

			Mysql &Sql()
			{
				return _ms->_mysql;
			}
			MysqlRes *Q()
			{
				return _ms->_mysql.Q(_ms->_buf);
			}

		private:
			AutoMysqlSync()
			{
			}

			SyncMysql *_ms;
		};

		typedef std::tr1::shared_ptr<AutoMysqlSync> TSharedMysqlSync;
		typedef std::set<Word32> TKeySet;
		typedef std::vector<TSharedMysqlSync> TSharedMysqlSyncList;

		const bool Init(const char *hostName, const char *userName, const char *password, const char *dbName, const unsigned int port, const size_t maxConnection)
		{
			if (!_msList.empty()) {
				for (TSyncMysqlList::iterator m = _msList.begin(); m != _msList.end(); m++)
					delete (*m);
				_msList.clear();
			}
			for (size_t i = 0; i < maxConnection; i++)
			{
				SyncMysql *sm =  new SyncMysql();
				if (!sm->Init(hostName, userName, password, dbName, port))
				{
					delete sm;
					return false;
				}
				_msList.push_back(sm);
			}	
			return true;
		}

		size_t getKey(const Word32 key) const
		{
			return key % _msList.size();
		}

		TSharedMysqlSync get(const Word32 key)
		{
			return TSharedMysqlSync(new AutoMysqlSync(_msList[getKey(key)]));
		}

		TSharedMysqlSyncList get(const TKeySet &keySet)
		{
			TSharedMysqlSyncList list;
			for (TKeySet::const_iterator i = keySet.begin(); i != keySet.end(); i++)
				list.push_back(TSharedMysqlSync(new AutoMysqlSync(_msList[*i])));

			return list;
		}

	private:
		class SyncMysql : public MutexSyncData {
		public:
			const bool Init(const char *hostName, const char *userName, const char *password, const char *dbName, const unsigned int port)
			{
				return _mysql.Connect(hostName, userName, password, dbName, port);
			}

		private:
			friend class AutoMysqlSync;
			Mysql _mysql;
			RString _buf;
		};

		typedef std::vector<SyncMysql *> TSyncMysqlList;

		TSyncMysqlList _msList;
	};

};
