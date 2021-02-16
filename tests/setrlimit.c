/*
 * Check decoding of setrlimit syscall.
 *
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_setrlimit

# include "xgetrlimit.c"

int
main(void)
{
	kernel_ulong_t *const rlimit = tail_alloc(sizeof(*rlimit) * 2);
	const struct xlat_data *xlat;
	size_t i = 0;

	for (xlat = resources->data, i = 0; i < resources->size; ++xlat, ++i) {
		if (!xlat->str)
			continue;

		unsigned long res = 0xfacefeed00000000ULL | xlat->val;
		long rc = syscall(__NR_setrlimit, res, 0);
# if XLAT_RAW
		printf("setrlimit(%#x, NULL) = %s\n",
		       (unsigned int) xlat->val, sprintrc(rc));
# elif XLAT_VERBOSE
		printf("setrlimit(%#x /* %s */, NULL) = %s\n",
		       (unsigned int) xlat->val,
		       xlat->str, sprintrc(rc));
# else
		printf("setrlimit(%s, NULL) = %s\n", xlat->str, sprintrc(rc));
# endif

		struct rlimit libc_rlim = {};
		if (getrlimit((int) res, &libc_rlim))
			continue;
		rlimit[0] = libc_rlim.rlim_cur;
		rlimit[1] = libc_rlim.rlim_max;

		rc = syscall(__NR_setrlimit, res, rlimit);
		const char *errstr = sprintrc(rc);
# if XLAT_RAW
		printf("setrlimit(%#x, {rlim_cur=%s, rlim_max=%s}) = %s\n",
		       (unsigned int) xlat->val,
		       sprint_rlim(rlimit[0]), sprint_rlim(rlimit[1]),
		       errstr);
# elif XLAT_VERBOSE
		printf("setrlimit(%#x /* %s */,"
		       " {rlim_cur=%s, rlim_max=%s}) = %s\n",
		       (unsigned int) xlat->val, xlat->str,
		       sprint_rlim(rlimit[0]), sprint_rlim(rlimit[1]),
		       errstr);
# else
		printf("setrlimit(%s, {rlim_cur=%s, rlim_max=%s}) = %s\n",
		       xlat->str,
		       sprint_rlim(rlimit[0]), sprint_rlim(rlimit[1]),
		       errstr);
# endif
	}

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_setrlimit")

#endif
