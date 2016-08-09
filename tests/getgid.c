#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_getgid

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	printf("getgid() = %ld\n", syscall(__NR_getgid));
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_getgid")

#endif
