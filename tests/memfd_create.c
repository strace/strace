#include "tests.h"
#include <asm/unistd.h>
#include "scno.h"

#ifdef __NR_memfd_create

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	static const char text[] = "strace";
	int rc = syscall(__NR_memfd_create, text, 7);

	printf("memfd_create(\"%s\", %s) = %d %s (%m)\n",
	       text, "MFD_CLOEXEC|MFD_ALLOW_SEALING|0x4", rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_memfd_create")

#endif
