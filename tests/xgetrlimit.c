/*
 * Check decoding of getrlimit/ugetrlimit syscall.
 *
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@altlinux.org>
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

const char *
sprint_rlim(kernel_ulong_t lim)
{
	if (sizeof(lim) == sizeof(uint64_t)) {
		if (lim == (kernel_ulong_t) -1ULL)
			return "RLIM64_INFINITY";
	} else {
		if (lim == (kernel_ulong_t) -1U)
			return "RLIM_INFINITY";
	}

	static char buf[2][sizeof(lim)*3 + sizeof("*1024")];
	static int i;
	i &= 1;
	if (lim > 1024 && lim % 1024 == 0)
		sprintf(buf[i], "%llu*1024", (unsigned long long) lim / 1024);
	else
		sprintf(buf[i], "%llu", (unsigned long long) lim);

	return buf[i++];
}

#ifdef NR_GETRLIMIT

int
main(void)
{
	kernel_ulong_t *const rlimit = tail_alloc(sizeof(*rlimit) * 2);
	const struct xlat *xlat;

	for (xlat = resources; xlat->str; ++xlat) {
		unsigned long res = 0xfacefeed00000000ULL | xlat->val;
		long rc = syscall(NR_GETRLIMIT, res, 0);
		if (rc && ENOSYS == errno)
			perror_msg_and_skip(STR_GETRLIMIT);
		printf("%s(%s, NULL) = %ld %s (%m)\n",
		       STR_GETRLIMIT, xlat->str, rc, errno2name());

		rc = syscall(NR_GETRLIMIT, res, rlimit);
		if (rc)
			printf("%s(%s, NULL) = %ld %s (%m)\n",
			       STR_GETRLIMIT, xlat->str, rc, errno2name());
		else
			printf("%s(%s, {rlim_cur=%s, rlim_max=%s})"
			       " = 0\n", STR_GETRLIMIT, xlat->str,
			       sprint_rlim(rlimit[0]), sprint_rlim(rlimit[1]));
	}

	puts("+++ exited with 0 +++");
	return 0;
}

#endif /* NR_GETRLIMIT */
