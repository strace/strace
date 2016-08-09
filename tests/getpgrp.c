#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_getpgrp

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	printf("getpgrp() = %ld\n", syscall(__NR_getpgrp));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_getpgrp")

#endif
