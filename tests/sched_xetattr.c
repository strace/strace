/*
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
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

#if defined __NR_sched_getattr && defined __NR_sched_setattr

# include <inttypes.h>
# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	struct {
		uint32_t size;
		uint32_t sched_policy;
		uint64_t sched_flags;
		int32_t  sched_nice;
		uint32_t sched_priority;
		uint64_t sched_runtime;
		uint64_t sched_deadline;
		uint64_t sched_period;
	} *sched_attr = tail_alloc(sizeof(*sched_attr));

	long rc = syscall(__NR_sched_getattr, 0xdeadface, NULL, 0, 0);
	printf("sched_getattr\\(%d, NULL, 0, 0\\) += %s\n",
		0xdeadface, sprintrc_grep(rc));

	rc = syscall(__NR_sched_getattr, -1,
		     sched_attr, 0xbadfaced, 0xc0defeed);
	printf("sched_getattr\\(-1, %p, %u, %u\\) += %s\n",
		sched_attr, 0xbadfaced, 0xc0defeed, sprintrc_grep(rc));

	rc = syscall(__NR_sched_getattr, 0,
		     sched_attr + 1, sizeof(*sched_attr), 0);
	printf("sched_getattr\\(0, %p, %u, 0\\) += %s\n",
		sched_attr + 1, (unsigned)sizeof(*sched_attr),
		sprintrc_grep(rc));

	if (syscall(__NR_sched_getattr, 0, sched_attr, sizeof(*sched_attr), 0))
		perror_msg_and_skip("sched_getattr");

	printf("sched_getattr\\(0, \\{size=%u, sched_policy=SCHED_[A-Z]+"
	       ", sched_flags=%s, sched_nice=%d, sched_priority=%u"
	       ", sched_runtime=%" PRIu64 ", sched_deadline=%" PRIu64
	       ", sched_period=%" PRIu64 "\\}, %u, 0\\) += 0\n",
	       sched_attr->size,
	       sched_attr->sched_flags ? "SCHED_FLAG_RESET_ON_FORK" : "0",
	       sched_attr->sched_nice,
	       sched_attr->sched_priority,
	       sched_attr->sched_runtime,
	       sched_attr->sched_deadline,
	       sched_attr->sched_period,
	       (unsigned) sizeof(*sched_attr));

	sched_attr->sched_flags |= 1;
	if (syscall(__NR_sched_setattr, 0, sched_attr, 0))
		perror_msg_and_skip("sched_setattr");

	printf("sched_setattr\\(0, \\{size=%u, sched_policy=SCHED_[A-Z]+"
	       ", sched_flags=%s, sched_nice=%d, sched_priority=%u"
	       ", sched_runtime=%" PRIu64 ", sched_deadline=%" PRIu64
	       ", sched_period=%" PRIu64 "\\}, 0\\) += 0\n",
	       sched_attr->size,
	       "SCHED_FLAG_RESET_ON_FORK",
	       sched_attr->sched_nice,
	       sched_attr->sched_priority,
	       sched_attr->sched_runtime,
	       sched_attr->sched_deadline,
	       sched_attr->sched_period);

	sched_attr->size = 0x90807060;
	sched_attr->sched_policy = 0xca7faced;
	sched_attr->sched_flags = 0xbadc0ded1057da7aULL;
	sched_attr->sched_nice = 0xafbfcfdf;
	sched_attr->sched_priority = 0xb8c8d8e8;
	sched_attr->sched_runtime = 0xbadcaffedeadf157ULL;
	sched_attr->sched_deadline = 0xc0de70a57badac75ULL;
	sched_attr->sched_period = 0xded1ca7edda7aca7ULL;

	rc = syscall(__NR_sched_setattr, 0xfacec0de, sched_attr, 0xbeeff00d);

	printf("sched_setattr\\(%d, \\{size=%u, "
		"sched_policy=%#x /\\* SCHED_\\?\\?\\? \\*/, "
		"sched_flags=%#" PRIx64 " /\\* SCHED_FLAG_\\?\\?\\? \\*/, "
		"sched_nice=%d, sched_priority=%u, sched_runtime=%" PRIu64 ", "
		"sched_deadline=%" PRIu64 ", sched_period=%" PRIu64 "\\}, "
		"%u\\) += %s\n",
		0xfacec0de, sched_attr->size,
		sched_attr->sched_policy,
		sched_attr->sched_flags,
		sched_attr->sched_nice,
		sched_attr->sched_priority,
		sched_attr->sched_runtime,
		sched_attr->sched_deadline,
		sched_attr->sched_period, 0xbeeff00d, sprintrc_grep(rc));

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_sched_getattr && __NR_sched_setattr")

#endif
