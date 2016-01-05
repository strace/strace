#include "tests.h"

#if defined HAVE_SYS_FANOTIFY_H && defined HAVE_FANOTIFY_MARK

# include <sys/fanotify.h>

int
main(void)
{
	fanotify_mark(-1, FAN_MARK_ADD, FAN_MODIFY | FAN_ONDIR, -100, ".");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_SYS_FANOTIFY_H && HAVE_FANOTIFY_MARK")

#endif
