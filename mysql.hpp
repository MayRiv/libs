#pragma once

#include "xtypes.hpp"
#include <mysql/mysql.h>
#include <stdlib.h>
#include <limits.h>
#include <inttypes.h>

namespace lb
{
	const int E_LOG		= 2;
	const int E_EXIT  = 4;


	const int LOST_CONNECTION = 2013;
	const int ER_LOCK_WAIT_TIMEOUT = 1205;

	class MysqlRes
	{
	public:
		MysqlRes(MYSQL_RES *res);
		MYSQL_ROW Next();
		void Reset();

		~MysqlRes();

		int GetInt(int id)
		{
			return atoi(_curRow[id]);
		}

		float GetFloat(int id)
		{
			return atof(_curRow[id]);
		}

		Word32 GetWord32(int id)
		{
			if (_curRow[id])
				return strtoul(_curRow[id], NULL, 10);
			else
				return 0;
		}

		Word64 GetWord64(int id)
		{
			if (_curRow[id])
				return strtoull(_curRow[id], NULL, 10);
			else
				return 0;
		}

		Int64 GetInt64(int id)
		{
			if (_curRow[id])
				return strtoll(_curRow[id], NULL, 10);
			else
				return 0;
		}

		double GetDouble(int id)
		{
			if (_curRow[id])
				return atof(_curRow[id]);
			else
				return 0;
		}

		char *Get(int id)
		{
			return _curRow[id];
		}

		template <typename R>
    R get(int id) { return get_value_impl<R>::apply(this, id); }

		template<typename R, typename = void>
    struct get_value_impl
    {
    };

		template<typename S>
    struct get_value_impl<Byte, S>
    {
        static Byte apply(MysqlRes *a, int id)
				{ 
					if (a->_curRow[id])
						return strtoul(a->_curRow[id], NULL, 10);
					else
						return 0;
				}
    };

		template<typename S>
    struct get_value_impl<Word16, S>
    {
        static Word16 apply(MysqlRes *a, int id)
				{ 
					if (a->_curRow[id])
						return strtoul(a->_curRow[id], NULL, 10);
					else
						return 0;
				}
    };


		template<typename S>
    struct get_value_impl<Word32, S>
    {
        static Word32 apply(MysqlRes *a, int id)
				{ 
					if (a->_curRow[id])
						return strtoul(a->_curRow[id], NULL, 10);
					else
						return 0;
				}
    };

		template<typename S>
    struct get_value_impl<Word64, S>
    {
        static Word64 apply(MysqlRes *a, int id)
				{ 
					if (a->_curRow[id])
						return strtoull(a->_curRow[id], NULL, 10);
					else
						return 0;
				}
    };

		template<typename S>
    struct get_value_impl<int, S>
    {
        static int apply(MysqlRes *a, int id)
				{ 
					if (a->_curRow[id])
						return atoi(a->_curRow[id]);
					else
						return 0;
				}
    };

		template<typename S>
    struct get_value_impl<double, S>
    {
        static double apply(MysqlRes *a, int id)
				{ 
					if (a->_curRow[id])
						return atof(a->_curRow[id]);
					else
						return 0;
				}
    };



		Word32 GetLength(int field);
		Word32 GetCount();

		const char *FetchFieldName()
		{
			MYSQL_FIELD *field = mysql_fetch_field(_resSet);
			return field ? field->name : NULL;
		}

	private:
		MYSQL_RES *_resSet;
		MYSQL_ROW _curRow;
	};

	/**
		Mysql lib wrapp
	*/
	class Mysql
	{
	public:
		Mysql();
		int Connect(const char *hostName, const char *userName, const char *password, const char *dbName, const unsigned int port, const char *socketName=NULL, const unsigned int flags = 0);
		~Mysql();
		MysqlRes *Q(const char *query, int fAbort=E_LOG) const;
		MysqlRes *QUse(const char *query,	int fAbort=E_LOG) const;
		int U(const char *query, int fAbort=E_LOG) const;
		unsigned long InsertID() const;
		unsigned long AffectedRows() const;

		const char *GetCharSet() const;
		const int SetCharSet (const char *locale);
		void GetCharSetInfo (MY_CHARSET_INFO *cs);
		
		unsigned long RealEscapeString(char *to, const char *from, unsigned long length)
		{
			return mysql_real_escape_string(_conn, to, from, length);
		}
		void AddRealEscapeString(class RString &buf, const char *value, const long length);
		int setCharset(const char *charset);
		static void setDefaultCharset(const char *charset);

#if (MYSQL_VERSION_ID > 50000)
		unsigned long EscapeString (char *to, const char *from, unsigned long length);
#endif // (MYSQL_VERSION_ID > 50000)

	private:
		static const char *_defaultCharset;
		MYSQL *_conn;
		bool _TryReQuery(const char *query) const;
	};
};

