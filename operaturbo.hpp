#pragma once
#include <vector>
#include "socket.hpp"

namespace lb
{
	class OperaTurbo
	{
	public:
		static bool IsOperaIp(const Word32 ip)
		{
			static OperaTurbo ot;
			return ot.Find(ip);
		}
	private:
		struct IpRange
		{
			IpRange(const char *startIp, const char *endIp)
				: start(SocketClient::IpToLong(startIp)), end(SocketClient::IpToLong(endIp))
			{
			}
			Word32 start;
			Word32 end;
		};
		typedef std::vector<IpRange> TIpRangeVector;
		TIpRangeVector _ipranges;
		OperaTurbo()
		{
			_ipranges.push_back(IpRange("37.228.104.0", "37.228.107.255"));
			_ipranges.push_back(IpRange("54.244.56.0", "54.244.63.255"));//amazon aws - nokia
			_ipranges.push_back(IpRange("66.54.64.0", "66.54.95.255"));
			_ipranges.push_back(IpRange("66.249.64.0", "66.249.95.255"));//google proxy
			_ipranges.push_back(IpRange("70.39.184.176", "70.39.187.240"));//wangsu
			_ipranges.push_back(IpRange("80.232.117.0", "80.232.117.255"));
			_ipranges.push_back(IpRange("80.239.242.0", "80.239.243.255"));
			_ipranges.push_back(IpRange("82.145.208.0", "82.145.223.255"));
			_ipranges.push_back(IpRange("83.218.67.120", "83.218.67.127"));
			_ipranges.push_back(IpRange("87.105.182.64", "87.105.182.71"));
			_ipranges.push_back(IpRange("91.203.96.0", "91.203.99.255"));
			_ipranges.push_back(IpRange("94.246.126.0", "94.246.127.255"));
			_ipranges.push_back(IpRange("107.167.96.0", "107.167.127.255"));//opera us
			_ipranges.push_back(IpRange("131.228.29.1", "131.228.29.126"));//nokia proxy
			_ipranges.push_back(IpRange("137.135.176.0", "137.135.176.255"));//ms proxy
			_ipranges.push_back(IpRange("141.0.8.0", "141.0.15.255"));
			_ipranges.push_back(IpRange("168.235.194.0", "168.235.197.255"));//mileweb
			_ipranges.push_back(IpRange("185.26.180.0", "185.26.182.63"));
			_ipranges.push_back(IpRange("195.116.30.192", "195.116.30.255"));
			_ipranges.push_back(IpRange("195.189.142.0", "195.189.143.255"));
			_ipranges.push_back(IpRange("213.236.208.0", "213.236.208.255"));
			_ipranges.push_back(IpRange("217.17.35.152", "217.17.35.159"));
			_ipranges.push_back(IpRange("217.212.230.0", "217.212.231.255"));
		}
		const bool Find(const Word32 ip) const
		{
			for (TIpRangeVector::const_iterator r = _ipranges.begin(); r != _ipranges.end(); ++r)
			{
				if ((ip >= r->start) && (ip <= r->end))
					return true;
			}
			return false;
		}
	};
};
