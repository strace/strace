/*
 * Copyright (c) 2015-2017 Dmitry V. Levin <ldv@altlinux.org>
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
# include <sched.h>
# include <unistd.h>
# include "sched_attr.h"
# include "xlat.h"
# include "xlat/schedulers.h"

static const char *errstr;

static long
sys_sched_getattr(kernel_ulong_t pid, kernel_ulong_t attr,
		  kernel_ulong_t size, kernel_ulong_t flags)
{
	long rc = syscall(__NR_sched_getattr, pid, attr, size, flags);
	errstr = sprintrc(rc);
	return rc;
}

static long
sys_sched_setattr(kernel_ulong_t pid, kernel_ulong_t attr, kernel_ulong_t flags)
{
	long rc = syscall(__NR_sched_setattr, pid, attr, flags);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	static const kernel_ulong_t bogus_pid =
		(kernel_ulong_t) 0xdefacedfacefeedULL;
	static const kernel_ulong_t bogus_size =
		(kernel_ulong_t) 0xdefacedcafef00dULL;
	static const kernel_ulong_t bogus_flags =
		(kernel_ulong_t) 0xdefaceddeadc0deULL;

	TAIL_ALLOC_OBJECT_CONST_PTR(struct sched_attr, attr);
	TAIL_ALLOC_OBJECT_CONST_PTR(unsigned int, psize);
	void *const efault = attr + 1;

	sys_sched_getattr(0, 0, 0, 0);
	printf("sched_getattr(0, NULL, 0, 0) = %s\n", errstr);

	sys_sched_getattr(0, (unsigned long) attr, 0, 0);
	printf("sched_getattr(0, %p, 0, 0) = %s\n", attr, errstr);

	sys_sched_getattr(bogus_pid, 0, 0, 0);
	printf("sched_getattr(%d, NULL, 0, 0) = %s\n", (int) bogus_pid, errstr);

	sys_sched_getattr(-1U, (unsigned long) attr, bogus_size, bogus_flags);
	printf("sched_getattr(-1, %p, %s%u, %u) = %s\n",
	       attr,
# if defined __arm64__ || defined __aarch64__
	       "0xdefaced<<32|",
# else
	       "",
# endif
	       (unsigned) bogus_size, (unsigned) bogus_flags, errstr);

	sys_sched_getattr(0, (unsigned long) efault, sizeof(*attr), 0);
	printf("sched_getattr(0, %p, %u, 0) = %s\n",
	       efault, (unsigned) sizeof(*attr), errstr);

	if (sys_sched_getattr(0, (unsigned long) attr, sizeof(*attr), 0))
		perror_msg_and_skip("sched_getattr");
	printf("sched_getattr(0, {size=%u, sched_policy=", attr->size);
	printxval(schedulers, attr->sched_policy, NULL);
	printf(", sched_flags=%s, sched_nice=%d, sched_priority=%u"
	       ", sched_runtime=%" PRIu64 ", sched_deadline=%" PRIu64
	       ", sched_period=%" PRIu64 "}, %u, 0) = 0\n",
	       attr->sched_flags ? "SCHED_FLAG_RESET_ON_FORK" : "0",
	       attr->sched_nice,
	       attr->sched_priority,
	       attr->sched_runtime,
	       attr->sched_deadline,
	       attr->sched_period,
	       (unsigned) sizeof(*attr));

# if defined __arm64__ || defined __aarch64__
	long rc =
# endif
	sys_sched_getattr(F8ILL_KULONG_MASK, (unsigned long) attr,
			  F8ILL_KULONG_MASK | sizeof(*attr), F8ILL_KULONG_MASK);
# if defined __arm64__ || defined __aarch64__
	if (rc) {
		printf("sched_getattr(0, %p, 0xffffffff<<32|%u, 0) = %s\n",
		       attr, (unsigned) sizeof(*attr), errstr);
	} else
# endif
	{
		printf("sched_getattr(0, {size=%u, sched_policy=", attr->size);
		printxval(schedulers, attr->sched_policy, NULL);
		printf(", sched_flags=%s, sched_nice=%d, sched_priority=%u"
		       ", sched_runtime=%" PRIu64 ", sched_deadline=%" PRIu64
		       ", sched_period=%" PRIu64 "}, %u, 0) = 0\n",
		       attr->sched_flags ? "SCHED_FLAG_RESET_ON_FORK" : "0",
		       attr->sched_nice,
		       attr->sched_priority,
		       attr->sched_runtime,
		       attr->sched_deadline,
		       attr->sched_period,
		       (unsigned) sizeof(*attr));
	}

	sys_sched_setattr(bogus_pid, 0, 0);
	printf("sched_setattr(%d, NULL, 0) = %s\n", (int) bogus_pid, errstr);

	attr->sched_flags |= 1;

	if (sys_sched_setattr(0, (unsigned long) attr, 0))
		perror_msg_and_skip("sched_setattr");
	printf("sched_setattr(0, {size=%u, sched_policy=", attr->size);
	printxval(schedulers, attr->sched_policy, NULL);
	printf(", sched_flags=%s, sched_nice=%d, sched_priority=%u"
	       ", sched_runtime=%" PRIu64 ", sched_deadline=%" PRIu64
	       ", sched_period=%" PRIu64 "}, 0) = 0\n",
	       "SCHED_FLAG_RESET_ON_FORK",
	       attr->sched_nice,
	       attr->sched_priority,
	       attr->sched_runtime,
	       attr->sched_deadline,
	       attr->sched_period);

	sys_sched_setattr(F8ILL_KULONG_MASK, (unsigned long) attr,
			  F8ILL_KULONG_MASK);
	printf("sched_setattr(0, {size=%u, sched_policy=", attr->size);
	printxval(schedulers, attr->sched_policy, NULL);
	printf(", sched_flags=%s, sched_nice=%d, sched_priority=%u"
	       ", sched_runtime=%" PRIu64 ", sched_deadline=%" PRIu64
	       ", sched_period=%" PRIu64 "}, 0) = 0\n",
	       "SCHED_FLAG_RESET_ON_FORK",
	       attr->sched_nice,
	       attr->sched_priority,
	       attr->sched_runtime,
	       attr->sched_deadline,
	       attr->sched_period);

	*psize = attr->size;

	sys_sched_setattr(0, (unsigned long) psize, 0);
	printf("sched_setattr(0, %p, 0) = %s\n", psize, errstr);

	attr->size = 0;

	sys_sched_setattr(0, (unsigned long) attr, 0);
	printf("sched_setattr(0, {size=%u, sched_policy=", attr->size);
	printxval(schedulers, attr->sched_policy, NULL);
	printf(", sched_flags=%s, sched_nice=%d, sched_priority=%u"
	       ", sched_runtime=%" PRIu64 ", sched_deadline=%" PRIu64
	       ", sched_period=%" PRIu64 "}, 0) = 0\n",
	       "SCHED_FLAG_RESET_ON_FORK",
	       attr->sched_nice,
	       attr->sched_priority,
	       attr->sched_runtime,
	       attr->sched_deadline,
	       attr->sched_period);

	attr->size = 1;

	sys_sched_setattr(0, (unsigned long) attr, 0);
	printf("sched_setattr(0, {size=%u} => {size=%u}, 0) = %s\n",
	       1, attr->size, errstr);

	attr->size = SCHED_ATTR_MIN_SIZE - 1;

	sys_sched_setattr(0, (unsigned long) attr, 0);
	printf("sched_setattr(0, {size=%u} => {size=%u}, 0) = %s\n",
	       SCHED_ATTR_MIN_SIZE - 1, attr->size, errstr);

	attr->size = 0x90807060;
	attr->sched_policy = 0xca7faced;
	attr->sched_flags = 0xbadc0ded1057da7aULL;
	attr->sched_nice = 0xafbfcfdf;
	attr->sched_priority = 0xb8c8d8e8;
	attr->sched_runtime = 0xbadcaffedeadf157ULL;
	attr->sched_deadline = 0xc0de70a57badac75ULL;
	attr->sched_period = 0xded1ca7edda7aca7ULL;

	sys_sched_setattr(bogus_pid, (unsigned long) attr, bogus_flags);
	printf("sched_setattr(%d, {size=%u, sched_policy=%#x /* SCHED_??? */, "
	       "sched_flags=%#" PRIx64 " /* SCHED_FLAG_??? */, "
	       "sched_nice=%d, sched_priority=%u, sched_runtime=%" PRIu64 ", "
	       "sched_deadline=%" PRIu64 ", sched_period=%" PRIu64 ", ...}, %u)"
	       " = %s\n",
	       (int) bogus_pid,
	       attr->size,
	       attr->sched_policy,
	       attr->sched_flags,
	       attr->sched_nice,
	       attr->sched_priority,
	       attr->sched_runtime,
	       attr->sched_deadline,
	       attr->sched_period,
	       (unsigned) bogus_flags, errstr);

	if (F8ILL_KULONG_SUPPORTED) {
		const kernel_ulong_t ill = f8ill_ptr_to_kulong(attr);

		sys_sched_getattr(0, ill, sizeof(*attr), 0);
		printf("sched_getattr(0, %#llx, %u, 0) = %s\n",
		       (unsigned long long) ill, (unsigned) sizeof(*attr),
		       errstr);

		sys_sched_setattr(0, ill, 0);
		printf("sched_setattr(0, %#llx, 0) = %s\n",
		       (unsigned long long) ill, errstr);
	}

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_sched_getattr && __NR_sched_setattr")

#endif
