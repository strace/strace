/*
 * Check decoding of fanotify_mark syscall.
 *
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2015-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include "scno.h"

#if defined HAVE_SYS_FANOTIFY_H && defined HAVE_FANOTIFY_MARK && \
	defined __NR_fanotify_mark

# include <limits.h>
# include <stdio.h>
# include <unistd.h>
# include <sys/fanotify.h>

# if XLAT_RAW
#  define str_fan_mark_add	"0x1"
#  define str_fan_modify_ondir	"0x40000002"
#  define str_at_fdcwd		"-100"
# elif XLAT_VERBOSE
#  define str_fan_mark_add	"0x1 /* FAN_MARK_ADD */"
#  define str_fan_modify_ondir	"0x40000002 /* FAN_MODIFY|FAN_ONDIR */"
#  define str_at_fdcwd		"-100 /* AT_FDCWD */"
# else
#  define str_fan_mark_add	"FAN_MARK_ADD"
#  define str_fan_modify_ondir	"FAN_MODIFY|FAN_ONDIR"
#  define str_at_fdcwd		"AT_FDCWD"
# endif

/* Performs fanotify_mark call via the syscall interface. */
static void
do_call(kernel_ulong_t fd, kernel_ulong_t flags, const char *flags_str,
	uint64_t mask, const char *mask_str, kernel_ulong_t dirfd,
	const char *dirfd_str, kernel_ulong_t path, const char *path_str)
{
	long rc;

	rc = syscall(__NR_fanotify_mark, fd, flags,
# if (LONG_MAX > INT_MAX) \
  || (defined __x86_64__ && defined __ILP32__) \
  || defined LINUX_MIPSN32
		mask,
# else
/* arch/parisc/kernel/sys_parisc32.c, commit ab8a261b */
#  ifdef HPPA
		LL_VAL_TO_PAIR((mask << 32) | (mask >> 32)),
#  else
		LL_VAL_TO_PAIR(mask),
#  endif
# endif
		dirfd, path);

	printf("fanotify_mark(%d, %s, %s, %s, %s) = %s\n",
	       (int) fd, flags_str, mask_str, dirfd_str, path_str,
	       sprintrc(rc));
}

struct strval {
	kernel_ulong_t val;
	const char *str;
};

# define STR16 "0123456789abcdef"
# define STR64 STR16 STR16 STR16 STR16

int
main(void)
{
	enum {
		PATH1_SIZE = 64,
	};

	static const kernel_ulong_t fds[] = {
		(kernel_ulong_t) 0xdeadfeed12345678ULL,
		F8ILL_KULONG_MASK,
		(kernel_ulong_t) 0xdeb0d1edffffffffULL,
	};
	static const struct strval flags[] = {
		{ F8ILL_KULONG_MASK, "0" },
		{ (kernel_ulong_t) 0xdec0deddefacec00ULL,
			"0xefacec00"
# if !XLAT_RAW
			" /* FAN_MARK_??? */"
# endif
			},
		{ (kernel_ulong_t) 0xda7a105700000040ULL,
# if XLAT_RAW
			"0x40"
# elif XLAT_VERBOSE
			"0x40 /* FAN_MARK_IGNORED_SURV_MODIFY */"
# else
			"FAN_MARK_IGNORED_SURV_MODIFY"
# endif
			},
		{ (kernel_ulong_t) 0xbadc0deddeadffffULL,
# if XLAT_RAW || XLAT_VERBOSE
			"0xdeadffff"
# endif
# if XLAT_VERBOSE
			" /* "
# endif
# if !XLAT_RAW
			"FAN_MARK_ADD|FAN_MARK_REMOVE|FAN_MARK_DONT_FOLLOW|"
			"FAN_MARK_ONLYDIR|FAN_MARK_MOUNT|FAN_MARK_IGNORED_MASK|"
			"FAN_MARK_IGNORED_SURV_MODIFY|FAN_MARK_FLUSH|"
			"FAN_MARK_FILESYSTEM|0xdeadfe00"
# endif
# if XLAT_VERBOSE
			" */"
# endif
			},
	};
	static const struct strval64 masks[] = {
		{ ARG_ULL_STR(0) },
		{ 0xdeadfeedffffffffULL,
# if XLAT_RAW || XLAT_VERBOSE
			"0xdeadfeedffffffff"
# endif
# if XLAT_VERBOSE
			" /* "
# endif
# if !XLAT_RAW
			"FAN_ACCESS|"
			"FAN_MODIFY|"
			"FAN_ATTRIB|"
			"FAN_CLOSE_WRITE|"
			"FAN_CLOSE_NOWRITE|"
			"FAN_OPEN|"
			"FAN_MOVED_FROM|"
			"FAN_MOVED_TO|"
			"FAN_CREATE|"
			"FAN_DELETE|"
			"FAN_DELETE_SELF|"
			"FAN_MOVE_SELF|"
			"FAN_OPEN_EXEC|"
			"FAN_Q_OVERFLOW|"
			"FAN_OPEN_PERM|"
			"FAN_ACCESS_PERM|"
			"FAN_OPEN_EXEC_PERM|"
			"FAN_ONDIR|"
			"FAN_EVENT_ON_CHILD|"
			"0xdeadfeedb7f8a000"
# endif
# if XLAT_VERBOSE
			" */"
# endif
			},
		{ ARG_ULL_STR(0xffffffffb7f8a000)
# if !XLAT_RAW
			" /* FAN_??? */"
# endif
			},
	};
	static const struct strval dirfds[] = {
		{ (kernel_ulong_t) 0xfacefeed00000001ULL, "1" },
		{ (kernel_ulong_t) 0xdec0ded0ffffffffULL,
# if XLAT_RAW
			"-1"
# elif XLAT_VERBOSE
			"-1 /* FAN_NOFD */"
# else
			"FAN_NOFD"
# endif
			},
		{ (kernel_ulong_t) 0xbadfacedffffff9cULL, str_at_fdcwd },
		{ (kernel_ulong_t) 0xdefaced1beeff00dULL, "-1091571699" },
	};
	static const char str64[] = STR64;

	static char bogus_path1_addr[sizeof("0x") + sizeof(void *) * 2];
	static char bogus_path1_after_addr[sizeof("0x") + sizeof(void *) * 2];

	char *bogus_path1 = tail_memdup(str64, PATH1_SIZE);
	char *bogus_path2 = tail_memdup(str64, sizeof(str64));

	struct strval paths[] = {
		{ (kernel_ulong_t) 0, "NULL" },
		{ (kernel_ulong_t) (uintptr_t) (bogus_path1 + PATH1_SIZE),
			bogus_path1_after_addr },
		{ (kernel_ulong_t) (uintptr_t) bogus_path1, bogus_path1_addr },
		{ (kernel_ulong_t) (uintptr_t) bogus_path2, "\"" STR64 "\"" },
	};

	unsigned int i;
	unsigned int j;
	unsigned int k;
	unsigned int l;
	unsigned int m;
	int rc;


	snprintf(bogus_path1_addr, sizeof(bogus_path1_addr), "%p", bogus_path1);
	snprintf(bogus_path1_after_addr, sizeof(bogus_path1_after_addr), "%p",
		bogus_path1 + PATH1_SIZE);

	rc = fanotify_mark(-1, FAN_MARK_ADD, FAN_MODIFY | FAN_ONDIR,
			       -100, ".");
	printf("fanotify_mark(-1, %s, %s, %s, \".\") = %s\n",
	       str_fan_mark_add, str_fan_modify_ondir, str_at_fdcwd,
	       sprintrc(rc));

	for (i = 0; i < ARRAY_SIZE(fds); i++) {
		for (j = 0; j < ARRAY_SIZE(flags); j++) {
			for (k = 0; k < ARRAY_SIZE(masks); k++) {
				for (l = 0; l < ARRAY_SIZE(dirfds); l++) {
					for (m = 0; m < ARRAY_SIZE(paths); m++)
						do_call(fds[i],
							flags[j].val,
							flags[j].str,
							masks[k].val,
							masks[k].str,
							dirfds[l].val,
							dirfds[l].str,
							paths[m].val,
							paths[m].str);
				}
			}
		}
	}

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_SYS_FANOTIFY_H && HAVE_FANOTIFY_MARK && "
		    "__NR_fanotify_mark")

#endif
