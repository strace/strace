#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_olduname

# include <stdio.h>
# include <linux/utsname.h>
# include <unistd.h>

int main(int ac, char **av)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(struct old_utsname, uname);
	int rc = syscall(__NR_olduname, uname);
	printf("olduname({sysname=");
	print_quoted_string(uname->sysname);
	printf(", nodename=");
	print_quoted_string(uname->nodename);
#if VERBOSE
	printf(", release=");
	print_quoted_string(uname->release);
	printf(", version=");
	print_quoted_string(uname->version);
	printf(", machine=");
	print_quoted_string(uname->machine);
#else /* !VERBOSE */
	printf(", ...");
#endif /* VERBOSE */
	printf("}) = %d\n", rc);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_olduname")

#endif
