#pragma once

#include <stdarg.h>
#include <stdio.h>

namespace lb
{
	enum ELogTypes
	{
		LOG_CRITICAL  = 1,
		LOG_ERROR,
		LOG_WARNING,
		LOG_INFO
	};

	class Log
	{
	public:
		class LogError {};
		Log(const int level)
			: _fd(NULL), _curLevel(level)
		{
		}
		Log(const char *fileName, const int level);
		~Log();

		void Writef(const ELogTypes type, char *fmt, ...)
		{
			va_list args;
			va_list argsForScreen;
			va_start(args, fmt);
			va_start(argsForScreen, fmt);
			Write(type, fmt, args, argsForScreen);
			va_end(args);
			va_end(argsForScreen);
		}
		void Write(const ELogTypes type, const char *fmt, va_list args, va_list argsForScreen);
		
		static int SetDefaultLog(const char *fileName, const int level)
		{
			Log *newlog = NULL;
			try
			{
				newlog = new Log(fileName, level);
			}
			catch (LogError &le)
			{
				newlog = NULL;
				printf("Cannot set default log file: %s\n", fileName);
			}
			if (!newlog)
				return 0;
			else
			{
				delete _defaultLog;
				_defaultLog = newlog;
				return 1;
			}
		}
		
	friend void L(ELogTypes type, const char *fmt, ...);
	static Log *DefaultLog()
	{
		return _defaultLog;
	}
	private:
		static Log *_defaultLog;
		FILE *_fd;
		int _curLevel;
	};

	inline void L(ELogTypes type, const char *fmt, ...)
	{
	 	va_list args;
		va_list argsForScreen;
		va_start(args, fmt);
		va_start(argsForScreen, fmt);
		Log::_defaultLog->Write(type, fmt, args, argsForScreen);
		va_end(args);
		va_end(argsForScreen);
	}
};

