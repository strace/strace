#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_getuid32

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	printf("getuid32() = %ld\n", syscall(__NR_getuid32));
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_getuid32")

#endif
