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
	int rc = syscall(__NR_memfd_create, text, 0xf);

	printf("memfd_create(\"%s\", %s) = %s\n",
	       text, "MFD_CLOEXEC|MFD_ALLOW_SEALING|MFD_HUGETLB|0x8",
	       sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_memfd_create")

#endif
