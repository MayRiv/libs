#include <stdlib.h>
#include "storage.hpp"
#include "log.hpp"

#include "datamanagerapi.hpp"

TDataManagerServerList DataManagerServer::_dataServers;
TDataManagerServerList DataManagerServer::_dataShowServers;

bool DataManagerServer::Connect()
{
	if (_conn)
		return true;
	_conn = new SocketClient();
	if (_conn->Connect(_ip, _port, MAX_WAIT_TIMEOUT))
		return true;
	L(LOG_WARNING, "Can't connect to datamanager %s:%u\n", SocketClient::GetIPStr(_ip), _port);
	delete _conn;
	_conn = NULL;
	return false;
}

bool DataManagerServer::SendData(Storage &st)
{
	if (_conn->Send((char*)st.Data(), st.Size(), MAX_SEND_TIMEOUT) == (int)st.Size())
	{
		Word32 answer = 0;
		if (_conn->ReadBin((char*)&answer, sizeof(answer), MAX_WAIT_TIMEOUT) > 0)
			return answer == 1;
	}
	return false;
}

char *DataManagerServer::IP()
{
	return SocketClient::GetIPStr(_ip);
}

void DataManagerServer::SendCmdToAll(Storage &st)
{
	for (TDataManagerServerList::iterator i = _dataServers.begin(); i != _dataServers.end(); ++i)
	{
		DataManagerServer *dms = (*i);
		if (dms->Connect())
		{
			//L(LOG_WARNING, "Send %u sync to %s:%u\n", st.Size(), dms->IP(), dms->Port());
			if (!dms->SendData(st))
				L(LOG_WARNING, "Can't Send %u sync to %s\n", st.Size(), SocketClient::GetIPStr(dms->_ip));
			dms->SetNotActive();
		}
	}
}

DataManagerServer *DataManagerServer::GetServer(unsigned int key, Word32 time)
{
	key &= 0x7ffffff;
	for (size_t i = 0; i < _dataShowServers.size(); i++) {
		DataManagerServer *dms = _dataShowServers[(key + i) % _dataShowServers.size()];
		if (dms->IsActive())
			return dms;

		if (time < dms->_errorTime + ERROR_RETRY_TIME) {
			L(LOG_ERROR, "Datamanager %s retry time reach\n", dms->IP());
			continue;
		}

		dms->_errorTime = time;
		
		AutoMutexSyncData autoSync(dms);

		if (dms->Connect()) {
			dms->_errorTime = 0;
			return dms;
		}
	}

	return NULL;
}

bool DataManagerServer::GetCmd(const unsigned int level, const unsigned int subLevel, const Word64 id, const int time, RString &buf)
{
	buf.snprintf("G,%d,%d,%llx,%d\r\n\r\n", level, subLevel, id, time);
	LockData();
	bool res = _SendCmd(buf);
	UnLockData();
	return res;
}

bool DataManagerServer::GetCmd(const unsigned int level, const unsigned int subLevel, const char *id, const int time, RString &buf)
{
	buf.snprintf("G,%d,%d,%s,%d\r\n\r\n", level, subLevel, id, time);
	LockData();
	bool res = _SendCmd(buf);
	UnLockData();
	return res;
}

bool DataManagerServer::_SendCmd(RString &buf)
{
	if (!Connect())
		return false;

	if (_conn->Send(buf.Data(), buf.Length(), MAX_SEND_TIMEOUT) != buf.Length()) {
		L(LOG_ERROR, "[CHM] Close. Error when send cmd: %s\n", buf.Data());
		Close();
		return false;
	}

	if (!_conn->ReadBin(buf, 9, MAX_WAIT_TIMEOUT)) {
		L(LOG_WARNING, "[CHM] Retry. Error when read cmd length\n");
		Close();
		if (!Connect())
			return false;
		if (_conn->Send(buf.Data(), buf.Length(), MAX_SEND_TIMEOUT) != buf.Length() || !_conn->ReadBin(buf, 9, MAX_WAIT_TIMEOUT)) {
			L(LOG_ERROR, "[CHM] Close. Error when read cmd length: %s\n", buf.Data());
			Close();
			return false;
		}
	}

	if (!strncmp(buf.Data(), "ERR", 3))
		return false;

	buf.SetLength(8);
	int size = strtoul(buf.Data(), NULL, 16);
	buf.Null();
	if (_conn->ReadBin(buf.GetBuf(size + 1), size, MAX_WAIT_TIMEOUT) != size) {
		L(LOG_ERROR, "[CHM] Close. Error when read answer: %s\n", buf.Data());
		Close();
		return false;
	}

	return true;
}
