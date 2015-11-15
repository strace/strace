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
#ifdef __NR_mlock2
	if (syscall(__NR_mlock2, 0xdeadbeef, 0xdefaced, 0xffff) != -1)
		return 77;
	printf("mlock2(0xdeadbeef, 233811181, MLOCK_ONFAULT|0xfffe) = -1 %s\n",
	       errno == ENOSYS ?
			"ENOSYS (Function not implemented)" :
			"EINVAL (Invalid argument)");
	puts("+++ exited with 0 +++");
	return 0;
#else
        return 77;
#endif
}
