/*
 * Check decoding of utimensat syscall.
 *
 * Copyright (c) 2015-2017 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include "scno.h"

#if defined __NR_utimensat && defined UTIME_NOW && defined UTIME_OMIT

# if SIZEOF_KERNEL_LONG_T == 4
#  define big_tv_sec "-559038737"
#  define huge_tv_sec "-559038737"
# else
#  define big_tv_sec "3735928559"
#  define huge_tv_sec "-3819351491602432273"
# endif

# if XLAT_RAW
#  define str_at_fdcwd			"-100"
#  define str_at_symlink_nofollow	"0x100"
#  define str_at_removedir		"0x200"
#  define str_flags1			"0x600"
#  define str_flags2			"0xffffffff"
#  define str_utime_now_omit \
	"[{tv_sec=" big_tv_sec ", tv_nsec=1073741823}, " \
	"{tv_sec=" huge_tv_sec ", tv_nsec=1073741822}]"
# elif XLAT_VERBOSE
#  define str_at_fdcwd			"-100 /* AT_FDCWD */"
#  define str_at_symlink_nofollow	"0x100 /* AT_SYMLINK_NOFOLLOW */"
#  define str_at_removedir		"0x200 /* AT_REMOVEDIR */"
#  define str_flags1 \
	"0x600 /* AT_REMOVEDIR|AT_SYMLINK_FOLLOW */"
#  define str_flags2 \
	"0xffffffff /* AT_SYMLINK_NOFOLLOW|AT_REMOVEDIR|AT_SYMLINK_FOLLOW" \
	"|AT_NO_AUTOMOUNT|AT_EMPTY_PATH|AT_RECURSIVE|0xffff60ff */"
#  define str_utime_now_omit \
	"[{tv_sec=" big_tv_sec ", tv_nsec=1073741823} /* UTIME_NOW */, " \
	"{tv_sec=" huge_tv_sec ", tv_nsec=1073741822} /* UTIME_OMIT */]"
# else
#  define str_at_fdcwd			"AT_FDCWD"
#  define str_at_symlink_nofollow	"AT_SYMLINK_NOFOLLOW"
#  define str_at_removedir		"AT_REMOVEDIR"
#  define str_flags1			"AT_REMOVEDIR|AT_SYMLINK_FOLLOW"
#  define str_flags2 \
	"AT_SYMLINK_NOFOLLOW|AT_REMOVEDIR|AT_SYMLINK_FOLLOW" \
	"|AT_NO_AUTOMOUNT|AT_EMPTY_PATH|AT_RECURSIVE|0xffff60ff"
#  define str_utime_now_omit		"[UTIME_NOW, UTIME_OMIT]"
# endif

static void
print_ts(const struct timespec *ts)
{
	printf("{tv_sec=%lld, tv_nsec=%llu}", (long long) ts->tv_sec,
		zero_extend_signed_to_ull(ts->tv_nsec));
	print_time_t_nsec(ts->tv_sec,
			  zero_extend_signed_to_ull(ts->tv_nsec), 1);
}

static const char *errstr;

static long
k_utimensat(const kernel_ulong_t dirfd,
	    const kernel_ulong_t pathname,
	    const kernel_ulong_t times,
	    const kernel_ulong_t flags)
{
	long rc = syscall(__NR_utimensat, dirfd, pathname, times, flags);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	static const kernel_ulong_t bogus_fd =
		(kernel_ulong_t) 0xbadc0deddeadbeef;
	static const kernel_ulong_t kfdcwd =
		(kernel_ulong_t) 0xdefaced00000000 | -100U;
	static const char proto_fname[] = "utimensat\nfilename";
	static const char qname[] = "\"utimensat\\nfilename\"";

	char *const fname = tail_memdup(proto_fname, sizeof(proto_fname));
	const kernel_ulong_t kfname = (uintptr_t) fname;
	struct timespec *const ts = tail_alloc(sizeof(*ts) * 2);

	(void) close(0);

	/* dirfd */
	k_utimensat(0, kfname, 0, 0);
	printf("utimensat(0, %s, NULL, 0) = %s\n", qname, errstr);

	k_utimensat(bogus_fd, kfname, 0, 0);
	printf("utimensat(%d, %s, NULL, 0) = %s\n",
	       (int) bogus_fd, qname, errstr);

	k_utimensat(-100U, kfname, 0, 0);
	printf("utimensat(" str_at_fdcwd ", %s, NULL, 0) = %s\n", qname, errstr);

	k_utimensat(kfdcwd, kfname, 0, 0);
	printf("utimensat(" str_at_fdcwd ", %s, NULL, 0) = %s\n", qname, errstr);

	/* pathname */
	k_utimensat(kfdcwd, 0, 0, 0);
	printf("utimensat(" str_at_fdcwd ", NULL, NULL, 0) = %s\n", errstr);

	k_utimensat(kfdcwd, kfname + sizeof(proto_fname) - 1, 0, 0);
	printf("utimensat(" str_at_fdcwd ", \"\", NULL, 0) = %s\n", errstr);

	fname[sizeof(proto_fname) - 1] = '+';
	k_utimensat(kfdcwd, kfname, 0, 0);
	fname[sizeof(proto_fname) - 1] = '\0';
	printf("utimensat(" str_at_fdcwd ", %p, NULL, 0) = %s\n", fname, errstr);

	if (F8ILL_KULONG_SUPPORTED) {
		k_utimensat(kfdcwd, f8ill_ptr_to_kulong(fname), 0, 0);
		printf("utimensat(" str_at_fdcwd ", %#jx, NULL, 0) = %s\n",
		       (uintmax_t) f8ill_ptr_to_kulong(fname), errstr);
	}

	/* times */
	k_utimensat(kfdcwd, kfname, (uintptr_t) (ts + 1), 0);
	printf("utimensat(" str_at_fdcwd ", %s, %p, 0) = %s\n",
	       qname, ts + 1, errstr);

	k_utimensat(kfdcwd, kfname, (uintptr_t) (ts + 2), 0);
	printf("utimensat(" str_at_fdcwd ", %s, %p, 0)"
	       " = %s\n", qname, ts + 2, errstr);

	ts[0].tv_sec = 1492358706;
	ts[0].tv_nsec = 123456789;
	ts[1].tv_sec = 1492357068;
	ts[1].tv_nsec = 234567890;

	k_utimensat(kfdcwd, kfname, (uintptr_t) ts, 0x100);
	printf("utimensat(" str_at_fdcwd ", %s, [", qname);
	print_ts(&ts[0]);
	printf(", ");
	print_ts(&ts[1]);
	printf("], " str_at_symlink_nofollow ") = %s\n", errstr);

	ts[0].tv_sec = -1;
	ts[0].tv_nsec = 2000000000;
	ts[1].tv_sec = (time_t) -0x100000001LL;
	ts[1].tv_nsec = 2345678900U;

	k_utimensat(kfdcwd, kfname, (uintptr_t) ts, 0x100);
	printf("utimensat(" str_at_fdcwd ", %s, [", qname);
	print_ts(&ts[0]);
	printf(", ");
	print_ts(&ts[1]);
	printf("], " str_at_symlink_nofollow ") = %s\n", errstr);

	ts[0].tv_sec = 0;
	ts[0].tv_nsec = 0;
	ts[1].tv_sec = (time_t) 0xcafef00ddeadbeefLL;
	ts[1].tv_nsec = 0;

	k_utimensat(kfdcwd, kfname, (uintptr_t) ts, 0x100);
	printf("utimensat(" str_at_fdcwd ", %s, [", qname);
	print_ts(&ts[0]);
	printf(", ");
	print_ts(&ts[1]);
	printf("], " str_at_symlink_nofollow ") = %s\n", errstr);

	ts[0].tv_sec = 0xdeadbeefU;
	ts[0].tv_nsec = 0xfacefeedU;
	ts[1].tv_sec = (time_t) 0xcafef00ddeadbeefLL;
	ts[1].tv_nsec = (long) 0xbadc0dedfacefeedLL;

	k_utimensat(kfdcwd, kfname, (uintptr_t) ts, 0x100);
	printf("utimensat(" str_at_fdcwd ", %s, [", qname);
	print_ts(&ts[0]);
	printf(", ");
	print_ts(&ts[1]);
	printf("], " str_at_symlink_nofollow ") = %s\n", errstr);

	ts[0].tv_nsec = UTIME_NOW;
	ts[1].tv_nsec = UTIME_OMIT;
	k_utimensat(kfdcwd, kfname, (uintptr_t) ts, 0x100);
	printf("utimensat(" str_at_fdcwd ", %s, " str_utime_now_omit
	       ", " str_at_symlink_nofollow ") = %s\n", qname, errstr);

	if (F8ILL_KULONG_SUPPORTED) {
		k_utimensat(kfdcwd, kfname, f8ill_ptr_to_kulong(ts), 0);
		printf("utimensat(" str_at_fdcwd ", %s, %#jx, 0) = %s\n",
		       qname, (uintmax_t) f8ill_ptr_to_kulong(ts), errstr);
	}

	/* flags */
	k_utimensat(kfdcwd, kfname, (uintptr_t) ts,
		    (kernel_ulong_t) 0xdefaced00000200);
	printf("utimensat(" str_at_fdcwd ", %s, " str_utime_now_omit
	       ", " str_at_removedir ") = %s\n",
	       qname, errstr);

	k_utimensat(kfdcwd, kfname, (uintptr_t) ts,
		    (kernel_ulong_t) 0xdefaced00000600);
	printf("utimensat(" str_at_fdcwd ", %s, " str_utime_now_omit
	       ", " str_flags1 ") = %s\n",
	       qname, errstr);

	k_utimensat(kfdcwd, kfname, (uintptr_t) ts, (kernel_ulong_t) -1ULL);
	printf("utimensat(" str_at_fdcwd ", %s, " str_utime_now_omit
	       ", " str_flags2 ") = %s\n",
	       qname, errstr);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_utimensat && UTIME_NOW && UTIME_OMIT")

#endif
