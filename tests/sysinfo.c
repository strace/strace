/*
 * This file is part of sysinfo strace test.
 *
 * Copyright (c) 2015 Elvira Khabirova <lineprinter0@gmail.com>
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
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
