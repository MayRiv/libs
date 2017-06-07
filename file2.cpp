#include "file2.hpp"

lb::File2::~File2()
{
	if (_fd)
		close(_fd );
}
//------------------------------------------------------------------------------
int lb::File2::Open (const char *pathname, int flags, mode_t mode)
{
	Close();

	if (!mode)	_fd = open( pathname, flags);
	else		_fd = open( pathname, flags, mode);

	return _fd;
}
