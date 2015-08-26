#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <fcntl.h>
#include <unistd.h>
#include <sys/syscall.h>

int
main(void)
{
#if defined __NR_eventfd2 && defined O_CLOEXEC
	(void) close(0);
	return syscall(__NR_eventfd2, -1L, 1 | O_CLOEXEC | O_NONBLOCK) == 0 ?
		0 : 77;
#else
        return 77;
#endif
}
