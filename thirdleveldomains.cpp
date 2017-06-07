#include <stdio.h>
#include "string.hpp"

using namespace lb;

const String TRUST_THIRD_LEVEL_DOMAINS[] = {
"in.ua",
"biz.ua",
"co.ua",
"com.ua",
"edu.ua",
"gov.ua",
"net.ua",
"org.ua",
"cherkassy.ua",
"chernigov.ua",
"chernovtsy.ua",
"ck.ua",
"cn.ua",
"crimea.ua",
"cv.ua",
"dn.ua",
"dnepropetrovsk.ua",
"donetsk.ua",
"dp.ua",
"if.ua",
"ivano-frankivsk.ua",
"kh.ua",
"kharkov.ua",
"kherson.ua",
"kiev.ua",
"kirovograd.ua",
"km.ua",
"kr.ua",
"ks.ua",
"lg.ua",
"lugansk.ua",
"lutsk.ua",
"lviv.ua",
"mk.ua",
"nikolaev.ua",
"od.ua",
"odessa.ua",
"pl.ua",
"poltava.ua",
"rovno.ua",
"rv.ua",
"sebastopol.ua",
"sumy.ua",
"te.ua",
"ternopil.ua",
"vinnica.ua",
"vn.ua",
"zaporizhzhe.ua",
"zp.ua",
"uz.ua",
"uzhgorod.ua",
"zhitomir.ua",
"zt.ua",
""
};

const int MAX_ACCEPT_LEVEL = 2;

int CheckDomainLevel(const String &domain, String &parentDomain)
{
	char *pdomain = domain.c_str();
//	if (domain.PartOf("www.", 4))
//		pdomain += 4;
		
	char *pd = pdomain;
	int level = 0;
	while (*pd)
	{
		if (*pd == '.')
			level++;
		pd++;
	}
		
	if (level < MAX_ACCEPT_LEVEL) // 2ld domain
		return 1;
	
	// else 3ld domain check in list
	int i = 0;
	while (TRUST_THIRD_LEVEL_DOMAINS[i])
	{
		int domlen = domain.GetLength();
		int trustLen = TRUST_THIRD_LEVEL_DOMAINS[i].GetLength(); 
		if (domlen >= trustLen)
			if (TRUST_THIRD_LEVEL_DOMAINS[i].PartOf(domain.c_str()+(domlen - trustLen), trustLen))
			{
				if (level == MAX_ACCEPT_LEVEL)
					return 1;
				level--;
				break;
			}
		i++;
	}
	pd = pdomain;
	while (*pd)
	{
  	if (*pd == '.')
  	{
  		level--;
  		pd++;
  		if (level < MAX_ACCEPT_LEVEL)
  			break;
  	}
  	pd++;
	}
	parentDomain.Set(pd);
	return 0;
}
