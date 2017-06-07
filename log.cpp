#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "log.hpp"

using namespace lb;

Log *Log::_defaultLog = new Log(LOG_INFO + 1);

Log::Log(const char *fileName, const int level)
{
	_fd = fopen(fileName, "a+");
  if (!_fd)
	{
		printf("Cannot open log file: %s\n", fileName);
    throw LogError();
	}
  _curLevel = level;
}

Log::~Log()
{
	if (_fd)
		fclose(_fd);
}
const char *ErrorTypeTable[] =
{
	NULL, // UNKNOWN_TYPE
	"[C]", // LOG_CRITICAL
	"[E]", // LOG_ERROR
	"[W]", // LOG_WARNING
	"   ", // LOG_INFO
};

void Log::Write(const ELogTypes type, const char *fmt, va_list args, va_list argsForScreen)
{
  if (type <= _curLevel)
  {
    time_t curTime = time(NULL);
    struct tm *ct = localtime(&curTime);
		if (_fd)
		{
			fprintf(_fd, "%02i.%02i %02i:%02i:%02i %s ", ct->tm_mday, ct->tm_mon+1, ct->tm_hour, ct->tm_min, ct->tm_sec, ErrorTypeTable[type]);
			vfprintf(_fd, fmt, args);
			fflush(_fd);
		}

#ifndef QUIET_MODE
		printf("%02i.%02i %02i:%02i:%02i %s ", ct->tm_mday, ct->tm_mon+1, ct->tm_hour, ct->tm_min, ct->tm_sec, ErrorTypeTable[type]);
	  vprintf(fmt, argsForScreen);
#endif
  	
  }
}

