#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_epoll_ctl

# include <inttypes.h>
# include <stdio.h>
# include <sys/epoll.h>
# include <unistd.h>

static long
invoke_syscall(unsigned long epfd, unsigned long op, unsigned long fd, void *ev)
{
	op |= (unsigned long) 0xffffffff00000000ULL;
	return syscall(__NR_epoll_ctl, epfd, op, fd, (unsigned long) ev);
}

int
main(void)
{
	struct epoll_event *const ev = tail_alloc(sizeof(*ev));
	ev->events = EPOLLIN;

	long rc = invoke_syscall(-1U, EPOLL_CTL_ADD, -2U, ev);
	printf("epoll_ctl(-1, EPOLL_CTL_ADD, -2, {EPOLLIN,"
	       " {u32=%u, u64=%" PRIu64 "}}) = %ld %s (%m)\n",
	       ev->data.u32, ev->data.u64, rc, errno2name());

	rc = invoke_syscall(-3U, EPOLL_CTL_DEL, -4U, ev);
	printf("epoll_ctl(-3, EPOLL_CTL_DEL, -4, %p) = %ld %s (%m)\n",
	       ev, rc, errno2name());

	rc = invoke_syscall(-1UL, EPOLL_CTL_MOD, -16UL, 0);
	printf("epoll_ctl(-1, EPOLL_CTL_MOD, -16, NULL) = %ld %s (%m)\n",
	       rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_epoll_ctl")

#endif
