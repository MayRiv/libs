#pragma once

#include <time.h>
#include "xtypes.hpp"
#include "tools.hpp"

namespace lb
{
	const int DAY_TIME_SEC = 86400;
	extern const AssocPair wdayTab[];
	extern const AssocPair monTab[];

	class Time
	{
	public:
		Time()
		{
			unixTime = time(NULL);
			SetLocalTime();
		}

		Time(time_t setTime)
		{
			unixTime = setTime;
			SetLocalTime();
		}

		Time(struct tm *localTm)
		{
			unixTime = mktime(localTm);
			SetLocalTime();
		}

		Time(const char *strTStamp);

		void Update()
		{
			unixTime = time(NULL);
			SetLocalTime();
		}

		time_t Unix() const { return unixTime; };
		tm &Local() { return local; };

		int Sec() const { return local.tm_sec; };
		int Min() const { return local.tm_min; };
		int Hour() const { return local.tm_hour; };
		int MDay() const { return local.tm_mday; };
		int Month() const { return local.tm_mon; };
		int Year() const { return local.tm_year; };
		int WDay() const { return local.tm_wday; };
		int FromMondayWDay() const { return ((local.tm_wday  + 6) % 7); };
		int YDay() const { return local.tm_yday; };
		int HourSec() const { return local.tm_min * 60 + local.tm_sec; };
		Word64 TStamp() const { return tStamp; }
		Word32 TDay() const { return tDay;}
		int Days(const Time &startTime) const
		{
			return (int)(( unixTime - startTime.unixTime)/(DAY_TIME_SEC));
		};
		int Days(Word32 tDay);
		
		static time_t Str2Unix(const char *stime); // convert from 
		static time_t HTTPStr2Unix(const char *strTime, const int len);

		static void GenDayPeriod(Word32 tDay, time_t &fromTime, time_t &toTime, Word32 &curYear, Time &curTime, bool yesterday);
	protected:
		void SetLocalTime();
		time_t unixTime;
		Word64 tStamp;
		Word32 tDay;
		struct tm local;
	};

	#include <sys/time.h>
	class MicroTimer
	{
	public:
		MicroTimer()
		{
			Start();
		}
		void Start();
		Word32 End();
		double EndSec()
		{
			return End()/1000000.0;
		}
		Word32 Sub();
	protected:
		struct timeval stv, etv;
	};
};


