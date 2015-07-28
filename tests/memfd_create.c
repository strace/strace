#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

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

int
main(void)
{
	return 77;
}

#endif
