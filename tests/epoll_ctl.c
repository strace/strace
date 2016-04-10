#include "tests.h"
#include <sys/syscall.h>

#if defined __NR_epoll_ctl && defined HAVE_SYS_EPOLL_H

# include <errno.h>
# include <inttypes.h>
# include <stdio.h>
# include <sys/epoll.h>
# include <unistd.h>

int
main(void)
{
	struct epoll_event *const ev = tail_alloc(sizeof(*ev));
	ev->events = EPOLLIN;

	int rc = syscall(__NR_epoll_ctl, -1, EPOLL_CTL_ADD, -2, ev);
	printf("epoll_ctl(-1, EPOLL_CTL_ADD, -2, {EPOLLIN,"
	       " {u32=%u, u64=%" PRIu64 "}}) = %d %s (%m)\n",
	       ev->data.u32, ev->data.u64, rc,
	       errno == ENOSYS ? "ENOSYS" : "EBADF");

	rc = syscall(__NR_epoll_ctl, -3, EPOLL_CTL_DEL, -4, ev);
	printf("epoll_ctl(-3, EPOLL_CTL_DEL, -4, %p) = %d %s (%m)\n",
	       ev, rc, errno == ENOSYS ? "ENOSYS" : "EBADF");

	rc = syscall(__NR_epoll_ctl, -1L, EPOLL_CTL_MOD, -16L, 0);
	printf("epoll_ctl(-1, EPOLL_CTL_MOD, -16, NULL) = %d %s (%m)\n",
	       rc, errno == ENOSYS ? "ENOSYS" : "EFAULT");

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_epoll_ctl && HAVE_SYS_EPOLL_H")

#endif
