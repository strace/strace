/*
 * This file is part of sysinfo strace test.
 *
 * Copyright (c) 2015 Elvira Khabirova <lineprinter0@gmail.com>
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdio.h>
#include <sys/sysinfo.h>

int
main(void)
{
	sysinfo(NULL);
	printf("sysinfo(NULL) = -1 EFAULT (%m)\n");

	TAIL_ALLOC_OBJECT_CONST_PTR(struct sysinfo, si);

	if (sysinfo(si))
		perror_msg_and_skip("sysinfo");
	printf("sysinfo({uptime=%llu"
		", loads=[%llu, %llu, %llu]"
		", totalram=%llu"
		", freeram=%llu"
		", sharedram=%llu"
		", bufferram=%llu"
		", totalswap=%llu"
		", freeswap=%llu"
		", procs=%u"
		", totalhigh=%llu"
		", freehigh=%llu"
		", mem_unit=%u"
		"}) = 0\n"
		, (unsigned long long) si->uptime
		, (unsigned long long) si->loads[0]
		, (unsigned long long) si->loads[1]
		, (unsigned long long) si->loads[2]
		, (unsigned long long) si->totalram
		, (unsigned long long) si->freeram
		, (unsigned long long) si->sharedram
		, (unsigned long long) si->bufferram
		, (unsigned long long) si->totalswap
		, (unsigned long long) si->freeswap
		, (unsigned) si->procs
		, (unsigned long long) si->totalhigh
		, (unsigned long long) si->freehigh
		, si->mem_unit
		);

	puts("+++ exited with 0 +++");
	return 0;
}
