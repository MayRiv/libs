#include <netdb.h>
#include <stdlib.h>
#include "spf.hpp"
#include "log.hpp"
#include "socket.hpp"

using namespace lb;


Spf::Spf(const std::string &domain, Storage &st)
{
	Resolv rv;
	Resolv::TxtRequest txtReq;
	_AddNets(domain, rv, txtReq, st, 0);
}

Spf::Spf(const char *spfNets, int len)
{
	char *pSpf = (char*)spfNets;
	while (*pSpf)
	{
		char *eSpf = NULL;
		Word32 net =  strtoul(pSpf, &eSpf, 16);
		if (eSpf && *eSpf == '/')
			pSpf = eSpf + 1;
		else
			break;
		Word32 mask =  strtoul(pSpf, &eSpf, 10);
		_nets.push_back(Ip4(net, NetMask[mask]));
		if (eSpf && *eSpf == ',')
			pSpf = eSpf + 1;
		else
			break;
	}
}


const char INCLUDE_STR[] = "include";
const int LEN_INCLUDE_STR = sizeof(INCLUDE_STR) - 1;
const char IP4_STR[] = "ip4";
const int LEN_IP4_STR = sizeof(IP4_STR) - 1;
const char REDIRECT_STR[] = "redirect";
const int LEN_REDIRECT_STR = sizeof(REDIRECT_STR) - 1;

const Word32 SINGL_HOST_MASK = 0xFFFFFFFF;

const Word32 Spf::NetMask[MAX_NET_MASK+1] = {0x0,0x80000000,0xc0000000,0xe0000000,0xf0000000,0xf8000000,0xfc000000,0xfe000000,0xff000000,0xff800000,0xffc00000,0xffe00000,0xfff00000,0xfff80000,0xfffc0000,0xfffe0000,0xffff0000,0xffff8000,0xffffc000,0xffffe000,0xfffff000,0xfffff800,0xfffffc00,0xfffffe00,0xffffff00,0xffffff80,0xffffffc0,0xffffffe0,0xfffffff0,0xfffffff8,0xfffffffc,0xfffffffe,0xffffffff};

void Spf::FormSqlString(RString &buf)
{
	if (!_nets.empty())
	{
		for (TIp4List::iterator n =  _nets.begin(); n != _nets.end(); ++n)
		{
			Word32 m = 0;
			for (; m <= MAX_NET_MASK; m++) 
				if (n->mask == NetMask[m])
					break;
			buf.snaprintf("%x/%u,", n->net, m);
		}
		buf.BackSpace();
	}
}

void Spf::_AddNets(const std::string &domain, Resolv &rv, Resolv::TxtRequest &txtReq, Storage &st, int level)
{
	typedef std::vector<std::string> TStringList;

	TStringList includeDomain;
	try
	{
		rv.Get(domain, txtReq, st);
	}
	catch (Resolv::NotFound &nf)
	{
		if (level == 0)
			throw nf;
		else
		{
			L(LOG_WARNING, "Recive resolv NotFound on %s in %u level\n", domain.c_str(), level);
			return;
		}
	}
	catch (Resolv::TryAgain &ta)
	{
		if (level == 0)
			throw ta;
		else
		{
			L(LOG_WARNING, "Recive resolv TryAgain on %s in %u level\n", domain.c_str(), level);
			return;
		}
	}
	catch (Resolv::Error &er)
	{
		if (level == 0)
			throw er;
		else
		{
			L(LOG_WARNING, "Recive resolv Error on %s in %u level\n", domain.c_str(), level);
			return;
		}
	}

	if (txtReq.Txt().Length() > 0)
	{
		//L(LOG_WARNING, "[%s] txt=%s\n", domain.c_str(), txtReq.Txt().Data());

		char *pstr = txtReq.Txt();
		char *pstart = pstr;
		while (*pstr)
		{
			if (*pstr == ':' || *pstr == '=' || !*pstr)
			{
				if (*pstart == '+' || *pstart == '?')
					pstart++;

				int len = pstr - pstart;

				pstr++;
				char *startData = pstr;
				while (*pstr && *pstr != ' ' && *pstr != '~')
					pstr++;
				char last = *pstr;
				*pstr = 0;

				int dataLen = pstr - startData;
				if (dataLen > 0)
				{
					if (len == 1 && tolower((unsigned char)*pstart) == 'a')
					{
						int slash = 0;
						for (; slash < dataLen; slash++)
							if (startData[slash] == '/')
								break;

						Word32 binMask = SINGL_HOST_MASK; 
						Byte maskIndex =  MAX_NET_MASK;
						int maskLen = dataLen - slash - 1;
						if (maskLen > 0)
						{
							String mask(startData + slash + 1, maskLen);
							maskIndex = atoi(mask.c_str());
							if (maskIndex <= MAX_NET_MASK)
								binMask = NetMask[maskIndex];
							else
								binMask = NetMask[MAX_NET_MASK];
						}


						String checkHost(startData, slash);
						struct addrinfo hints, *res, *res0 = NULL;
						int error;

						memset(&hints, 0, sizeof(hints));
						hints.ai_family = AF_INET;
						hints.ai_socktype = SOCK_STREAM;
						error = getaddrinfo(checkHost.c_str(), NULL, &hints, &res0);
						if (!error) 
						{
							for (res = res0; res; res = res->ai_next)
							{
								if (res->ai_addr)
								{
									sockaddr_in *addr = (sockaddr_in*) res->ai_addr;
									Word32 ip = ntohl(addr->sin_addr.s_addr);
									if (binMask != SINGL_HOST_MASK)
										ip = ip & binMask;
									_nets.push_back(Ip4(ip, binMask));
									//L(LOG_WARNING, "AAdd ip: %s/%u\n", SocketClient::GetIPStr(ip), maskIndex);
								}
							}
						}
						if (res0)
							freeaddrinfo(res0);
					} else if (len == LEN_INCLUDE_STR && !strncasecmp(pstart, INCLUDE_STR, LEN_INCLUDE_STR))
					{
						std::string incDomain(startData, dataLen);
						Word32 ip = SocketClient::IpToLong(incDomain.c_str());
						if ((int)ip != -1)
						{
							_nets.push_back(Ip4(ip, SINGL_HOST_MASK));
						}
						else
							includeDomain.push_back(incDomain);
					}
					else if (len == LEN_IP4_STR && !strncasecmp(pstart, IP4_STR, LEN_IP4_STR))
					{
						char *endNet = strchr(startData, '/');
						if (endNet)
						{
							int netLen = endNet - startData;
							if (netLen > 0)
							{
								String net(startData, netLen);
								int maskLen = dataLen - netLen - 1;
								if (maskLen > 0)
								{
									String mask(endNet + 1, maskLen);
									//L(LOG_WARNING, "Add net: %s/%s\n", net.c_str(), mask.c_str());
									Word32 binMask = 0;
									if (strchr(mask.c_str(), '.')) // ip mask
										binMask = SocketClient::IpToLong(mask.c_str());
									else
									{
										Byte maskIndex = atoi(mask.c_str());
										if (maskIndex <= MAX_NET_MASK)
											binMask = NetMask[maskIndex];
										else
											binMask = NetMask[MAX_NET_MASK];
									}
									_nets.push_back(Ip4(SocketClient::IpToLong(net.c_str()), binMask));

								}
							}
						}
						else
						{
							String ip(startData, dataLen);
							_nets.push_back(Ip4(SocketClient::IpToLong(ip.c_str()), SINGL_HOST_MASK));
							//L(LOG_WARNING, "Add ip: %s\n", ip.c_str());
						}
					}
					else if (len == LEN_REDIRECT_STR && !strncasecmp(pstart, REDIRECT_STR, LEN_REDIRECT_STR))
					{
						includeDomain.push_back(std::string(startData, dataLen));
					}
				}
				*pstr = last;
			}
			if (*pstr == ' ')
			{
				pstart = pstr + 1;
				pstr++;
			}
			else if (*pstr)
				pstr++;
		}
	}
	level++;
	if (level < MAX_REQ_LEVEL)
	{
		for (TStringList::iterator s = includeDomain.begin(); s != includeDomain.end(); ++s)
			_AddNets(*s, rv, txtReq, st, level);
	}
}

bool Spf::CheckIn(const Word32 ip)
{
	for (TIp4List::iterator n = _nets.begin();  n != _nets.end(); ++n)
	{
		if ((ip & n->mask) == n->net)
			return true;
	}
	return false;
}
