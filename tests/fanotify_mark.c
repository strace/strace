#include "tests.h"

#if defined HAVE_SYS_FANOTIFY_H && defined HAVE_FANOTIFY_MARK

# include <stdio.h>
# include <sys/fanotify.h>

int
main(void)
{
	int rc = fanotify_mark(-1, FAN_MARK_ADD, FAN_MODIFY | FAN_ONDIR,
			       -100, ".");
	printf("fanotify_mark(-1, FAN_MARK_ADD, FAN_MODIFY|FAN_ONDIR"
	       ", AT_FDCWD, \".\") = %d %s (%m)\n", rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_SYS_FANOTIFY_H && HAVE_FANOTIFY_MARK")

#endif
