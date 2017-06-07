#pragma once


#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef LINUX
typedef off_t __off64_t;
#define lseek64 lseek
#endif

namespace lb
{
	class DFile
	{
	public:
		typedef int TDescriptor;
		static const int DEFAULT_FILE_PERMS = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

		DFile()
			: _descr(0)
		{
		}
		~DFile();
		bool createTmpFile(const char* tmpDir);
		bool Open(const char *name, const int flags, const mode_t mode = DEFAULT_FILE_PERMS);
		void Close();
		ssize_t Read(void *buf, const size_t size);
		ssize_t Write(const void *buf, const size_t size);
		TDescriptor Descr()
		{
			return _descr;
		}
		TDescriptor Release()
		{
			TDescriptor res = _descr;
			_descr = 0;
			return res;
		}
		ssize_t FileSize();
		__off64_t FileSize64();
		off_t LSeek(off_t offset, int whence);
		__off64_t LSeek64(__off64_t offset, int whence);
	private:
		TDescriptor _descr;
	};

	class DSeekRead
	{
	public:
		DSeekRead(DFile *fd, const size_t cacheBlockSize);
		DSeekRead(const size_t cacheBlockSize);
		~DSeekRead();
		bool Read(char *buf, const ssize_t size);
		void Skip(const ssize_t size);
		void SetFile(DFile *fd);
		DFile *Release();
	private:
		DFile *_fd;
		ssize_t _blockSize;
		ssize_t _curBlockSeek;
		ssize_t _loaded;
		char *_memBlock;
	};
};
