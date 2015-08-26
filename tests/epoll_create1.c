#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <fcntl.h>
#include <unistd.h>
#include <sys/syscall.h>

int
main(void)
{
#if defined __NR_epoll_create1 && defined O_CLOEXEC
	(void) close(0);
	if (syscall(__NR_epoll_create1, O_CLOEXEC))
		return 77;
	return syscall(__NR_epoll_create1, O_CLOEXEC | O_NONBLOCK) >= 0;
#else
        return 77;
#endif
}
