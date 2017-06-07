#ifdef LINUX
#include <sys/prctl.h>
#endif
#include <sys/resource.h>
#include <unistd.h>

#include "limit.hpp"

using namespace lb;


int Limit::EnableCoreDump()
{
#ifdef LINUX
  prctl(PR_SET_DUMPABLE, 1);
#endif
  struct rlimit limit;
  limit.rlim_cur = limit.rlim_max = RLIM_INFINITY;
	return !setrlimit(RLIMIT_CORE, &limit);
}

int Limit::SetMaxOpenFiles(const int maxOpenFiles)
{
  struct rlimit limit;
  limit.rlim_cur = limit.rlim_max = maxOpenFiles;
	return !setrlimit(RLIMIT_NOFILE, &limit);
}

int Limit::SetMaxProcess(const int maxProcess)
{
	struct rlimit limit;
	limit.rlim_cur = limit.rlim_max = maxProcess;
#ifdef RLIMIT_NPROC
	return !setrlimit(RLIMIT_NPROC, &limit);
#else
#warning "Error: RLIMIT_NPROC not supported"
#endif
	return 1;
}

