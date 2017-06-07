#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <ctype.h>
#include "time.hpp"
#include "tools.hpp"

using namespace lb;

void Time::SetLocalTime()
{
	localtime_r(&unixTime, &local);
	ToTStamp(&local, tDay, tStamp);
};

int Time::Days(Word32 tDay)
{
	struct tm td;
	//struct tm td;
	memset(&td, 0, sizeof(tm));
	td.tm_year = (int)tDay/10000 + 100;
	td.tm_mon = (int)(tDay/100)%100 - 1;
	td.tm_mday = (int)tDay%100;
	td.tm_hour = 0;
	td.tm_min  = 0;
	td.tm_sec  = 0;
	td.tm_isdst = -1;
	Word32 st = mktime(&td);
	return (int)(((unixTime + 120)- st)/(DAY_TIME_SEC)); // unixTime + 120 min for dismis error with 0:0
};

struct strlong {
    const char* s;
    long l;
    };


static void
pound_case( char* str )
    {
    for ( ; *str != '\0'; ++str )
	{
	if ( isupper( (int) *str ) )
	    *str = tolower( (int) *str );
	}
    }

static int strlong_compare(const void* v1, const void* v2)
{
	return strcmp( ((struct strlong*) v1)->s, ((struct strlong*) v2)->s );
}


static int
strlong_search( char* str, struct strlong* tab, int n, long* lP )
    {
    int i, h, l, r;

    l = 0;
    h = n - 1;
    for (;;)
	{
	i = ( h + l ) / 2;
	r = strcmp( str, tab[i].s );
	if ( r < 0 )
	    h = i - 1;
	else if ( r > 0 )
	    l = i + 1;
	else
	    {
	    *lP = tab[i].l;
	    return 1;
	    }
	if ( h < l )
	    return 0;
	}
    }
namespace lb
{
const AssocPair wdayTab[] =
{
	{ 0, "sun",  }, { 0, "sunday"},
	{ 1, "mon",  }, { 1, "monday"},
	{ 2, "tue",  }, { 2, "tuesday"},
	{ 3, "wed",  }, { 3, "wednesday"},
	{ 4, "thu",  }, { 4, "thursday"},
	{ 5, "fri",  }, { 5, "friday"},
	{ 6, "sat",  }, { 6, "saturday"},
	{ 0, NULL},
};
const AssocPair monTab[] = {
	{ 0, "jan"}, { 0, "january"},
	{ 1, "feb"}, { 1, "february"},
	{ 2, "mar"}, { 2, "march"},
	{ 3, "apr"}, { 3, "april"},
	{ 4, "may"},
	{ 5, "jun"}, { 5, "june"},
	{ 6, "jul"}, { 6, "july"},
	{ 7, "aug"}, { 7, "august"},
	{ 8, "sep"}, { 8, "september"},
	{ 9, "oct"}, { 9, "october"},
	{ 10, "nov"}, { 10, "november"},
	{ 11, "dec"}, { 11, "december"},
	{ 0, NULL},
};
}

static int
scan_wday( char* str_wday, long* tm_wdayP )
{
    static struct strlong wday_tab[] = {
	{ "sun", 0 }, { "sunday", 0 },
	{ "mon", 1 }, { "monday", 1 },
	{ "tue", 2 }, { "tuesday", 2 },
	{ "wed", 3 }, { "wednesday", 3 },
	{ "thu", 4 }, { "thursday", 4 },
	{ "fri", 5 }, { "friday", 5 },
	{ "sat", 6 }, { "saturday", 6 },
	};
    static int sorted = 0;

  if ( ! sorted )
	{
		(void) qsort(
	    wday_tab, sizeof(wday_tab)/sizeof(struct strlong),
	    sizeof(struct strlong), strlong_compare );
			sorted = 1;
	}
    pound_case( str_wday );
    return strlong_search(
		str_wday, wday_tab, sizeof(wday_tab)/sizeof(struct strlong), tm_wdayP );
}

static int scan_mon( char* str_mon, long* tm_monP )
{
  static struct strlong mon_tab[] = {
	{ "jan", 0 }, { "january", 0 },
	{ "feb", 1 }, { "february", 1 },
	{ "mar", 2 }, { "march", 2 },
	{ "apr", 3 }, { "april", 3 },
	{ "may", 4 },
	{ "jun", 5 }, { "june", 5 },
	{ "jul", 6 }, { "july", 6 },
	{ "aug", 7 }, { "august", 7 },
	{ "sep", 8 }, { "september", 8 },
	{ "oct", 9 }, { "october", 9 },
	{ "nov", 10 }, { "november", 10 },
	{ "dec", 11 }, { "december", 11 },
	};
	
  static int sorted = 0;
  if ( ! sorted )
	{
	(void) qsort(
	    mon_tab, sizeof(mon_tab)/sizeof(struct strlong),
	    sizeof(struct strlong), strlong_compare );
	sorted = 1;
	}
    pound_case( str_mon );
    return strlong_search(
	str_mon, mon_tab, sizeof(mon_tab)/sizeof(struct strlong), tm_monP );
}
    
time_t Time::HTTPStr2Unix(const char *strTime, const int len)
{
	//Mon, 13 Sep 2010 19:55:07 GMT
	struct tm tm;
	memset( (char*) &tm, 0, sizeof(struct tm));

	char *cp = (char*)strTime;
  for (; *cp == ' ' || *cp == '\t'; ++cp )
		continue;
	if (!*cp)
		return 0;
	cp = strchr(cp, ',');
	if (!cp)
		return 0;
	cp++;
	char *endP = NULL;
	tm.tm_mday = strtoul(cp, &endP, 10);
	if (!endP)
		return 0;
	cp = endP + 1;
	endP = strchr(cp, ' ');
	if (!endP)
		return 0;
	switch (tolower(*cp))
	{
	case 'j':
		if (tolower(*(cp + 1)) == 'a')
			tm.tm_mon = 0;
		else if (tolower(*(cp + 2)) == 'n')
			tm.tm_mon = 5;
		else if (tolower(*(cp + 2)) == 'l')
			tm.tm_mon = 6;
	break;
	case 'f':
		tm.tm_mon = 1;
	break;
	case 'm':
		if (tolower(*(cp + 2)) == 'r')
			tm.tm_mon = 2;
		else if (tolower(*(cp + 2)) == 'y')
			tm.tm_mon = 4;
	break;
	case 'a':
		if (tolower(*(cp + 1)) == 'p')
			tm.tm_mon = 3;
		else if (tolower(*(cp + 1)) == 'u')
			tm.tm_mon = 7;
	break;
	case 's':
		tm.tm_mon = 8;
	break;
	case 'o':
		tm.tm_mon = 9;
	break;
	case 'n':
		tm.tm_mon = 10;
	break;
	case 'd':
		tm.tm_mon = 11;
	break;
	default:
		return 0;
	};

	cp = endP + 1;
	endP = NULL;
	tm.tm_year = strtoul(cp, &endP, 10) - 1900;
	if (!endP)
		return 0;
	cp = endP + 1;
	endP = NULL;
	tm.tm_hour = strtoul(cp, &endP, 10);
	if (!endP)
		return 0;
	cp = endP + 1;
	endP = NULL;
	tm.tm_min = strtoul(cp, &endP, 10);
	if (!endP)
		return 0;
	cp = endP + 1;
	endP = NULL;
	tm.tm_sec = strtoul(cp, &endP, 10);
	if (!endP)
		return 0;
	cp = endP + 1;

	tm.tm_isdst = 1; // ukraine use day ligth saving
#ifndef __CYGWIN__
	tm.tm_zone = cp;
#else
#warning "Error: HAVE_TM_ZONE not supported"
#endif

	return timegm(&tm);
}

time_t Time::Str2Unix(const char *str)
{
	struct tm tm;
  char* cp;
  char str_mon[500], str_wday[500];
  int tm_sec, tm_min, tm_hour, tm_mday, tm_year;
  long tm_mon, tm_wday;
  time_t t;

    /* Initialize. */
  (void) memset( (char*) &tm, 0, sizeof(struct tm) );

  /* Skip initial whitespace ourselves - sscanf is clumsy at this. */
  for ( cp = (char*)str; *cp == ' ' || *cp == '\t'; ++cp )
		continue;

  /* And do the sscanfs.  WARNING: you can add more formats here,
  ** but be careful!  You can easily screw up the parsing of existing
   ** formats when you add new ones.  The order is important.
   */

    /* DD-mth-YY HH:MM:SS GMT */
	if ( sscanf( cp, "%d-%400[a-zA-Z]-%d %d:%d:%d GMT",
		&tm_mday, str_mon, &tm_year, &tm_hour, &tm_min,
		&tm_sec ) == 6 &&
	    scan_mon( str_mon, &tm_mon ) )
	{
	tm.tm_mday = tm_mday;
	tm.tm_mon = tm_mon;
	tm.tm_year = tm_year;
	tm.tm_hour = tm_hour;
	tm.tm_min = tm_min;
	tm.tm_sec = tm_sec;
	}

    /* DD mth YY HH:MM:SS GMT */
    else if ( sscanf( cp, "%d %400[a-zA-Z] %d %d:%d:%d GMT",
		&tm_mday, str_mon, &tm_year, &tm_hour, &tm_min,
		&tm_sec) == 6 &&
	    scan_mon( str_mon, &tm_mon ) )
	{
	tm.tm_mday = tm_mday;
	tm.tm_mon = tm_mon;
	tm.tm_year = tm_year;
	tm.tm_hour = tm_hour;
	tm.tm_min = tm_min;
	tm.tm_sec = tm_sec;
	}

    /* HH:MM:SS GMT DD-mth-YY */
    else if ( sscanf( cp, "%d:%d:%d GMT %d-%400[a-zA-Z]-%d",
		&tm_hour, &tm_min, &tm_sec, &tm_mday, str_mon,
		&tm_year ) == 6 &&
	    scan_mon( str_mon, &tm_mon ) )
	{
	tm.tm_hour = tm_hour;
	tm.tm_min = tm_min;
	tm.tm_sec = tm_sec;
	tm.tm_mday = tm_mday;
	tm.tm_mon = tm_mon;
	tm.tm_year = tm_year;
	}

    /* HH:MM:SS GMT DD mth YY */
    else if ( sscanf( cp, "%d:%d:%d GMT %d %400[a-zA-Z] %d",
		&tm_hour, &tm_min, &tm_sec, &tm_mday, str_mon,
		&tm_year ) == 6 &&
	    scan_mon( str_mon, &tm_mon ) )
	{
	tm.tm_hour = tm_hour;
	tm.tm_min = tm_min;
	tm.tm_sec = tm_sec;
	tm.tm_mday = tm_mday;
	tm.tm_mon = tm_mon;
	tm.tm_year = tm_year;
	}

    /* wdy, DD-mth-YY HH:MM:SS GMT */
    else if ( sscanf( cp, "%400[a-zA-Z], %d-%400[a-zA-Z]-%d %d:%d:%d GMT",
		str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min,
		&tm_sec ) == 7 &&
	    scan_wday( str_wday, &tm_wday ) &&
	    scan_mon( str_mon, &tm_mon ) )
	{
	tm.tm_wday = tm_wday;
	tm.tm_mday = tm_mday;
	tm.tm_mon = tm_mon;
	tm.tm_year = tm_year;
	tm.tm_hour = tm_hour;
	tm.tm_min = tm_min;
	tm.tm_sec = tm_sec;
	}

    /* wdy, DD mth YY HH:MM:SS GMT */
    else if ( sscanf( cp, "%400[a-zA-Z], %d %400[a-zA-Z] %d %d:%d:%d GMT",
		str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min,
		&tm_sec ) == 7 &&
	    scan_wday( str_wday, &tm_wday ) &&
	    scan_mon( str_mon, &tm_mon ) )
	{
	tm.tm_wday = tm_wday;
	tm.tm_mday = tm_mday;
	tm.tm_mon = tm_mon;
	tm.tm_year = tm_year;
	tm.tm_hour = tm_hour;
	tm.tm_min = tm_min;
	tm.tm_sec = tm_sec;
	}

    /* wdy mth DD HH:MM:SS GMT YY */
    else if ( sscanf( cp, "%400[a-zA-Z] %400[a-zA-Z] %d %d:%d:%d GMT %d",
		str_wday, str_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec,
		&tm_year ) == 7 &&
	    scan_wday( str_wday, &tm_wday ) &&
	    scan_mon( str_mon, &tm_mon ) )
	{
	tm.tm_wday = tm_wday;
	tm.tm_mon = tm_mon;
	tm.tm_mday = tm_mday;
	tm.tm_hour = tm_hour;
	tm.tm_min = tm_min;
	tm.tm_sec = tm_sec;
	tm.tm_year = tm_year;
	}
		/* wdy mth DD HH:MM:SS YYYY */
	else if ( sscanf( cp, "%400[a-zA-Z] %400[a-zA-Z] %d %d:%d:%d %d",
		str_wday, str_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec,
		&tm_year ) == 7 &&
	    scan_wday( str_wday, &tm_wday ) &&
	    scan_mon( str_mon, &tm_mon ) )
	{
		tm.tm_wday = tm_wday;
		tm.tm_mon = tm_mon;
		tm.tm_mday = tm_mday;
		tm.tm_hour = tm_hour;
		tm.tm_min = tm_min;
		tm.tm_sec = tm_sec;
		tm.tm_year = tm_year;
		tm.tm_isdst = 1;
	}
  else
		return (time_t) -1;

    if ( tm.tm_year > 1900 )
	tm.tm_year -= 1900;
    else if ( tm.tm_year < 70 )
	tm.tm_year += 100;

    t = mktime( &tm );

    return t;
}

Time::Time(const char *strTStamp)
{
	struct tm td;
	int tm_sec = 0, tm_min = 0, tm_hour = 0, tm_mon = 0, tm_mday = 0, tm_year = 0;


	
	if (strlen(strTStamp) == 14) // Ymdhms
		sscanf( strTStamp, "%04d%02d%02d%02d%02d%02d", &tm_year, &tm_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec); 
	else
	{
		sscanf( strTStamp, "%02d%02d%02d%02d%02d%02d", &tm_year, &tm_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec); 
		tm_year += 2000;
	}
	memset(&td, 0, sizeof(td));

	td.tm_year = tm_year;
  if ( td.tm_year > 1900 )
		td.tm_year -= 1900;
  else if ( td.tm_year < 70 )
		td.tm_year += 100;

	td.tm_mon = tm_mon - 1;
	td.tm_mday = tm_mday;
	td.tm_hour = tm_hour;
	td.tm_min  = tm_min;
	td.tm_sec  = tm_sec;
	td.tm_isdst = -1;
  unixTime = mktime( &td );
	SetLocalTime();
}

void Time::GenDayPeriod(Word32 tDay, time_t &fromTime, time_t &toTime, Word32 &curYear, Time &curTime, bool yesterday)
{
	if (tDay < 20000000)
		tDay += 20000000;
	tm lastDayTm;
	memcpy(&lastDayTm, &curTime.Local(), sizeof(tm));
	lastDayTm.tm_year = (int)tDay/10000 + 100 - 2000;
	lastDayTm.tm_mon = (int)(tDay/100)%100 - 1;
	lastDayTm.tm_mday = (int)tDay%100 - (yesterday ? 1 : 0);
	lastDayTm.tm_hour = 0;
	lastDayTm.tm_min  = 0;
	lastDayTm.tm_sec  = 0;
	fromTime = mktime(&lastDayTm);

	memcpy(&lastDayTm, &curTime.Local(), sizeof(tm));
	lastDayTm.tm_year = (int)tDay/10000 + 100 - 2000;
	lastDayTm.tm_mon = (int)(tDay/100)%100 - 1;
	lastDayTm.tm_mday = (int)tDay%100 - (yesterday ? 1 : 0);

	lastDayTm.tm_hour = 23;
	lastDayTm.tm_min = 59;
	lastDayTm.tm_sec = 59;
	toTime = mktime(&lastDayTm);

		/* check for summer -> winter time change */
	Time tFromTime(fromTime);
	if (tFromTime.Hour() == 1)
	{
		fromTime -= 3600;
		toTime -= 3600;
	}
	/* check for winter -> summer time change */
	Time tToTime(toTime);
	if (tToTime.Hour() == 22)
	{
		toTime += 3600;
		fromTime += 3600;
	}
	curYear = lastDayTm.tm_year;
}


void MicroTimer::Start()
{
	gettimeofday(&stv, NULL);
}

Word32 MicroTimer::End()
{
	gettimeofday(&etv, NULL);
	return Sub();
}

Word32 MicroTimer::Sub()
{
	return (etv.tv_sec-stv.tv_sec) * 1000000 + (etv.tv_usec-stv.tv_usec);
}



