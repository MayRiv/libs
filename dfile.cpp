#include <stdlib.h>
#include <string.h>

#include "dfile.hpp"
#include "string.hpp"

using namespace lb;

DFile::~DFile()
{
	Close();
}

bool DFile::createTmpFile(const char* tmpDir)
{
	RString templ;
	templ.snprintf("%s/~%u.XXXXXX", tmpDir, getpid());
	_descr = mkstemp(templ);
	return (_descr > 0) && (unlink(templ) == 0);// file will be removed after close
}

bool DFile::Open(const char *name, const int flags, const mode_t mode)
{
	_descr = open(name, flags, mode);
	if (_descr > 0)
		return true;
	else
		return false;
}

void DFile::Close()
{
	if (_descr)
	{
		close(_descr);
		_descr = 0;
	}
}

ssize_t DFile::Read(void *buf, const size_t size)
{
	return read(_descr, buf, size);
}

ssize_t DFile::Write(const void *buf, const size_t size)
{
	return write(_descr, buf, size);
}

__off64_t DFile::FileSize64()
{
	__off64_t curPos = lseek64 (_descr,  0, SEEK_CUR); 
	__off64_t size = lseek64(_descr, 0, SEEK_END);
	lseek64(_descr, curPos, SEEK_SET);
	return size;
}

ssize_t DFile::FileSize()
{
	ssize_t curPos = lseek(_descr, 0, SEEK_CUR);
	ssize_t size = lseek(_descr, 0, SEEK_END);
	lseek(_descr, curPos, SEEK_SET);
	return size;
}

__off64_t DFile::LSeek64(__off64_t offset, int whence)
{
	return lseek64(_descr, offset, whence);
}

off_t DFile::LSeek(off_t offset, int whence)
{
	return lseek(_descr, offset, whence);
}

DSeekRead::DSeekRead(DFile *fd, const size_t cacheBlockSize)
	: _fd(fd), _blockSize(cacheBlockSize), _curBlockSeek(0), _loaded(0), _memBlock((char*)malloc(_blockSize+1))
{
}

DSeekRead::DSeekRead(const size_t cacheBlockSize)
	: _fd(NULL), _blockSize(cacheBlockSize), _curBlockSeek(0), _loaded(0), _memBlock((char*)malloc(_blockSize+1))
{
}

void DSeekRead::SetFile(DFile *fd)
{
	_fd = fd;
	_curBlockSeek = 0;
	_loaded = 0;
}

DFile *DSeekRead::Release()
{
	DFile *fd = _fd;
	_fd = NULL;
	return fd;
}

DSeekRead::~DSeekRead()
{
	free(_memBlock);
}

bool DSeekRead::Read(char *buf, const ssize_t size)
{
	if (_curBlockSeek + size > _loaded) // need load more data
	{
		size_t leftLoadSize = size;
		size_t curBufSeek = 0;
		while (leftLoadSize > 0)
		{
			size_t leftInMem = _loaded - _curBlockSeek;
			if (leftInMem > 0)
			{
				if (leftInMem >= leftLoadSize)
				{
					memcpy(buf + curBufSeek, _memBlock + _curBlockSeek, leftLoadSize);
					_curBlockSeek += leftLoadSize;
					return true;
				}
				else
				{
					memcpy(buf + curBufSeek, _memBlock + _curBlockSeek, leftInMem);
					leftLoadSize -= leftInMem;
					curBufSeek += leftInMem;
				}
			}
			_loaded = _fd->Read(_memBlock, _blockSize);
			if (_loaded <= 0)
				return false;
			_curBlockSeek = 0;
		}
		return true;
	}
	else
	{
		memcpy(buf, _memBlock + _curBlockSeek, size);
		_curBlockSeek += size;
		return true;
	}
}

void DSeekRead::Skip(const ssize_t size)
{
	if (_curBlockSeek + size > _loaded)
	{
		int leftInMem = _loaded - _curBlockSeek;
		size_t needSeek = size - leftInMem;
		_fd->LSeek(needSeek, SEEK_CUR);
		_curBlockSeek = 0;
		_loaded = 0;
	}
	else
	{
		_curBlockSeek += size;
	}
}
