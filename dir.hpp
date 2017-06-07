#pragma once

#include <dirent.h>
#include <sys/types.h>

namespace lb
{
	class Dir
	{
	public:
		class CannotOpen {};
		Dir(const char *dirName) throw (CannotOpen);
		~Dir();
		const char *Name() const;
		int Next();
		int IsDir()
		{
			if (entry.d_type == DT_DIR)
				return 1;
			else
				return 0;
		}
		unsigned int Inode()
		{
			return entry.d_fileno;
		}
		bool IsSpecName();
		void Reset();
		int IsNFSDir(const char *path, class RString &buf); // is dir if NFS used
		static int RRmDir(const char *path);
		static int RMkDir(const char *path, int mode);
	protected:
		unsigned int fd;
		DIR *dir;
		struct dirent entry;
	};
};

