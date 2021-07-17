/*
 * Check decoding of sched_getattr and sched_setattr syscalls.
 *
 * Copyright (c) 2015-2017 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <inttypes.h>
#include <stdio.h>
#include <sched.h>
#include <unistd.h>
#include "pidns.h"
#include "sched_attr.h"
#include "xlat.h"
#include "xlat/schedulers.h"

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
	PIDNS_TEST_INIT;

	static const kernel_ulong_t bogus_pid =
		(kernel_ulong_t) 0xdefacedfacefeedULL;
	static const kernel_ulong_t bogus_size =
		(kernel_ulong_t) 0xdefacedcafef00dULL;
	static const kernel_ulong_t bogus_flags =
		(kernel_ulong_t) 0xdefaceddeadc0deULL;

	const int pid = getpid();
	const char *pid_str = pidns_pid2str(PT_TGID);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct sched_attr, attr);
	TAIL_ALLOC_OBJECT_CONST_PTR(unsigned int, psize);
	void *const efault = attr + 1;

	sys_sched_getattr(pid, 0, 0, 0);
	pidns_print_leader();
	printf("sched_getattr(%d%s, NULL, 0, 0) = %s\n",
		pid, pid_str, errstr);

	sys_sched_getattr(0, (unsigned long) attr, 0, 0);
	pidns_print_leader();
	printf("sched_getattr(0, %p, 0, 0) = %s\n", attr, errstr);

	sys_sched_getattr(bogus_pid, 0, 0, 0);
	pidns_print_leader();
	printf("sched_getattr(%d, NULL, 0, 0) = %s\n", (int) bogus_pid, errstr);

	sys_sched_getattr(-1U, (unsigned long) attr, bogus_size, bogus_flags);
	pidns_print_leader();
	printf("sched_getattr(-1, %p, %s%u, %u) = %s\n",
	       attr,
#if defined __arm64__ || defined __aarch64__
	       "0xdefaced<<32|",
#else
	       "",
#endif
	       (unsigned) bogus_size, (unsigned) bogus_flags, errstr);

	sys_sched_getattr(0, (unsigned long) efault, SCHED_ATTR_MIN_SIZE, 0);
	pidns_print_leader();
	printf("sched_getattr(0, %p, %u, 0) = %s\n",
	       efault, (unsigned) SCHED_ATTR_MIN_SIZE, errstr);

	if (sys_sched_getattr(0, (unsigned long) attr, SCHED_ATTR_MIN_SIZE, 0))
		perror_msg_and_skip("sched_getattr");
	pidns_print_leader();
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
	       (unsigned) SCHED_ATTR_MIN_SIZE);

	sys_sched_getattr(0, (unsigned long) efault, sizeof(*attr), 0);
	pidns_print_leader();
	printf("sched_getattr(0, %p, %u, 0) = %s\n",
	       efault, (unsigned) sizeof(*attr), errstr);

	if (sys_sched_getattr(0, (unsigned long) attr, sizeof(*attr), 0))
		perror_msg_and_skip("sched_getattr");
	pidns_print_leader();
	printf("sched_getattr(0, {size=%u, sched_policy=", attr->size);
	printxval(schedulers, attr->sched_policy, NULL);
	printf(", sched_flags=%s, sched_nice=%d, sched_priority=%u"
	       ", sched_runtime=%" PRIu64 ", sched_deadline=%" PRIu64
	       ", sched_period=%" PRIu64,
	       attr->sched_flags ? "SCHED_FLAG_RESET_ON_FORK" : "0",
	       attr->sched_nice,
	       attr->sched_priority,
	       attr->sched_runtime,
	       attr->sched_deadline,
	       attr->sched_period);
	if (attr->size >= SCHED_ATTR_SIZE_VER1) {
		printf(", sched_util_min=%u, sched_util_max=%u",
		       attr->sched_util_min,
		       attr->sched_util_max);
	}
	printf("}, %u, 0) = 0\n", (unsigned) sizeof(*attr));

#if defined __arm64__ || defined __aarch64__
	long rc =
#endif
	sys_sched_getattr(F8ILL_KULONG_MASK, (unsigned long) attr,
			  F8ILL_KULONG_MASK | sizeof(*attr), F8ILL_KULONG_MASK);
#if defined __arm64__ || defined __aarch64__
	if (rc) {
		pidns_print_leader();
		printf("sched_getattr(0, %p, 0xffffffff<<32|%u, 0) = %s\n",
		       attr, (unsigned) sizeof(*attr), errstr);
	} else
#endif
	{
		pidns_print_leader();
		printf("sched_getattr(0, {size=%u, sched_policy=", attr->size);
		printxval(schedulers, attr->sched_policy, NULL);
		printf(", sched_flags=%s, sched_nice=%d, sched_priority=%u"
		       ", sched_runtime=%" PRIu64 ", sched_deadline=%" PRIu64
		       ", sched_period=%" PRIu64,
		       attr->sched_flags ? "SCHED_FLAG_RESET_ON_FORK" : "0",
		       attr->sched_nice,
		       attr->sched_priority,
		       attr->sched_runtime,
		       attr->sched_deadline,
		       attr->sched_period);
		if (attr->size >= SCHED_ATTR_SIZE_VER1) {
			printf(", sched_util_min=%u, sched_util_max=%u",
			       attr->sched_util_min,
			       attr->sched_util_max);
		}
		printf("}, %u, 0) = 0\n", (unsigned) sizeof(*attr));
	}

	sys_sched_setattr(bogus_pid, 0, 0);
	pidns_print_leader();
	printf("sched_setattr(%d, NULL, 0) = %s\n", (int) bogus_pid, errstr);

	attr->sched_flags |= 1;

	if (sys_sched_setattr(pid, (unsigned long) attr, 0))
		perror_msg_and_skip("sched_setattr");
	pidns_print_leader();
	printf("sched_setattr(%d%s, {size=%u, sched_policy=",
		pid, pid_str, attr->size);
	printxval(schedulers, attr->sched_policy, NULL);
	printf(", sched_flags=%s, sched_nice=%d, sched_priority=%u"
	       ", sched_runtime=%" PRIu64 ", sched_deadline=%" PRIu64
	       ", sched_period=%" PRIu64,
	       "SCHED_FLAG_RESET_ON_FORK",
	       attr->sched_nice,
	       attr->sched_priority,
	       attr->sched_runtime,
	       attr->sched_deadline,
	       attr->sched_period);
	if (attr->size >= SCHED_ATTR_SIZE_VER1) {
		printf(", sched_util_min=%u, sched_util_max=%u",
		       attr->sched_util_min,
		       attr->sched_util_max);
	}
	printf("}, 0) = 0\n");

	sys_sched_setattr(F8ILL_KULONG_MASK, (unsigned long) attr,
			  F8ILL_KULONG_MASK);
	pidns_print_leader();
	printf("sched_setattr(0, {size=%u, sched_policy=", attr->size);
	printxval(schedulers, attr->sched_policy, NULL);
	printf(", sched_flags=%s, sched_nice=%d, sched_priority=%u"
	       ", sched_runtime=%" PRIu64 ", sched_deadline=%" PRIu64
	       ", sched_period=%" PRIu64,
	       "SCHED_FLAG_RESET_ON_FORK",
	       attr->sched_nice,
	       attr->sched_priority,
	       attr->sched_runtime,
	       attr->sched_deadline,
	       attr->sched_period);
	if (attr->size >= SCHED_ATTR_SIZE_VER1) {
		printf(", sched_util_min=%u, sched_util_max=%u",
		       attr->sched_util_min,
		       attr->sched_util_max);
	}
	printf("}, 0) = 0\n");

	*psize = attr->size;

	sys_sched_setattr(0, (unsigned long) psize, 0);
	pidns_print_leader();
	printf("sched_setattr(0, %p, 0) = %s\n", psize, errstr);

	attr->size = 0;

	sys_sched_setattr(0, (unsigned long) attr, 0);
	pidns_print_leader();
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
	pidns_print_leader();
	printf("sched_setattr(0, {size=%u} => {size=%u}, 0) = %s\n",
	       1, attr->size, errstr);

	attr->size = SCHED_ATTR_MIN_SIZE - 1;

	sys_sched_setattr(0, (unsigned long) attr, 0);
	pidns_print_leader();
	printf("sched_setattr(0, {size=%u} => {size=%u}, 0) = %s\n",
	       SCHED_ATTR_MIN_SIZE - 1, attr->size, errstr);

	attr->size = 0x90807060;
	attr->sched_policy = 0xca7faced;
	attr->sched_flags = 0xbadc0ded1057da80ULL;
	attr->sched_nice = 0xafbfcfdf;
	attr->sched_priority = 0xb8c8d8e8;
	attr->sched_runtime = 0xbadcaffedeadf157ULL;
	attr->sched_deadline = 0xc0de70a57badac75ULL;
	attr->sched_period = 0xded1ca7edda7aca7ULL;

	sys_sched_setattr(bogus_pid, (unsigned long) attr, bogus_flags);
	pidns_print_leader();
	printf("sched_setattr(%d, {size=%u, sched_policy=%#x /* SCHED_??? */, "
	       "sched_flags=%#" PRIx64 " /* SCHED_FLAG_??? */, "
	       "sched_nice=%d, sched_priority=%u, sched_runtime=%" PRIu64 ", "
	       "sched_deadline=%" PRIu64 ", sched_period=%" PRIu64 ", "
	       "sched_util_min=%u, sched_util_max=%u, ...}, %u) = %s\n",
	       (int) bogus_pid,
	       attr->size,
	       attr->sched_policy,
	       attr->sched_flags,
	       attr->sched_nice,
	       attr->sched_priority,
	       attr->sched_runtime,
	       attr->sched_deadline,
	       attr->sched_period,
	       attr->sched_util_min,
	       attr->sched_util_max,
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


	attr->size = 0x90807060;
	attr->sched_policy = 0xca7faced;
	attr->sched_flags = 0x87ULL;
	attr->sched_nice = 0xafbfcfdf;
	attr->sched_priority = 0xb8c8d8e8;
	attr->sched_runtime = 0xbadcaffedeadf157ULL;
	attr->sched_deadline = 0xc0de70a57badac75ULL;
	attr->sched_period = 0xded1ca7edda7aca7ULL;

	sys_sched_setattr(bogus_pid, (unsigned long) attr, bogus_flags);
	pidns_print_leader();
	printf("sched_setattr(%d, {size=%u, sched_policy=%#x /* SCHED_??? */, "
	       "sched_flags=SCHED_FLAG_RESET_ON_FORK|SCHED_FLAG_RECLAIM|"
	       "SCHED_FLAG_DL_OVERRUN|0x80, "
	       "sched_nice=%d, sched_priority=%u, sched_runtime=%" PRIu64 ", "
	       "sched_deadline=%" PRIu64 ", sched_period=%" PRIu64 ", "
	       "sched_util_min=%u, sched_util_max=%u, ...}, %u) = %s\n",
	       (int) bogus_pid,
	       attr->size,
	       attr->sched_policy,
	       attr->sched_nice,
	       attr->sched_priority,
	       attr->sched_runtime,
	       attr->sched_deadline,
	       attr->sched_period,
	       attr->sched_util_min,
	       attr->sched_util_max,
	       (unsigned) bogus_flags, errstr);

	if (F8ILL_KULONG_SUPPORTED) {
		const kernel_ulong_t ill = f8ill_ptr_to_kulong(attr);

		sys_sched_getattr(0, ill, sizeof(*attr), 0);
		pidns_print_leader();
		printf("sched_getattr(0, %#llx, %u, 0) = %s\n",
		       (unsigned long long) ill, (unsigned) sizeof(*attr),
		       errstr);

		sys_sched_setattr(0, ill, 0);
		pidns_print_leader();
		printf("sched_setattr(0, %#llx, 0) = %s\n",
		       (unsigned long long) ill, errstr);
	}

	attr->size = SCHED_ATTR_MIN_SIZE;
	attr->sched_policy = 0xdefaced;
	attr->sched_flags = 0x8fULL;

	sys_sched_setattr(bogus_pid, (unsigned long) attr, bogus_flags);
	pidns_print_leader();
	printf("sched_setattr(%d, {size=%u, "
	       "sched_flags=SCHED_FLAG_RESET_ON_FORK|SCHED_FLAG_RECLAIM|"
	       "SCHED_FLAG_DL_OVERRUN|SCHED_FLAG_KEEP_POLICY|0x80, "
	       "sched_nice=%d, sched_priority=%u, sched_runtime=%" PRIu64 ", "
	       "sched_deadline=%" PRIu64 ", sched_period=%" PRIu64 "}, %u)"
	       " = %s\n",
	       (int) bogus_pid,
	       attr->size,
	       attr->sched_nice,
	       attr->sched_priority,
	       attr->sched_runtime,
	       attr->sched_deadline,
	       attr->sched_period,
	       (unsigned) bogus_flags, errstr);

	if (F8ILL_KULONG_SUPPORTED) {
		const kernel_ulong_t ill = f8ill_ptr_to_kulong(attr);

		sys_sched_getattr(0, ill, sizeof(*attr), 0);
		pidns_print_leader();
		printf("sched_getattr(0, %#llx, %u, 0) = %s\n",
		       (unsigned long long) ill, (unsigned) sizeof(*attr),
		       errstr);

		sys_sched_setattr(0, ill, 0);
		pidns_print_leader();
		printf("sched_setattr(0, %#llx, 0) = %s\n",
		       (unsigned long long) ill, errstr);
	}

	attr->size = SCHED_ATTR_SIZE_VER1;
	attr->sched_flags = 0xe7ULL;

	sys_sched_setattr(bogus_pid, (unsigned long) attr, bogus_flags);
	pidns_print_leader();
	printf("sched_setattr(%d, {size=%u, sched_policy=%#x /* SCHED_??? */, "
	       "sched_flags=SCHED_FLAG_RESET_ON_FORK|SCHED_FLAG_RECLAIM|"
	       "SCHED_FLAG_DL_OVERRUN|SCHED_FLAG_UTIL_CLAMP_MIN"
	       "|SCHED_FLAG_UTIL_CLAMP_MAX|0x80, "
	       "sched_nice=%d, sched_priority=%u, sched_runtime=%" PRIu64 ", "
	       "sched_deadline=%" PRIu64 ", sched_period=%" PRIu64 ", "
	       "sched_util_min=%u, sched_util_max=%u}, %u) = %s\n",
	       (int) bogus_pid,
	       attr->size,
	       attr->sched_policy,
	       attr->sched_nice,
	       attr->sched_priority,
	       attr->sched_runtime,
	       attr->sched_deadline,
	       attr->sched_period,
	       attr->sched_util_min,
	       attr->sched_util_max,
	       (unsigned) bogus_flags, errstr);

	if (F8ILL_KULONG_SUPPORTED) {
		const kernel_ulong_t ill = f8ill_ptr_to_kulong(attr);

		sys_sched_getattr(0, ill, sizeof(*attr), 0);
		pidns_print_leader();
		printf("sched_getattr(0, %#llx, %u, 0) = %s\n",
		       (unsigned long long) ill, (unsigned) sizeof(*attr),
		       errstr);

		sys_sched_setattr(0, ill, 0);
		pidns_print_leader();
		printf("sched_setattr(0, %#llx, 0) = %s\n",
		       (unsigned long long) ill, errstr);
	}

	attr->sched_flags = 0xcaffee90LL;

	sys_sched_setattr(bogus_pid, (unsigned long) attr, bogus_flags);
	pidns_print_leader();
	printf("sched_setattr(%d, {size=%u, sched_flags=SCHED_FLAG_KEEP_PARAMS"
	       "|0xcaffee80, sched_util_min=%u, sched_util_max=%u}, %u) = %s\n",
	       (int) bogus_pid,
	       attr->size,
	       attr->sched_util_min,
	       attr->sched_util_max,
	       (unsigned) bogus_flags, errstr);

	if (F8ILL_KULONG_SUPPORTED) {
		const kernel_ulong_t ill = f8ill_ptr_to_kulong(attr);

		sys_sched_getattr(0, ill, sizeof(*attr), 0);
		pidns_print_leader();
		printf("sched_getattr(0, %#llx, %u, 0) = %s\n",
		       (unsigned long long) ill, (unsigned) sizeof(*attr),
		       errstr);

		sys_sched_setattr(0, ill, 0);
		pidns_print_leader();
		printf("sched_setattr(0, %#llx, 0) = %s\n",
		       (unsigned long long) ill, errstr);
	}

	pidns_print_leader();
	puts("+++ exited with 0 +++");
	return 0;
}
