#pragma once

#include <vector>

#include "xtypes.hpp"
#include "mutexsync.hpp"
#include "storage.hpp"
#include "socket.hpp"

using namespace lb;

typedef std::vector<class DataManagerServer*> TDataManagerServerList;
class DataManagerServer : public MutexSyncData
{
public:
	enum EDataManagerCMD
	{
		MAIL_UNSEEN = 1,
		IIM_CMD,
	};

	static const Word32 MAX_WAIT_TIMEOUT = 10;
	static const Word32 MAX_SEND_TIMEOUT = 10;
	static const Word32 ERROR_RETRY_TIME = 20;

	static void InitDatamanager()
	{
#ifdef _DEV
		_dataServers.push_back(new DataManagerServer(SocketClient::GetIPStr("192.168.0.31"), 9987));
		_dataServers.push_back(new DataManagerServer(SocketClient::GetIPStr("192.168.0.31"), 9988));
#else
		_dataServers.push_back(new DataManagerServer(SocketClient::GetIPStr("10.0.0.12"), 9986));
		_dataServers.push_back(new DataManagerServer(SocketClient::GetIPStr("10.0.0.10"), 9986));
#endif
	}

	static void InitShowDatamanager()
	{
#ifdef _DEV
		_dataShowServers.push_back(new DataManagerServer(SocketClient::GetIPStr("192.168.0.31"), 8987));
		_dataShowServers.push_back(new DataManagerServer(SocketClient::GetIPStr("192.168.0.31"), 8986));
#else
		_dataShowServers.push_back(new DataManagerServer(SocketClient::GetIPStr("10.0.0.12"), 8986));
		_dataShowServers.push_back(new DataManagerServer(SocketClient::GetIPStr("10.0.0.10"), 8986));
#endif
	}

	static void SendCmdToAll(class Storage &st);
	static DataManagerServer *GetServer(unsigned int key, Word32 time);

	DataManagerServer(const Word32 ip, const Word32 port)
		: _ip(ip), _port(port), _conn(NULL), _errorTime(0)
	{
	}
	
	~DataManagerServer()
	{
		Close();
	}

	const bool IsActive() const
	{
		return _conn != NULL;
	}

	void Close()
	{
		if (_conn) {
			delete _conn;
			_conn = NULL;
		}
	}

	void SetNotActive()
	{
		Close();
	}

	const Word32 Port() const
	{
		return _port;
	}

	bool Connect();
	bool SendData(class Storage &st);
	char *IP();
	bool GetCmd(const unsigned int level, const unsigned int subLevel, const char *id, const int time, RString &buf);
	bool GetCmd(const unsigned int level, const unsigned int subLevel, const Word64 id, const int time, RString &buf);

private:
	static TDataManagerServerList _dataServers;
	static TDataManagerServerList _dataShowServers;

	Word32 _ip;
	Word32 _port;
	class SocketClient *_conn;
	Word32 _errorTime;

	bool _SendCmd(RString &buf);
};

