#pragma once

#include <vector>
#include "hash.hpp"
#include "xtypes.hpp"
#include "mysql.hpp"

namespace lb
{
	const Word32 MAX_IPS  = 4000;

	class IP
	{
	public:
		IP(Word16 start, Word16 end, Word32 id)
		{
			this->start = start;
			this->end = end;
			this->id = id;
			ipClass = this->end - this->start;
		}
		IP(Word16 ip)
		{
			this->start = ip;
			this->end = ip;
			ipClass = 0;
		}
		Word16 IpClass() { return ipClass; }
		Word16 start;
		Word16 end;
		Word16 ipClass;
		Word32 id;
		int Max(IP &mip)
		{
			if (mip.end>end)
				return 0;
			else
				return 1;
		}
		IP &operator=(Word32 ip)
		{
			this->start = ip;
			this->end = ip;
			return *this;
		}
		friend int operator<(IP &left, IP &right)
		{
			if (left.start <= right.start)
				return 1;
			else
				return 0;
		}
		friend int operator==(IP &left, IP &right)
		{
			if (right.start >= left.start && right.end <= left.end)
				return 1;
			else
				return 0;
		}
	};

	
	typedef std::vector<class IP*> TIPList;

	class Liner
	{
	public:
		Liner(int id)
		{
			this->id = id;
		}
		~Liner();
		void Insert(IP *ip)
		{
			ips.push_back(ip);
		}
		int Find(Word16 ip)
		{
			Word32 minclass = 0xFFFFFF;
			int id = 0;
			for (TIPList::const_iterator i = ips.begin(); i != ips.end(); ++i)
				if ((*i)->start > ip)
					break;
				else
				{
					if ((*i)->end >= ip)
					{
						if (((*i)->IpClass() < minclass) || (id==37))// if id=Ukraine find inside
						{
							minclass = (*i)->IpClass();
							id = (*i)->id;
						}
					}
				}
			return id;
		}
		Word16 Key() { return id; }
		int GetAllInitData(class Storage &st);
	protected:
		TIPList ips;
		Word16 id;
	};

	typedef ListPtrHash<Liner*, Word16> TIpsList;

	class Ips
	{
	public:
		int GetID(Word32 ip)
		{	
			Liner	*liner = main.Find(ip>>16);
			if (liner)
				return liner->Find(ip&0xFFFF);
			return 0;
				
		}
		~Ips();
		int LoadAllInitData(class Storage &st);
		int LoadInitData(class Storage &st);
		int GetAllInitData(class Storage &st);
		int LoadTree(class Mysql &sql, const char *tableName, const char *idName = "cityID", const char *where = "");
	protected:
		PtrHash<Liner*, Word16> main;
	};

	struct RegData
	{
		Word16 start;
		Word16 end;
		Word32 regID;
	};
};

