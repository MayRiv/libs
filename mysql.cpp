#include <stdlib.h>
#include <stdio.h>
#include <mysql.h>
#include <errmsg.h>
#include "log.hpp"
#include "mysql.hpp"
#include "string.hpp"

using namespace lb;

const char *Mysql::_defaultCharset = "CP1251";

void Mysql::setDefaultCharset(const char *charset)
{
	_defaultCharset = charset;
}


Mysql::Mysql()
{
	_conn = mysql_init (NULL);
	if (_conn == NULL)
	{
		L(LOG_CRITICAL, "Cannot mysql init\n");
		exit(-1);
	}
}

int Mysql::setCharset(const char *charset)
{
	if (mysql_set_character_set(_conn, charset))
	{
		L(LOG_ERROR, "ERROR setCharset %d:%s\n", mysql_errno(_conn), mysql_error(_conn));
		return 0;
	}
	else
		return 1;
}

int Mysql::Connect(const char *hostName, const char *userName, const char *password, const char *dbName, const unsigned int port, const char *socketName, const unsigned int flags)
{
	if (mysql_real_connect (_conn, hostName, userName, password,
			dbName, port, socketName, flags) == NULL)
	{
		L(LOG_ERROR, "ERROR %d:%s\n", mysql_errno(_conn), mysql_error(_conn));
		return 0;
	}
#if (MYSQL_VERSION_ID > 50000)
	mysql_options(_conn, MYSQL_OPT_RECONNECT, "true");
#endif
	return setCharset(_defaultCharset);
}

Mysql::~Mysql()
{
	mysql_close (_conn);
}

MysqlRes *Mysql::QUse(const char *query,	int fAbort) const
{
	int res = mysql_query(_conn, query);
  if (res && !_TryReQuery(query))
	{
		int merrno = mysql_errno(_conn);
		if (fAbort&E_LOG)
		{
			const char *merrstr = mysql_error(_conn);
			L(LOG_ERROR, "ERROR: [%s]: %d : %s\n", query, merrno, merrstr);
		}
		if (fAbort&E_EXIT)
			exit(1);
		return NULL;
	}
	MYSQL_RES *resSet = mysql_use_result(_conn);
	if (resSet)
		return new MysqlRes(resSet);
	else
	{
		L(LOG_ERROR, "ERROR: [%s]: %d : %s\n",query, mysql_errno(_conn), mysql_error(_conn));
		return NULL;
	}
}

bool Mysql::_TryReQuery(const char *query) const
{
	int merrno = mysql_errno(_conn);
	if (((merrno == ER_LOCK_WAIT_TIMEOUT) || (merrno == CR_SERVER_GONE_ERROR) || (merrno == CR_SERVER_LOST))
			&& !mysql_ping(_conn))
	{
		L(LOG_WARNING, "Reconected...Trying requery\n");
		int res = mysql_query(_conn, query);
		if (!res)
			return true;
	}
	return false;
}

MysqlRes *Mysql::Q(const char *query,	int fAbort) const
{
	int res = mysql_query(_conn, query);
  if (res && !_TryReQuery(query))
	{
		int merrno = mysql_errno(_conn);
		if (fAbort&E_LOG)
		{
			const char *merrstr = mysql_error(_conn);
			L(LOG_ERROR, "ERROR: [%s]: %d : %s\n", query, merrno, merrstr);
		}
		if (fAbort&E_EXIT)
			exit(1);
		return NULL;
	}
	MYSQL_RES *resSet = mysql_store_result(_conn);
	if (resSet)
		return new MysqlRes(resSet);
	else
	{
		L(LOG_ERROR, "ERROR: [%s]: %d : %s\n",query, mysql_errno(_conn), mysql_error(_conn));
		return NULL;
	}
}

int Mysql::U(const char *query, int fAbort ) const
{
	int res=mysql_query(_conn, query);
  if (res && !_TryReQuery(query))
	{
		int merrno = mysql_errno(_conn);
		if (fAbort&E_LOG)
		{
			const char *merrstr = mysql_error(_conn);
			L(LOG_ERROR, "ERROR: [%s]: %d : %s\n", query, merrno, merrstr);
		}
		if (fAbort&E_EXIT)
			exit(1);
		return 0;
	}
	return !res;
}

unsigned long  Mysql::AffectedRows() const
{
	return mysql_affected_rows(_conn);
}

unsigned long Mysql::InsertID() const
{
	return mysql_insert_id(_conn);
}

const char *Mysql::GetCharSet() const
{
	return mysql_character_set_name(_conn);
}

const int Mysql::SetCharSet (const char *locale)
{
	return mysql_set_character_set(_conn, locale);
}

void Mysql::GetCharSetInfo (MY_CHARSET_INFO *cs)
{
	mysql_get_character_set_info(_conn, cs);
}

#if (MYSQL_VERSION_ID > 50000)
unsigned long Mysql::EscapeString (char *to, const char *from, unsigned long length)
{
	return mysql_real_escape_string(_conn, to, from, length);
}
#endif // (MYSQL_VERSION_ID > 50000)

void Mysql::AddRealEscapeString(class RString &buf, const char *value, const long length)
{
	int curLen = buf.Length();
	curLen += mysql_real_escape_string(_conn, buf.GetBuf(length * 2  + 1), value, length);
	buf.SetLength(curLen);
}

MysqlRes::MysqlRes(MYSQL_RES *res)
	: _resSet(res), _curRow(NULL)
{
}
/*
MysqlRes::MysqlRes(MYSQL *conn, int useRes)
{
	_resSet=mysql_use_result (conn);
	_curRow=NULL;
}

MysqlRes::MysqlRes(MYSQL *conn)
{
	_resSet=mysql_store_result (conn);
	_curRow=NULL;
}*/

MYSQL_ROW MysqlRes::Next()
{
	_curRow = mysql_fetch_row(_resSet);
	return _curRow;
}

void MysqlRes::Reset()
{
	mysql_data_seek(_resSet, 0);
}


Word32 MysqlRes::GetLength(int field)
{
	unsigned long *lengths = mysql_fetch_lengths(_resSet);
	return lengths[field];
}
Word32 MysqlRes::GetCount()
{
	return mysql_num_rows(_resSet);
}

MysqlRes::~MysqlRes()
{
	mysql_free_result(_resSet);
}
