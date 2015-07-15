#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdint.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/time.h>

#if defined HAVE_UTIMENSAT \
 && defined AT_FDCWD && defined AT_SYMLINK_NOFOLLOW \
 && defined UTIME_NOW && defined UTIME_OMIT

static void
print_ts(const struct timespec *ts)
{
	printf("{%ju, %ju}", (uintmax_t) ts->tv_sec, (uintmax_t) ts->tv_nsec);
}

int
main(void)
{
	struct timeval tv;
	struct timespec ts[2];

	if (gettimeofday(&tv, NULL))
		return 77;

	ts[0].tv_sec = tv.tv_sec;
	ts[0].tv_nsec = tv.tv_usec;
	ts[1].tv_sec = tv.tv_sec - 1;
	ts[1].tv_nsec = tv.tv_usec + 1;
	if (!utimensat(AT_FDCWD, "utimensat\nfilename", ts,
	     AT_SYMLINK_NOFOLLOW))
		return 77;

	#define PREFIX "utimensat(AT_FDCWD, \"utimensat\\nfilename\", ["

	printf(PREFIX);
	print_ts(&ts[0]);
	printf(", ");
	print_ts(&ts[1]);
	puts("], AT_SYMLINK_NOFOLLOW) = -1 ENOENT (No such file or directory)");

	ts[0].tv_nsec = UTIME_NOW;
	ts[1].tv_nsec = UTIME_OMIT;
	if (!utimensat(AT_FDCWD, "utimensat\nfilename", ts,
	     AT_SYMLINK_NOFOLLOW))
		return 77;

	printf(PREFIX);
	puts("UTIME_NOW, UTIME_OMIT], AT_SYMLINK_NOFOLLOW) = -1 ENOENT (No such file or directory)");
	puts("+++ exited with 0 +++");

	return 0;
}

#else

int
main(void)
{
	return 77;
}

#endif
