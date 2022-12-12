/*
 * Check decoding of ICMP_FILTER.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2022 The strace developers.
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
	printf("getsockopt(-1, SOL_RAW, ICMP_FILTER, NULL, NULL) = %s\n",
	       sprintrc(-1));

	setsockopt(-1, SOL_RAW, ICMP_FILTER, NULL, 0);
	printf("setsockopt(-1, SOL_RAW, ICMP_FILTER, NULL, 0) = %s\n",
	       sprintrc(-1));

	TAIL_ALLOC_OBJECT_CONST_PTR(socklen_t, plen);
	void *const efault = plen + 1;
	TAIL_ALLOC_OBJECT_CONST_PTR(struct icmp_filter, f);

	getsockopt(-1, SOL_RAW, ICMP_FILTER, f, plen);
	printf("getsockopt(-1, SOL_RAW, ICMP_FILTER, %p, [%d]) = %s\n",
	       f, *plen, sprintrc(-1));

	setsockopt(-1, SOL_RAW, ICMP_FILTER, efault, sizeof(*f));
	printf("setsockopt(-1, SOL_RAW, ICMP_FILTER, %p, %u) = %s\n",
	       efault, (unsigned) sizeof(*f), sprintrc(-1));

	setsockopt(-1, SOL_RAW, ICMP_FILTER, f, -2);
	printf("setsockopt(-1, SOL_RAW, ICMP_FILTER, %p, -2) = %s\n",
	       f, sprintrc(-1));

	setsockopt(-1, SOL_RAW, ICMP_FILTER, f, sizeof(*f));
	printf("setsockopt(-1, SOL_RAW, ICMP_FILTER, ~[], %u) = %s\n",
	       (unsigned) sizeof(*f), sprintrc(-1));

	f->data = 0;

	setsockopt(-1, SOL_RAW, ICMP_FILTER, f, sizeof(*f));
	printf("setsockopt(-1, SOL_RAW, ICMP_FILTER, [], %u) = %s\n",
	       (unsigned) sizeof(*f), sprintrc(-1));

	f->data = 1<<ICMP_ECHOREPLY |
		  1<<ICMP_DEST_UNREACH |
		  1<<ICMP_SOURCE_QUENCH |
		  1<<ICMP_REDIRECT |
		  1<<ICMP_TIME_EXCEEDED |
		  1<<ICMP_PARAMETERPROB;
	static const char data_str[] =
		"[ICMP_ECHOREPLY"
		" ICMP_DEST_UNREACH"
		" ICMP_SOURCE_QUENCH"
		" ICMP_REDIRECT"
		" ICMP_TIME_EXCEEDED"
		" ICMP_PARAMETERPROB]";

	setsockopt(-1, SOL_RAW, ICMP_FILTER, f, sizeof(*f));
	printf("setsockopt(-1, SOL_RAW, ICMP_FILTER, %s, %u) = %s\n",
	       data_str, (unsigned) sizeof(*f), sprintrc(-1));

	f->data = ~f->data;

	setsockopt(-1, SOL_RAW, ICMP_FILTER, f, sizeof(*f) * 2);
	printf("setsockopt(-1, SOL_RAW, ICMP_FILTER, ~%s, %u) = %s\n",
	       data_str, (unsigned) sizeof(*f) * 2, sprintrc(-1));

	puts("+++ exited with 0 +++");
	return 0;
}
