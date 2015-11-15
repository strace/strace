#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

int
main(void)
{
#if defined __NR_userfaultfd && defined O_CLOEXEC
	if (syscall(__NR_userfaultfd, 1 | O_NONBLOCK | O_CLOEXEC) != -1)
		return 77;
	printf("userfaultfd(O_NONBLOCK|O_CLOEXEC|0x1) = -1 %s\n",
	       errno == ENOSYS ?
			"ENOSYS (Function not implemented)" :
			"EINVAL (Invalid argument)");
	puts("+++ exited with 0 +++");
	return 0;
#else
        return 77;
#endif
}
