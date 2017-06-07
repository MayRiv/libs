#include <errno.h>
#include <netdb.h>
#include "resolv.hpp"
#include "log.hpp"

using namespace lb;


void Resolv::MxRequest::Parse(Storage &st, const Word32 mLen)
{
	Word16 weight;
	st.Get(&weight);
	weight = ntohs(weight);
	Byte *msg = st.Attach(mLen - sizeof(weight));
	char expanded[2*NS_MAXDNAME+1];
	int cs = dn_expand(st.Data(), st.Data() + st.Size(), msg, expanded, 2*NS_MAXDNAME);;
	if (cs < 0)
		throw Resolv::Error();
	_mxs.push_back(MxRecord(std::string(expanded), weight));
}

void Resolv::PtrRequest::Parse(Storage &st, const Word32 mLen)
{
	Byte *msg = st.Attach(mLen);
	char expanded[2*NS_MAXDNAME+1];
	int cs = dn_expand(st.Data(), st.Data() + st.Size(), msg, expanded, 2*NS_MAXDNAME);;
	if (cs < 0)
		throw Resolv::Error();
	_ptr.Set(expanded);
}
void Resolv::TxtRequest::Parse(Storage &st, const Word32 mLen)
{
	Byte *msg = st.Attach(mLen);
	_txt.Null();

	Word32 curOff = 0;
	while (curOff < mLen)
	{
		Byte strlen =  msg[curOff];
		_txt.Add((char*)(msg+curOff+1), strlen);
		curOff += strlen + 1;
	}
	_txt.AddEndZero();
}

void Resolv::Get(const std::string &domain, Request &req, Storage &st)
{
	AutoResState autoResState;

	st.Start();
	Byte *buf = st.AddAttach(MAX_ANSWER_SIZE);
	int res = res_nsearch(autoResState.Res(), domain.c_str(), C_IN, req.GetReq(), buf, st.Size());
	if (res < 0)
	{
		if (h_errno == TRY_AGAIN) // 
			throw TryAgain();
		else if (h_errno == NO_DATA) // no requeted data
			throw NoData();
		else if (h_errno == HOST_NOT_FOUND)
			throw NotFound();
		else
			throw Error();
	}

	if (res < (int)sizeof(HEADER))
	{
		L(LOG_ERROR, "Res too small %d < %d\n", res,  sizeof(HEADER));
		throw Error();
	}
	try
	{
		st.RollBack(res);
		st.Begin();

		HEADER *h =(HEADER*)st.Attach(sizeof(HEADER));
		Byte *end = st.Data() + st.Size();
		Word16 qCount = ntohs(h->qdcount);
		for (Word32 i = 0;  i < qCount; i++)
		{
			int len = dn_skipname(st.GetPos(), end);
      if (len < 0)
      {
				L(LOG_ERROR, "Can't dn_skipname in question\n");
				throw Error();
			}
      else if (len > 0)
				st.Skip(len);
			Word16 type;
			st.Get(&type);
			type = ntohs(type);
			Word16 qclass;
			st.Get(&qclass);
			qclass = ntohs(qclass);
			if ((qclass != C_IN) && (type != req.GetReq()))
			{
				L(LOG_ERROR, "Get answer with bad question\n");
				throw Error();
			}
		}

		Word32 anCount = ntohs(h->ancount);
		for (Word32 i = 0;  i < anCount; i++)
		{
			int len = dn_skipname(st.GetPos(), end);
			if (len < 0) 
			{
				L(LOG_ERROR, "Can't dn_skipname in answer\n");
				throw true;
			}
			else if (len > 0)
				st.Skip(len);
			Word16 type;
			st.Get(&type);
			type = ntohs(type);
			Word16 qclass;
			st.Get(&qclass);
			qclass = ntohs(qclass);
			Word32 ttl;
			st.Get(&ttl);
			ttl = ntohl(ttl);
			Word16 mLen;
			st.Get(&mLen);
			mLen = ntohs(mLen);
			if (mLen <= 0)
			{
				L(LOG_ERROR, "Bad mLen\n");
				throw true;
			}
			if (type == req.GetReq())
			{
				req.SetTTL(ttl);
				req.Parse(st, mLen);
			}
			else
				st.Skip(mLen);
		}
	}
	catch (Storage::Error &er)
	{
	}
}
