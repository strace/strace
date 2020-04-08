/*
 * Check decoding of openat2 syscall.
 *
 * Copyright (c) 2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_openat2

# include <errno.h>
# include <stdint.h>
# include <inttypes.h>
# include <stdio.h>
# include <string.h>
# include <unistd.h>
# include <asm/fcntl.h>

# ifdef HAVE_LINUX_OPENAT2
#  include <linux/openat2.h>
# endif

# ifndef HAVE_LINUX_OPENAT2
struct open_how {
	uint64_t flags;
	uint64_t mode;
	uint64_t resolve;
};
# endif

# ifndef AT_FDCWD
#  define AT_FDCWD -100
# endif
# ifndef RESOLVE_NO_XDEV
#  define RESOLVE_NO_XDEV 0x01
# endif
# ifndef RESOLVE_IN_ROOT
#  define RESOLVE_IN_ROOT 0x10
# endif

# ifndef O_TMPFILE
#  if defined __hppa__
#   define __O_TMPFILE     040000000
#  elif defined __alpha__
#   define __O_TMPFILE     0100000000
#  elif defined __sparc__
#   define __O_TMPFILE     0200000000
#  else
#   define __O_TMPFILE     020000000
#  endif
# endif

# ifndef VERBOSE
#  define VERBOSE 0
# endif
# ifndef FD0_PATH
#  define FD0_PATH ""
# endif

static const char sample[] = "openat2.sample";

int
main(void)
{
	long rc;
	const char *rcstr;
	struct open_how *how = tail_alloc(sizeof(*how));
	struct open_how *how_big = tail_alloc(sizeof(*how_big) + 8);

	rc = syscall(__NR_openat2, 0, NULL, NULL,
		     (kernel_ulong_t) 0xdeadc0debadc0dedULL);
	printf("openat2(0" FD0_PATH ", NULL, NULL, %llu) = %s\n",
	       (unsigned long long) (kernel_ulong_t) 0xdeadc0debadc0dedULL,
	       sprintrc(rc));

	rc = syscall(__NR_openat2, -100, "", how + 1, sizeof(*how));
	printf("openat2(%s, \"\", %p, %zu) = %s\n",
	       XLAT_KNOWN(-100, "AT_FDCWD"), how + 1, sizeof(*how),
	       sprintrc(rc));

	rc = syscall(__NR_openat2, -1, sample, how, 11);
	printf("openat2(-1, \"%s\", %p, 11) = %s\n", sample, how, sprintrc(rc));

	static struct strval64 flags[] = {
		{ ARG_STR(O_RDONLY|O_EXCL) },
		{ ARG_STR(O_WRONLY|O_CREAT) },
		{ ARG_STR(O_RDWR|O_LARGEFILE) },
# ifdef O_TMPFILE
		{ ARG_STR(O_ACCMODE|O_TMPFILE) },
# else
		{ ARG_STR(O_ACCMODE|__O_TMPFILE) },
# endif
		{ ARG_ULL_STR(O_RDONLY|0xdeadface80000000) },
	};
	static uint64_t modes[] = { 0, 0777, 0xbadc0dedfacebeefULL };
	static struct strval64 resolve[] = {
		{ 0, NULL },
		{ ARG_STR(RESOLVE_NO_XDEV) },
		{ ARG_ULL_STR(RESOLVE_NO_XDEV|RESOLVE_IN_ROOT|0xfeedfacedcaffee0) },
		{ 0xdec0dedbeefface0, NULL },
	};
	const size_t iters = MAX(MAX(ARRAY_SIZE(flags), ARRAY_SIZE(modes)),
				 ARRAY_SIZE(resolve));


	for (size_t i = 0; i < iters * 4; i++) {
		how->flags = flags[i % ARRAY_SIZE(flags)].val;
		how->mode = modes[i % ARRAY_SIZE(modes)];
		how->resolve = resolve[i % ARRAY_SIZE(resolve)].val;

		fill_memory(how_big + 1, 8);
		memcpy(how_big, how, sizeof(*how));

		for (size_t j = 0; j < 4; j++) {
			rc = syscall(__NR_openat2, -1, sample,
				     j > 1 ? how_big : how,
				     j ? sizeof(*how) + 8 : sizeof(*how));
			rcstr = sprintrc(rc);
			printf("openat2(-1, \"%s\", {flags=%s",
			       sample,
			       sprintxlat(flags[i % ARRAY_SIZE(flags)].str,
					  flags[i % ARRAY_SIZE(flags)].val,
					  NULL));

			if (how->mode || (i % ARRAY_SIZE(flags) == 1)
				      || (i % ARRAY_SIZE(flags) == 3)) {
				printf(", mode=%#03" PRIo64,
				       modes[i % ARRAY_SIZE(modes)]);
			}

			printf(", resolve=%s",
			       sprintxlat(resolve[i % ARRAY_SIZE(resolve)].str,
					  resolve[i % ARRAY_SIZE(resolve)].val,
					  resolve[i % ARRAY_SIZE(resolve)].val
						? "RESOLVE_???" : NULL));
			if (j == 1)
				printf(", ???");
			if (j == 2) {
				printf(", /* bytes %zu..%zu */ \"\\x80\\x81"
				       "\\x82\\x83\\x84\\x85\\x86\\x87\"",
				       sizeof(*how), sizeof(*how) + 7);
			}
			printf("}, %zu) = %s\n",
			       j ? sizeof(*how) + 8 : sizeof(*how), rcstr);

			if (j == 2)
				memset(how_big + 1, 0, 8);
		}
	}

	how->flags = O_RDONLY | O_NOCTTY;
	how->mode = 0;
	how->resolve = 0;
	rc = syscall(__NR_openat2, -100, "/dev/full", how, sizeof(*how));
	printf("openat2(%s, \"/dev/full\", {flags=%s, resolve=0}, %zu)"
	       " = %s%s\n",
	       XLAT_KNOWN(-100, "AT_FDCWD"), XLAT_STR(O_RDONLY|O_NOCTTY),
	       sizeof(*how), sprintrc(rc), rc >= 0 ? FD0_PATH : "");

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_openat2");

#endif
