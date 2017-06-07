#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <memory>

#include "xtypes.hpp"
#include "string.hpp"
#include "tools.hpp"
#include "log.hpp"
#include "ips.hpp"
#include "storage.hpp"


using namespace lb;


int Ips::LoadTree(Mysql &sql, const char *tableName, const char *idName, const char *where)
{
	RString sqlBuf;
	enum EIpTable
	{
		FLD_AB,
		FLD_START,
		FLD_END,
		FLD_CITYID,
	};
	sqlBuf.snprintf("SELECT ab, cd_s, cd_e, %s FROM %s %s ORDER BY ab,cd_s", idName, tableName, where);
	std::auto_ptr<MysqlRes> res(sql.Q(sqlBuf));
	if (res.get() == NULL)
		return 0;

	Liner *liner;

	Word16 ab;
	int i = 0;
	int t = 0;
	while (res->Next())
	{
		ab = res->GetWord32(FLD_AB);									
		if ((liner = main.Find(ab)) != NULL)
		{
 			liner->Insert(new IP(res->GetWord32(FLD_START), res->GetWord32(FLD_END), res->GetWord32(FLD_CITYID)));			
		}
		else
		{
			liner  = new Liner(ab);
			liner->Insert(new IP(res->GetWord32(FLD_START), res->GetWord32(FLD_END), res->GetWord32(FLD_CITYID)));
			main.Add(liner, ab);
			t++;
		}
		i++;
	}
	L(LOG_WARNING, "Load %d ips %d trees\n", i, t);
	return 1;
}

Liner::~Liner()
{
	for (TIPList::iterator i = ips.begin(); i != ips.end(); ++i)
		delete (*i);
}

Ips::~Ips()
{
	for (ListPtrHash<Liner*, Word16> liner = main; liner; liner.Next())
		delete liner.Ptr();
}

int Ips::LoadAllInitData(Storage &st)
{
	Word32 c = 0;
	st.Get(&c);
	for (Word32 i = 0; i < c ; i++)
		Ips::LoadInitData(st);
	return 1;
}

int Ips::LoadInitData(Storage &st)
{
	Word32 c = 0;
	Word16 id = 0;
	st.Get(&c);
	st.Get(&id);
	Liner *liner  = new Liner(id);
	RegData data;
	for (Word32 i = 0; i < c; i++)
	{
		st.Get(&data);
		liner->Insert(new IP(data.start, data.end, data.regID));
	}
	main.Add(liner, id);
	return 1;
}

int Ips::GetAllInitData(Storage &st)
{
	Word32 c = 0;
	Word32 pos = st.Space(sizeof(Word32));
	for (TIpsList qIP = main; qIP; qIP.Next())
	{
		qIP->GetAllInitData(st);
		c++;
	}
	st.Set(pos, c);
	return st.Size();
}

int Liner::GetAllInitData(Storage &st)
{
	Word32 c = 0;
	Word32 pos = st.Space(sizeof(Word32));
	st.Add(id);
	RegData data;
	for (TIPList::const_iterator i = ips.begin(); i != ips.end(); ++i)
	{
		data.start = (*i)->start;
		data.end = (*i)->end;
		data.regID = (*i)->id;
		st.Add(data);
		c++;
	}
	st.Set(pos, c);
	return 1;
}

