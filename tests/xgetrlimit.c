/*
 * Check decoding of getrlimit/ugetrlimit syscall.
 *
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/resource.h>
#include <unistd.h>

#include "xlat.h"
#include "xlat/resources.h"

static const char *
sprint_rlim(kernel_ulong_t lim)
{
	static char buf[2][ /* space for 2 llu strings */
				2*sizeof(lim)*3 +
			    /* space for XLAT_STYLE_ABBREV decoding */
				sizeof("*1024") + sizeof("RLIM64_INFINITY") +
			    /* space for C style comments */
				6];
	static unsigned int i;

	i &= 1;

#if XLAT_RAW
	sprintf(buf[i], "%llu", (unsigned long long) lim);
	return buf[i++];
#else
	if (sizeof(lim) == sizeof(uint64_t)) {
		if (lim == (kernel_ulong_t) -1ULL) {
# if XLAT_VERBOSE
			sprintf(buf[i], "%llu /* RLIM64_INFINITY */",
				(unsigned long long) lim);
			return buf[i++];
# else /* XLAT_ABBREV */
			return "RLIM64_INFINITY";
# endif
		}
	} else {
		if (lim == (kernel_ulong_t) -1U) {
# if XLAT_VERBOSE
			sprintf(buf[i], "%llu /* RLIM_INFINITY */",
				(unsigned long long) lim);
			return buf[i++];
# else /* XLAT_ABBREV */
			return "RLIM_INFINITY";
# endif
		}
	}

	if (lim > 1024 && lim % 1024 == 0)
# if XLAT_VERBOSE
		sprintf(buf[i], "%llu /* %llu*1024 */",
			(unsigned long long) lim,
			(unsigned long long) lim / 1024);
# else /* XLAT_ABBREV */
		sprintf(buf[i], "%llu*1024", (unsigned long long) lim / 1024);
# endif
	else
		sprintf(buf[i], "%llu", (unsigned long long) lim);

	return buf[i++];
#endif /* !XLAT_RAW */
}

#ifdef NR_GETRLIMIT

int
main(void)
{
	TAIL_ALLOC_OBJECT_CONST_ARR(kernel_ulong_t, rlimit, 2);
	const struct xlat_data *xlat;
	size_t i;
	long rc;

	rc = syscall(NR_GETRLIMIT, 16, 0);
	printf("%s(0x10 /* RLIMIT_??? */, NULL) = %s\n",
	       STR_GETRLIMIT, sprintrc(rc));

	for (xlat = resources->data, i = 0; i < resources->size; ++xlat, ++i) {
		if (!xlat->str)
			continue;

		unsigned long res = 0xfacefeed00000000ULL | xlat->val;
		rc = syscall(NR_GETRLIMIT, res, 0);
		if (rc && ENOSYS == errno)
			perror_msg_and_skip(STR_GETRLIMIT);
		printf("%s(%s, NULL) = %s\n",
		       STR_GETRLIMIT, xlat->str, sprintrc(rc));

		rc = syscall(NR_GETRLIMIT, res, rlimit);
		if (rc)
			printf("%s(%s, %p) = %s\n",
			       STR_GETRLIMIT, xlat->str, rlimit, sprintrc(rc));
		else
			printf("%s(%s, {rlim_cur=%s, rlim_max=%s})"
			       " = 0\n", STR_GETRLIMIT, xlat->str,
			       sprint_rlim(rlimit[0]), sprint_rlim(rlimit[1]));
	}

	puts("+++ exited with 0 +++");
	return 0;
}

#endif /* NR_GETRLIMIT */
