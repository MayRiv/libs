#pragma once

#include <vector>
#include "string.hpp"
#include "storage.hpp"
#include "resolv.hpp"

namespace lb
{
class Spf
{
public:
	Spf(const std::string &domain, Storage &st);
	Spf(const char *spfNets, int len);
	bool CheckIn(const Word32 ip);
	static const int MAX_REQ_LEVEL = 5;
	static const Word32 MAX_NET_MASK = 32;
	static const Word32 NetMask[MAX_NET_MASK+1];
	void FormSqlString(RString &buf);
	const bool Empty() const
	{
		return _nets.empty();
	}
private:
	
	void _AddNets(const std::string &domain, Resolv &rv, Resolv::TxtRequest &txtReq, Storage &st, int level);
	class Ip4
	{
	public:
		Ip4(const Word32 net, const Word32 mask)
			: net(net), mask(mask)
		{
		}
		Word32 net;
		Word32 mask;
	};
	typedef std::vector<Ip4> TIp4List;
	TIp4List _nets;
};
};
