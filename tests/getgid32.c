#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_getgid32

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	printf("getgid32() = %ld\n", syscall(__NR_getgid32));
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_getgid32")

#endif
