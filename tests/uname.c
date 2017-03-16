#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_uname

# include <stdio.h>
# include <sys/utsname.h>
# include <unistd.h>

int main(int ac, char **av)
{
	int abbrev = ac > 1;
	TAIL_ALLOC_OBJECT_CONST_PTR(struct utsname, uname);
	int rc = syscall(__NR_uname, uname);
	printf("uname({sysname=\"");
	print_quoted_string(uname->sysname);
	printf("\", nodename=\"");
	print_quoted_string(uname->nodename);
	if (abbrev) {
		printf("\", ...");
	} else {
		printf("\", release=\"");
		print_quoted_string(uname->release);
		printf("\", version=\"");
		print_quoted_string(uname->version);
		printf("\", machine=\"");
		print_quoted_string(uname->machine);
# ifdef HAVE_STRUCT_UTSNAME_DOMAINNAME
		printf("\", domainname=\"");
		print_quoted_string(uname->domainname);
# endif
		printf("\"");
	}
	printf("}) = %d\n", rc);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_uname")

#endif
