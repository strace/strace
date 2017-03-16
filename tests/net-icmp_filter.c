/*
 * Check decoding of ICMP_FILTER.
 *
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
	printf("getsockopt(-1, SOL_RAW, ICMP_FILTER, %p, %p) = -1 %s (%m)\n",
	       f, plen, errno2name());

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
