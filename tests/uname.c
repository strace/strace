#include "tests.h"
#include <sys/syscall.h>

#ifdef __NR_uname

# include <stdio.h>
# include <sys/utsname.h>
# include <unistd.h>

int main()
{
	struct utsname *const uname = tail_alloc(sizeof(struct utsname));
	int rc = syscall(__NR_uname, uname);
	printf("uname({sysname=\"%s\", nodename=\"%s\", release=\"%s\""
	       ", version=\"%s\", machine=\"%s\""
# ifdef HAVE_STRUCT_UTSNAME_DOMAINNAME
	       ", domainname=\"%s\""
# endif
	       "}) = %d\n",
	       uname->sysname,
	       uname->nodename,
	       uname->release,
	       uname->version,
	       uname->machine,
# ifdef HAVE_STRUCT_UTSNAME_DOMAINNAME
	       uname->domainname,
# endif
	       rc);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_uname")

#endif
