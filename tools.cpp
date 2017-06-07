#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/statvfs.h>
#include <utime.h>

#include "xtypes.hpp"
#include "string.hpp"
#include "tools.hpp"



namespace lb 
{

bool GetLoadAvg(LoadAvgData &avg)
{
	int fd = open("/proc/loadavg", O_RDONLY);
	if (fd > 0)
	{
		const int READ_BUF_LEN = 25;
		char buf[READ_BUF_LEN+1];
		read(fd, buf, READ_BUF_LEN);
		close(fd);

		char *endBuf = NULL;
		avg.avg1 = strtod(buf, &endBuf);
		if (!endBuf)
			return false;
		
		char *startBuf = endBuf;
		avg.avg5 = strtod(startBuf, &endBuf);
		if (!endBuf)
			return false;
		startBuf = endBuf;
		avg.avg15 = strtod(startBuf, &endBuf);
		if (!endBuf)
			return false;
		return true;
	}
	return false;
}

int FileExists(const char *fileName)
{
	struct stat fStat;
	if (lstat(fileName, &fStat))
		return false;
	else
		return true;
}

int Touch(const char *fileName, const int defaultFileMode)
{
	utimbuf tb;
	tb.modtime = time(NULL);
	tb.actime = tb.modtime;
	if (utime(fileName, &tb)) {
		int fd = open(fileName, O_RDONLY | O_CREAT, defaultFileMode);
		if (fd) {
			close(fd);
			return true;
		} else {
			return false;
		}
	}
	return true;
}

char *FindDomain(char *email)
{
	char *p;
	if (((p=strchr(email, '@')) != 0))
		return p + 1;
	return NULL;
}

off_t FileSize(const char *path)
{
	struct stat st;
	if (lstat(path, &st) == -1)
		return 0;
	return st.st_size;
}

Word32 FileSize(FILE *file)
{
	Word32 curPos = ftell(file);
	fseek(file, 0, SEEK_END);
	Word32 size = ftell(file);
	fseek(file, curPos, SEEK_SET);
	return size;
}

Word32 ToTStamp(Word32 &tDay, Word64 &tStamp)
{
	time_t currentTime = time(0);
	struct tm *localTm=localtime(&currentTime);
	tDay = (localTm->tm_year-100)*10000+(localTm->tm_mon+1)*100+localTm->tm_mday;
	tStamp = (Word64)(tDay+20000000)*1000000;
	tStamp += (Word64)localTm->tm_hour*10000 + localTm->tm_min*100 + localTm->tm_sec;
  return currentTime;
}

void ToTStamp(time_t currentTime, Word32 &tDay, Word64 &tStamp){
	struct tm *localTm=localtime(&currentTime);
	tDay = (localTm->tm_year-100)*10000+(localTm->tm_mon+1)*100+localTm->tm_mday;
	tStamp = (Word64)(tDay+20000000)*1000000;
	tStamp += (Word64)localTm->tm_hour*10000 + localTm->tm_min*100 + localTm->tm_sec;
}

void ToTStamp(struct tm *localTm, Word32 &tDay, Word64 &tStamp){
	tDay = (localTm->tm_year-100)*10000+(localTm->tm_mon+1)*100+localTm->tm_mday;
	tStamp = (Word64)(tDay+20000000)*1000000;
	tStamp += (Word64)localTm->tm_hour*10000 + localTm->tm_min*100 + localTm->tm_sec;
}


void ParseEmail(char *email, char *name, char *address)
{
	char *pstr = strchr(email, '<');
	if (!pstr)
	{
		name[0] = 0;
		strcpy(address, email);
		
	}
	else
	{
		memcpy(name, email, pstr-email);
		name[ pstr-email] = 0;
		pstr++;
		char *pend = strchr(pstr, '>');
		if (!pend)
		{
			address[0] = 0;
		}
		else
		{
			memcpy(address, pstr, pend-pstr);
			address[pend-pstr] = 0;
		}
	}
}

void FormExpiresDate(Word32 secs, RString &buf)
{
static const char *day_short_names[]={"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static const char *mon_short_names[]={"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
 	time_t t = time(0)+secs;
  struct tm* tm1 = gmtime(&t);
buf.snprintf("%s, %02d-%s-%04d %02d:%02d:%02d GMT",
day_short_names[tm1->tm_wday],
tm1->tm_mday,
mon_short_names[tm1->tm_mon],
tm1->tm_year+1900,
tm1->tm_hour, tm1->tm_min, tm1->tm_sec);
}

void GetExpiresDate(Word32 secs, char *buffer_str){
static const char *day_short_names[]={"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static const char *mon_short_names[]={"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
 	time_t t = time(0)+secs;
  struct tm* tm1 = gmtime(&t);
sprintf(buffer_str, "%s, %02d-%s-%04d %02d:%02d:%02d GMT",
day_short_names[tm1->tm_wday],
tm1->tm_mday,
mon_short_names[tm1->tm_mon],
tm1->tm_year+1900,
tm1->tm_hour, tm1->tm_min, tm1->tm_sec);
}



void GetHTTPDate(const time_t t, RString &buf)
{
	static const char *day_short_names[]={"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	static const char *mon_short_names[]={"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  struct tm* tm1 = gmtime(&t);
	buf.snaprintf("%s, %02d %s %04d %02d:%02d:%02d GMT",
day_short_names[tm1->tm_wday],
tm1->tm_mday,
mon_short_names[tm1->tm_mon],
tm1->tm_year+1900,
tm1->tm_hour, tm1->tm_min, tm1->tm_sec);
}

#define OS_WIN9X 1
#define OS_WINNT 2
#define OS_WIN3X 3
#define OS_LINUX 4
#define OS_FBSD	 5
#define OS_SUN	 6
#define OS_IRIX	 7
#define OS_OS2	 8
#define OS_MAC	 9
/*

int ParseOs(char * os_str)
{
	register char * t = os_str;
	//printf("\nPARSE_OS %.10s\n", os_str);
	
parse_win:
	if((t = strstr(t,"Win"))!=NULL)
	{
		t += 3;
		//printf("%s\n", t);
		if(memcmp(t,"dows",4)==0)
			t += 4;

		if(*t==';')//for Netscape 6+
		{
			t+=2;
			goto parse_win;
		}

		if(*t==' ')
			t++;
		if(t[0]=='9' && (t[1]=='5'||t[1]=='8'))
		{
			return OS_WIN9X;
		}
		if( (t[0]=='N' && t[1]=='T') || (t[0]=='2' && t[1]=='0') )
		{
			return OS_WINNT;
		}
		if(t[0]=='3')
		{
			if(t[1]=='.')
				return OS_WIN3X;
			else if(t[1]=='2')
				return OS_WIN9X;
		}
	}
	t = os_str;
	if(strstr( t, "Linux")){
		return OS_LINUX;
	}
	if(strstr( t,"FreeBSD")){
		return OS_FBSD;
	}
	if(strstr( t,"Sun")){
		return OS_SUN;
	}
	if(strstr( t,"irix")){
		return OS_SUN;
	}
	if(strstr( t,"OS/2")){
		return OS_OS2;
	}
	if((t = strstr( t,"Mac"))!=NULL){
		t+=3;
		if(*t=='i' || *t=='_')
			return OS_MAC;
	}
	return 0;
}

Word16 GetOsBrws(char *userAgent){
														 //	 0 1	2	 3	4	 5	6	 7	8	 9
	static const int opera[10]=	  {0,0, 0,22,21,20,19,18, 0, 0};
	static const int msie[10]=	  {0,0, 4, 3, 2, 1, 5, 6, 0, 0};
	static const int netscape[10]={0,0,14,13,12, 0,15,16, 0, 0};
	register char *p=userAgent;
  register char *s=0;
	Byte os = 0;
	Byte brws = 0;
	Byte version = 0;
	char ch;
	int len = strlen(userAgent);
	if(len>10)
	{
		if(!memcmp("Mozilla",userAgent, 7))
		{
			ch = userAgent[8];
			if(ch>='0' && ch <='9'){
				version = ch - '0';
				brws = netscape[version];
			}
			p+=9;
			//printf("M %.4s V%c ",p,ch);
			if( (s = strchr(p,'('))!=NULL)
			{//MSIE || Netscape && OS
				if((p = strstr(s,"MSIE"))!=NULL )
				{
//				printf("Msie %.4s V%d ",p,version);
					p+=5;
					os = ParseOs(p+2);
					ch = p[0];
					if(ch>='0' && ch<='9'){
						version = ch - '0';
						brws = msie[version];
					}
					if((p = strstr(s, "Opera"))!=NULL){//search after MSIE string
						ch = p[6];
						if(ch>='0' && ch <='9'){
							version = ch - '0';
							brws = opera[version];
							//printf("\nOpr %.4s V%c ",p,ch);
						}
					}
				}
				else
				{
					if(!os)
					{
//					printf("%s\n",s);
						os = ParseOs(s);
					}
					if( (p = strstr(s,"Netscape"))!=NULL )
					{
						if(p[8]=='6')
							brws = netscape[6];
						else
						{
							ch = p[9];
							if(ch>='0' && ch <='9')
							{
								version = ch - '0';
								brws = netscape[version];
								//printf("ns V %c \n", p, ch);
							}
						}
					}
				}
			}
		}
		else if(!memcmp("Opera/",userAgent, 6))
		{
			ch = userAgent[6];
			if(ch>='0' && ch <='9')
			{
				version = ch - '0';
				brws = opera[version];
			}
			os = ParseOs(userAgent + 8);
			//printf("\nO %.4s V%c ",p,ch);
		}
	}//len smaller than 10
	
	if(!os)
		os = ParseOs(userAgent);
	return (os<<8)|brws;
}
*/
int GetAssocPairKey(const char *value, const int len, const AssocPair *pairs)
{
  int i = 0;
  while (pairs[i].key)
  {
    if (!strncasecmp(value, pairs[i].value, len))
      return pairs[i].key;
    i++;
  }
  return 0;
              
}


int GetAssocPairKey(const char *value, const AssocPair *pairs)
{
  int i = 0;
  while (pairs[i].key)
  {
    if (!strcasecmp(value, pairs[i].value))
      return pairs[i].key;
    i++;
  }
  return 0;
              
}

int GetAssocPairZeroKey(const char *value, const AssocPair *pairs)
{
  int i = 0;
  while (pairs[i].value)
  {
    if (!strcasecmp(value, pairs[i].value))
      return pairs[i].key;
    i++;
  }
  return pairs[i].key;
              
}

Word64 binImplode(char* src, char delim)
{
	Word64 mask = 0ll;
	char* next_char = src;
	int currentInt;

	while('\0' != *next_char)
	{
		currentInt = strtol(next_char, &next_char, 10);
		if(currentInt && currentInt<64)
		{
			register Word64 temp = 1;
			temp <<= (currentInt-1);
			mask |= temp;
			//printf("%02d %016llx \n",currentInt, mask);
		}
		if(*next_char != delim)
			break;
		else
			next_char++;
	}
	return mask;
}

int ParseHeader(const char *header, char *&value, const int len, char del, const AssocPair *pairs)
{
	char *pstr = (char*)memchr(header, del, len);
	if (!pstr)
		return 0;
	int id = GetAssocPairKey(header, pstr - header, pairs);
	value = pstr + 1;
	return id;
}

int CheckHeader(const char *header, const char *reqHeader, char del, char *&value)
{
	char *pstr = (char*)strchr(header, del);
	if (!pstr)
		return 0;
	if (strncasecmp(header, reqHeader, pstr-header))
		return 0;
	value = pstr + 1;
	return 1;
}
/*
//some test function
#include <stdio.h>

char *uas[] = {
"Mozilla/5.0 (Windows; U; Windows NT 5.0; en-US; rv:1.0.1) Gecko/20020823 Netscape/7.0"
,"Mozilla/4.6 [en] (Win95; I)"
,"Mozilla/4.0 (compatible; MSIE 4.01; Windows 98)"
,"Mozilla/4.0 (compatible; MSIE 4.01; Windows 95)"
,"Mozilla/4.0 (compatible; MSIE 4.0; Windows 95)"
,"Mozilla/3.0 (compatible; MSIE 3.0)"
,"Mozilla/2.0 (compatible; MSIE 3.02; Update a; Windows 95)"
,"Mozilla/4.04 [en] (Win95; I)"
,"Mozilla/3.0 (Win95; I)"
,"Mozilla/2.0 (compatible; MSIE 3.01; Windows 95)"
,"Teleport Pro/1.28"
,"Mozilla/2.0 (compatible; MSIE 3.0; Windows 95)"
,"Mozilla/4.05 [en] (Win95; I)"
,"Mozilla/4.0 (compatible; MSIE 4.01; Windows NT)"
,"Mozilla/2.0 (compatible; MSIE 3.02; Windows 95)"
,"Arkanavt/1.02.015 (compatible; Win16; I)"
,"Mozilla/4.03 [en] (Win95; I)"
,"Teleport Pro/1.29"
,"Mozilla/4.01 [en] (Win95; I)"
,"Mozilla/3.01 (Win95; I)"
,"Mozilla/3.01Gold (Win95; I)"
,"Mozilla/2.02 (OS/2; I)"
,"Mozilla/3.01 (WinNT; I)"
,"WebZIP/2.32 (http://www.spidersoft.com)"
,"Mozilla/3.0 (Win95; I; HTTPClient 1.0)"
,"IBM-WebExplorer-DLL/v1.2 "
,"Mozilla/4.0 [en] (Win95; I)"
,"Mozilla/2.0 (compatible; MSIE 3.02; Update a; AK; Windows 95)"
,"Mozilla/2.0 (compatible; MSIE 3.02; AK; Windows 95)"
,"Mozilla/3.0"
,"Mozilla/4.0 (compatible; MSIE 4.0; Windows NT)"
,"Mozilla/4.04 [en] (WinNT; I)"
,"Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.0)"
,"Mozilla/4.0 (compatible; MSIE 5.0; Windows 2000) Opera 5.12  [en]"
,"Mozilla/4.75C-CCK-MCD {C-UDP; EBM-APPLE} (Macintosh; U; PPC)"
,"Mozilla/4.0 (compatible; MSIE 5.0; Mac_PowerPC)"
,"Opera/6.0 (Windows 2000; U)  [en]"
,"Mozilla/4.0 (compatible; MSIE 5.0; Windows NT 5.1) Opera 5.12  [ru]"
,"Mozilla/4.0 (compatible; MSIE 5.0; Windows 98) Opera 6.03  [en]"
,"Opera/6.03 (Windows 2000; U)  [ru]"
,"Mozilla/3.01 (compatible;)"
,"Mozilla/2.02"
,"Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1)"
,"Mozilla/4.0 (compatible; MSIE 6.0; Windows 98; e-Gloryon Browser)"
,"Netscape/4.5 (WindowsNT;8-bit)"
,"Mozilla/5.0 (Windows; U; Win98; en-US; rv:0.9.4) Gecko/20011128 Netscape6/6.2.1"
,"Mozilla/4.0 (compatible; MSIE 5.0; Windows 2000) Opera 6.01  [ru]"
,"Opera/5.12 (Windows 98; U)"
,"Mozilla/4.0 (compatible; MSIE 5.5; Windows 98; Win 9x 4.90)"
	};

int main(){
//	Word32 tDay;
//	Word64 tStamp;
//	ToTStamp(tDay,tStamp);
	//printf("%llu %lu",tStamp, tDay);
	int j,i=0,total = sizeof(uas)/sizeof(uas[0]);
	char* ua;
	
	srand(time(0));
	printf("%d\r\n",total);
	for (i=0;i<total;++i)
	{
		j=i;//rand()%total;
		ua=uas[j];
		printf("0x%x %s\r\n", GetOsBrws(ua), ua);
	}
	return 0;
}
*/

size_t strip_html_tags(char *text, const char *const *allowTags, const char *const *denyContainer)
{
	//puts("-----------------------------");
	//puts(text);

	char *pIn = text, *pOut = text;
	size_t *tagLen = NULL, *contLen = NULL;

	if (allowTags) {
		size_t len = 0;
		while (allowTags[len]) 
			len++;

		tagLen = (size_t *) malloc(sizeof(size_t) * len);
		if (!tagLen)
			return 0;

		for (size_t i = 0; i < len; i++)
			tagLen[i] = strlen(allowTags[i]);
	}
	if (denyContainer) {
		size_t len = 0;
		while (denyContainer[len]) 
			len++;

		contLen = (size_t *) malloc(sizeof(size_t) * len);
		if (!contLen)
			return 0;

		for (size_t i = 0; i < len; i++)
			contLen[i] = strlen(denyContainer[i]);
	}

	const char *const *tag;
	const size_t *len;
	char *tmp;
	while (*pIn) {
		if (*pIn == '<' && pIn[1]) {
			if (!strncmp(pIn + 1, "!--", 3)) {
				pIn = strstr(pIn + 1, "-->");
				if (pIn) {
					pIn += 3;
					continue;
				} else {
					break;
				}
			}
			if (allowTags) {
				tag = allowTags;
				len = tagLen;
				tmp = pIn[1] == '/' ? pIn + 2 : pIn + 1;
				while (*tag) {
					if (!strncasecmp(tmp, *tag, *len) && !isalnum(tmp[*len])) 
						break;
					tag++;
					len++;
				}
				if (*tag) {
					tmp = strstr(tmp + *len, ">");
					if (!tmp) 
						break;

					tmp++;
					memcpy(pOut, pIn, tmp - pIn);
					pOut += tmp - pIn;
					pIn = tmp;
					continue;
				}
			}
			if (denyContainer) {
				tag = denyContainer;
				len = contLen;
				tmp = pIn[1] == '/' ? pIn + 2 : pIn + 1;
				while (*tag) {
					if (!strncasecmp(tmp, *tag, *len) && !isalnum(tmp[*len])) 
						break;
					tag++;
					len++;
				}
				if (*tag) {
					tmp = strstr(tmp + *len, ">");
					if (!tmp) 
						break;

					if (pIn[1] == '/') {
						pIn = tmp + 1;
						continue;
					}

					tmp--;
					while ((tmp = strstr(tmp + 2, "</")) && (strncasecmp(tmp + 2, *tag, *len) || isalnum(tmp[2 + *len])));
					if (!tmp)
						break;

					tmp = strstr(tmp + 2 + *len, ">");
					if (!tmp) 
						break;

					pIn = tmp + 1;
					continue;
				}
			}

			pIn = strstr(pIn + 1, ">");
			if (!pIn)
				break;

			pIn++;
		//} else if (*pIn == '&') {
		//	pIn++;
		//	if (!strncasecmp(pIn, "nbsp;", 5)) {
		//		*pOut++ = ' ';
		//		pIn += 5;
		//	} else if (!strncasecmp(pIn, "amp;", 4)) {
		//		*pOut++ = '&';
		//		pIn += 4;
		//	} else if (!strncasecmp(pIn, "quot;", 5)) {
		//		*pOut++ = '"';
		//		pIn += 5;
		//	} else if (!strncasecmp(pIn, "lt;", 3)) {
		//		*pOut++ = '<';
		//		pIn += 3;
		//	} else if (!strncasecmp(pIn, "gt;", 3)) {
		//		*pOut++ = '>';
		//		pIn += 3;
		//	} else {
		//		*pOut++ = '&';
		//	}
		} else {
			*pOut++ = *pIn++;
		}
	}

	*pOut = '\0';

	if (allowTags)
		free(tagLen);
	if (denyContainer)
		free(contLen);

	//puts("\n-------------------------------------------------------------\n");
	//puts(text);

	return pOut - text;
}

size_t compress_spaces(char *text)
{
	const char *b = text;
	char *dst = text;

	while (*text) {
		*dst++ = *text;

		if (isspace(*text)) {
			do {
				text++;
			} while (isspace(*text));
		} else {
			text++;
		}
	}
	
	*dst = '\0';
	return dst - b;
}

static long gSeed1;
static long gSeed2;
static unsigned long gShift;

void SeedRandom( )
{
	while( !gSeed1 )
	{
		gSeed1 = clock();
		gSeed1 <<= 16;
		gSeed1 ^= clock();
	}
	while( !gSeed2 )
		gSeed2 = time( NULL );
		
	if ( !gShift )
		gShift = (unsigned long)gSeed1 ^ (unsigned long)gSeed2;
}

#define kMultiplierOne	53668
#define kIncrementOne	40014
#define kConstantOne	12211
#define kModulusOne		0x7FFFFFAB

#define kMultiplierTwo	52774
#define kIncrementTwo	40692
#define kConstantTwo	3791
#define kModulusTwo		0x7FFFFF07

#define kZAdder			0x7FFFFFAA

long CombinedLCGRandom()
{
	long vRatio,vRetVal;
	
	vRatio = gSeed1/kMultiplierOne;
	gSeed1 = kIncrementOne * (gSeed1 - kMultiplierOne * vRatio) - kConstantOne * vRatio;
	if ( gSeed1 < 0 )
		gSeed1 += kModulusOne;

	vRatio = gSeed2/kMultiplierTwo;
	gSeed2 = kIncrementTwo * (gSeed2 - kMultiplierTwo * vRatio) - kConstantTwo * vRatio;
	if ( gSeed2 < 0 )
		gSeed2 += kModulusTwo;
	
	vRetVal = gSeed1 - gSeed2;
	if ( vRetVal < 1 )
		vRetVal += kZAdder;
		
	return vRetVal;
}


void dmybetween(const int& d2, const int& m2, const int& y2, const int& d1, const int& m1, const int& y1, int& d, int& m, int& y)
{
    bool yleap;
		int dm[13];
    int cf;

    yleap = y2%4==0&&(y2%100!=0||y2%400==0);
    dm[1] = 31;
    dm[3] = 31;
    dm[4] = 30;
    dm[5] = 31;
    dm[6] = 30;
    dm[7] = 31;
    dm[8] = 31;
    dm[9] = 30;
    dm[10] = 31;
    dm[11] = 30;
    dm[12] = 31;
    if( yleap )
    {
        dm[2] = 29;
    }
    else
    {
        dm[2] = 28;
    }
    cf = 0;
    d = d1-d2;
    if( d<0 )
    {
        d = d+dm[m2];
        cf = 1;
    }
    m = m1-m2-cf;
    cf = 0;
    if( m<0 )
    {
        m = m+12;
        cf = 1;
    }
    y = y1-y2-cf;
}


unsigned char *GetCharTableFromString(const char *str)
{
	int tblLen = 1 << (sizeof(unsigned char)* 8);
	Byte *tbl = new Byte[tblLen + 1];
	memset(tbl, 0, tblLen + 1);
	int strLen = strlen(str);
	for (int i = 0; i < strLen; i++)
		tbl[(unsigned char)str[i]] = 1;
	return tbl;
}

const char * const GetHostName()
{
	static const Word32 MAX_HOST_NAME_LEN = 128;
	static char hostName[MAX_HOST_NAME_LEN] = "\0";
	if (hostName[0] == 0)
	{
		gethostname(hostName, MAX_HOST_NAME_LEN);
	}
	return hostName;
}

bool GetDiskSize(const char *path, Word64 &freeDiskSpace, Word64 &allDiskSpace)
{
	struct statvfs svfs;
	if (statvfs(path, &svfs) == 0)
	{
		freeDiskSpace = (Word64)svfs.f_bavail * svfs.f_frsize;
		allDiskSpace  = (Word64)svfs.f_blocks * svfs.f_frsize;
		//L(LOG_WARNING, "%s Space %llu from %llu (%llu)\n", _path.c_str(), _freeDiskSpace, _allDiskSpace, (_freeDiskSpace * 100)/_allDiskSpace);
		return true;
	}
	else 
		return false;
}

bool GetDomainFromURL(const char *url, String &domain)
{
	static const Word32 HTTP_LENGTH = sizeof("http://") - 1;
	static const Word32 HTTPS_LENGTH = sizeof("https://") - 1;

	const char *pHostStart = url;
	if (!strncasecmp(pHostStart, "http://", HTTP_LENGTH))
		pHostStart += HTTP_LENGTH;
	else if (!strncasecmp(pHostStart, "https://", HTTPS_LENGTH))
		pHostStart += HTTPS_LENGTH;
	const char *pHostEnd = strstr(pHostStart, "/");
	if (!pHostEnd)
		pHostEnd = url + strlen(url);
	int len = pHostEnd -  pHostStart;
	if (len > 0)
	{
		domain.Set(pHostStart, len);
		return true;
	}
	return false;
}
};

