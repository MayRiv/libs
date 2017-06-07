#pragma once

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace lb
{

class File2
{
public:
	File2 ();
	virtual ~File2();

	int Open (const char *pathname, int flags, mode_t mode = 0);
	int Close();

	int Fcntl (int cmd);
	int Fcntl (int cmd, long arg);
	int Fcntl (int cmd, long arg, struct flock *lock);

	off_t Lseek (off_t offset, int whence);
	int Fstat (struct stat *buf);

	ssize_t Read (void *buf, size_t count);
	ssize_t Write(const void *buf, size_t count);
	ssize_t FileSize()
	{
		off_t curPos = lseek(_fd, 0, SEEK_CUR);
		off_t endPos = lseek(_fd, 0, SEEK_END);
		lseek(_fd, curPos, SEEK_SET);
		return endPos;
	}
	int FD()
	{
		return _fd;
	}

protected:
	int	_fd;
};
//==============================================================================
inline File2::File2() : _fd (-1)
{
}
//------------------------------------------------------------------------------
inline int File2::Close()
{
	if (_fd != -1)
		return close(_fd );

	return 0;
}
//------------------------------------------------------------------------------
inline int File2::Fcntl (int cmd)
{
	if (_fd == -1)
		return -1;

	return fcntl(_fd, cmd);
}
//------------------------------------------------------------------------------
inline int File2::Fcntl (int cmd, long arg)
{
	if (_fd == -1)
		return -1;

	return fcntl(_fd, cmd, arg);
}
//------------------------------------------------------------------------------
inline int File2::Fcntl (int cmd, long arg, struct flock *lock)
{
	if (_fd == -1)
		return -1;

	return fcntl(_fd, cmd, arg, lock);
}
//------------------------------------------------------------------------------
inline off_t File2::Lseek (off_t offset, int whence)
{
	return lseek(_fd, offset, whence);
}
//------------------------------------------------------------------------------
inline ssize_t File2::Read (void *buf, size_t count)
{
	return read(_fd, buf, count);
}
//------------------------------------------------------------------------------
inline ssize_t File2::Write (const void *buf, size_t count)
{
	return write(_fd, buf, count);
}
//------------------------------------------------------------------------------
inline int File2::Fstat (struct stat *buf)
{
	return fstat(_fd, buf);
}
};


