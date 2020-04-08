/*
 * Check decoding of ICMP_FILTER.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdio.h>
#include <sys/socket.h>
#include <linux/icmp.h>

int
main(void)
{
	getsockopt(-1, SOL_RAW, ICMP_FILTER, 0, 0);
	printf("getsockopt(-1, SOL_RAW, ICMP_FILTER, NULL, NULL) = -1 %s (%m)\n",
	       errno2name());

	setsockopt(-1, SOL_RAW, ICMP_FILTER, NULL, 0);
	printf("setsockopt(-1, SOL_RAW, ICMP_FILTER, NULL, 0) = -1 %s (%m)\n",
	       errno2name());

	TAIL_ALLOC_OBJECT_CONST_PTR(socklen_t, plen);
	void *const efault = plen + 1;
	TAIL_ALLOC_OBJECT_CONST_PTR(struct icmp_filter, f);

	getsockopt(-1, SOL_RAW, ICMP_FILTER, f, plen);
	printf("getsockopt(-1, SOL_RAW, ICMP_FILTER, %p, [%d]) = -1 %s (%m)\n",
	       f, *plen, errno2name());

	setsockopt(-1, SOL_RAW, ICMP_FILTER, efault, sizeof(*f));
	printf("setsockopt(-1, SOL_RAW, ICMP_FILTER, %p, %u) = -1 %s (%m)\n",
	       efault, (unsigned) sizeof(*f), errno2name());

	f->data = ~(
		1<<ICMP_ECHOREPLY |
		1<<ICMP_DEST_UNREACH |
		1<<ICMP_SOURCE_QUENCH |
		1<<ICMP_REDIRECT |
		1<<ICMP_TIME_EXCEEDED |
		1<<ICMP_PARAMETERPROB);

	setsockopt(-1, SOL_RAW, ICMP_FILTER, f, -2);
	printf("setsockopt(-1, SOL_RAW, ICMP_FILTER, %p, -2) = -1 %s (%m)\n",
	       f, errno2name());

	setsockopt(-1, SOL_RAW, ICMP_FILTER, f, sizeof(*f));
	printf("setsockopt(-1, SOL_RAW, ICMP_FILTER, %s, %u) = -1 %s (%m)\n",
	       "~(1<<ICMP_ECHOREPLY|1<<ICMP_DEST_UNREACH|1<<ICMP_SOURCE_QUENCH"
	       "|1<<ICMP_REDIRECT|1<<ICMP_TIME_EXCEEDED|1<<ICMP_PARAMETERPROB)",
	       (unsigned) sizeof(*f), errno2name());

	setsockopt(-1, SOL_RAW, ICMP_FILTER, f, sizeof(*f) * 2);
	printf("setsockopt(-1, SOL_RAW, ICMP_FILTER, %s, %u) = -1 %s (%m)\n",
	       "~(1<<ICMP_ECHOREPLY|1<<ICMP_DEST_UNREACH|1<<ICMP_SOURCE_QUENCH"
	       "|1<<ICMP_REDIRECT|1<<ICMP_TIME_EXCEEDED|1<<ICMP_PARAMETERPROB)",
	       (unsigned) sizeof(*f) * 2, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}
