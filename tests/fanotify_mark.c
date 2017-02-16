/*
 * Check decoding of fanotify_mark syscall.
 *
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "tests.h"

#include <asm/unistd.h>

#if defined HAVE_SYS_FANOTIFY_H && defined HAVE_FANOTIFY_MARK && \
	defined __NR_fanotify_mark

# include <limits.h>
# include <stdio.h>
# include <unistd.h>
# include <sys/fanotify.h>

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

struct strval64 {
	uint64_t val;
	const char *str;
};

#define STR16 "0123456789abcdef"
#define STR64 STR16 STR16 STR16 STR16

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
		{ (kernel_ulong_t) 0xdec0deddefaced00ULL,
			"0xefaced00 /* FAN_MARK_??? */" },
		{ (kernel_ulong_t) 0xda7a105700000040ULL,
			"FAN_MARK_IGNORED_SURV_MODIFY" },
		{ (kernel_ulong_t) 0xbadc0deddeadfeedULL,
			"FAN_MARK_ADD|FAN_MARK_DONT_FOLLOW|FAN_MARK_ONLYDIR|"
			"FAN_MARK_IGNORED_MASK|FAN_MARK_IGNORED_SURV_MODIFY|"
			"FAN_MARK_FLUSH|0xdeadfe00" },
	};
	static const struct strval64 masks[] = {
		{ ARG_ULL_STR(0) },
		{ 0xdeadfeedfacebeefULL,
			"FAN_ACCESS|FAN_MODIFY|FAN_CLOSE_WRITE|FAN_OPEN|"
			"FAN_ACCESS_PERM|FAN_ONDIR|FAN_EVENT_ON_CHILD|"
			"0xdeadfeedb2ccbec4" },
		{ ARG_ULL_STR(0xffffffffb7fcbfc4) " /* FAN_??? */" },
	};
	static const struct strval dirfds[] = {
		{ (kernel_ulong_t) 0xfacefeed00000001ULL, "1" },
		{ (kernel_ulong_t) 0xdec0ded0ffffffffULL, "FAN_NOFD" },
		{ (kernel_ulong_t) 0xbadfacedffffff9cULL, "AT_FDCWD" },
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
	printf("fanotify_mark(-1, FAN_MARK_ADD, FAN_MODIFY|FAN_ONDIR"
	       ", AT_FDCWD, \".\") = %s\n", sprintrc(rc));

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
