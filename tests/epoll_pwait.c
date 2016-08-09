#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_epoll_pwait

# include <signal.h>
# include <stdio.h>
# include <sys/epoll.h>
# include <unistd.h>

# include "kernel_types.h"

int
main(void)
{
	sigset_t set[2];
	struct epoll_event *const ev = tail_alloc(sizeof(*ev));

	long rc = syscall(__NR_epoll_pwait, -1, ev, 1, -2,
			  set, (kernel_ulong_t) sizeof(set));
	printf("epoll_pwait(-1, %p, 1, -2, %p, %u) = %ld %s (%m)\n",
	       ev, set, (unsigned) sizeof(set), rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_epoll_pwait")

#endif
