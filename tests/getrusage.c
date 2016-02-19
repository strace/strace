#include "tests.h"
#include <sys/syscall.h>

#ifdef __NR_getrusage

# include <stdio.h>
# include <sys/resource.h>
# include <unistd.h>

int
main(void)
{
	struct rusage usage;
	int rc = syscall(__NR_getrusage, RUSAGE_SELF, &usage);
	printf("getrusage(RUSAGE_SELF, {ru_utime={%lu, %lu}"
	       ", ru_stime={%lu, %lu}, ru_maxrss=%lu, ru_ixrss=%lu"
	       ", ru_idrss=%lu, ru_isrss=%lu, ru_minflt=%lu"
	       ", ru_majflt=%lu, ru_nswap=%lu, ru_inblock=%lu"
	       ", ru_oublock=%lu, ru_msgsnd=%lu, ru_msgrcv=%lu"
	       ", ru_nsignals=%lu, ru_nvcsw=%lu, ru_nivcsw=%lu}) = %d\n",
	       usage.ru_utime.tv_sec, usage.ru_utime.tv_usec,
	       usage.ru_stime.tv_sec, usage.ru_stime.tv_usec,
	       usage.ru_maxrss, usage.ru_ixrss, usage.ru_idrss,
	       usage.ru_isrss, usage.ru_minflt, usage.ru_majflt,
	       usage.ru_nswap, usage.ru_inblock, usage.ru_oublock,
	       usage.ru_msgsnd, usage.ru_msgrcv, usage.ru_nsignals,
	       usage.ru_nvcsw, usage.ru_nivcsw, rc);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_getrusage")

#endif
