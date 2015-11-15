#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

int
main(void)
{
#ifdef __NR_membarrier
	if (syscall(__NR_membarrier, 3, 255) != -1)
		return 77;
	printf("membarrier(0x3 /* MEMBARRIER_CMD_??? */, 255) = -1 %s\n",
	       errno == ENOSYS ?
			"ENOSYS (Function not implemented)" :
			"EINVAL (Invalid argument)");
	if (errno != ENOSYS) {
		if (syscall(__NR_membarrier, 0, 0) != 1)
			return 1;  /* the test needs to be updated? */
		puts("membarrier(MEMBARRIER_CMD_QUERY, 0)"
		     " = 0x1 (MEMBARRIER_CMD_SHARED)");
	}
	puts("+++ exited with 0 +++");
	return 0;
#else
        return 77;
#endif
}
