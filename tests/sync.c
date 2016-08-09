#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_sync

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	printf("sync() = %ld\n", syscall(__NR_sync));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_sync")

#endif
