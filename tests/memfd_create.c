#include "tests.h"
#include <unistd.h>
#include <sys/syscall.h>

#ifdef __NR_memfd_create

int
main(void)
{
	syscall(__NR_memfd_create, "strace", 7);
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_memfd_create")

#endif
