#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include <dirent.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include "dir.hpp"
#include "string.hpp"

using namespace lb;

Dir::Dir(const char *dirName) throw (CannotOpen)
{
	this->dir = opendir(dirName);
	if (!this->dir)
		throw CannotOpen();
}

Dir::~Dir()
{
	closedir(dir);
}

int Dir::IsNFSDir(const char *path, RString &buf)
{
	if (entry.d_type == DT_DIR)
		return 1;
	else if (!entry.d_type) // maybe NFS
	{
		buf.snprintf("%s/%s", path, entry.d_name);
		struct stat sb;
		if (!lstat(buf.Data(), &sb))
		{
			if ((sb.st_mode & S_IFMT) == S_IFDIR)
				return 1;
			else 
				return 0;
		}
		else
			return -1;
	}
	else
		return 0;
}

int Dir::Next()
{
	struct dirent *result = NULL;
	if (!readdir_r(this->dir, &entry, &result))
	{
		if (result)
			return 1;
		else
			return 0;
	}
	return 0;
}
void Dir::Reset()
{
	rewinddir(dir);
}
const char *Dir::Name() const
{
	return entry.d_name;
}

bool Dir::IsSpecName()
{
	if (entry.d_name[0] == '.')
	{
		if (entry.d_name[1] == 0)
			return true;
		if ((entry.d_name[1] == '.') && (entry.d_name[2] == 0))
			return true;
	}
	return false;
}

int Dir::RRmDir(const char *path)
{
	try
	{
		Dir dir(path);
		RString cpath;
		while (dir.Next())
		{
			cpath.snprintf("%s/%s", path, dir.Name());
			if (dir.IsDir())
			{
				if (dir.Name()[0] == '.' && ((dir.Name()[1] == '.' &&  dir.Name()[2] == 0) || dir.Name()[1] == 0))
					continue;
				if (!RRmDir(cpath))
					return 0;
			}
			else
				if (unlink(cpath))
					return 0;
		}
		if (rmdir(path))
			return 0;
	}
	catch (Dir::CannotOpen &co)
	{
		return 0;
	}
	return 1;
}

int Dir::RMkDir(const char *path, int mode)
{
	char *pstr = (char*)path;
  char *fch;
  while ((fch=strchr(pstr, '/')) != NULL)
  {
		char ch = *fch;
		*fch = 0;
		if (*pstr)
			if (mkdir(path,  mode) && errno != EEXIST)
			{
				*fch = ch;
				return 0;
			}
		*fch = ch;
		pstr = fch+1;
  }
	if (mkdir(path,  mode) && errno != EEXIST)
		return 0;
	return 1;
}
