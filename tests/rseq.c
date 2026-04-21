/*
 * Check decoding of rseq syscall.
 *
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <linux/rseq.h>

enum {
	RSEQ_TEST_ALIGN	= 32,
	RSEQ_TEST_MIN	= 32,
};

static_assert(sizeof(struct rseq) >= RSEQ_TEST_MIN,
	      "rseq buffer covers minimum decode length");
static_assert(sizeof(struct rseq) % RSEQ_TEST_MIN == 0,
	      "rseq buffer is properly aligned");

static const char *errstr;

static long
k_rseq(const void *rseq, const unsigned int rseq_len,
       const unsigned int flags, const unsigned int sig)
{
	const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t arg1 = (uintptr_t) rseq;
	const kernel_ulong_t arg2 = fill | rseq_len;
	const kernel_ulong_t arg3 = fill | flags;
	const kernel_ulong_t arg4 = fill | sig;
	const long rc = syscall(__NR_rseq, arg1, arg2, arg3, arg4, bad, bad);

	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(struct rseq, rseq);
	void *const efault = rseq + 1;
	unsigned char *const mis = (unsigned char *) rseq + 1;
	unsigned int sig = 0xfacefed0U;

	k_rseq(rseq, RSEQ_TEST_MIN - 1, 0, ++sig);
	printf("rseq(%p, %u, 0, %#x) = %s\n",
	       rseq, RSEQ_TEST_MIN - 1, sig, errstr);

	k_rseq(NULL, RSEQ_TEST_ALIGN, -1, ++sig);
	printf("rseq(NULL, %u, %s, %#x) = %s\n",
	       RSEQ_TEST_ALIGN,
	       "RSEQ_FLAG_UNREGISTER|RSEQ_FLAG_SLICE_EXT_DEFAULT_ON|0xfffffffc",
	       sig, errstr);

	k_rseq(mis, RSEQ_TEST_MIN, RSEQ_FLAG_SLICE_EXT_DEFAULT_ON, ++sig);
	printf("rseq(%p, %u, %s, %#x) = %s\n",
	       mis, RSEQ_TEST_MIN,
	       "RSEQ_FLAG_SLICE_EXT_DEFAULT_ON",
	       sig, errstr);

	k_rseq(efault, RSEQ_TEST_MIN, RSEQ_FLAG_UNREGISTER, ++sig);
	printf("rseq(%p, %u, RSEQ_FLAG_UNREGISTER, %#x) = %s\n",
	       efault, RSEQ_TEST_MIN, sig, errstr);

	k_rseq(rseq, RSEQ_TEST_MIN, 0xfffffffc, ++sig);
	printf("rseq({cpu_id_start=%s, cpu_id=%s, rseq_cs=%#jx, flags=%s"
	       ", node_id=%u, mm_cid=%u, slice_ctrl={request=%u, granted=%u"
	       ", __reserved=%#x}, __reserved=0}, %u"
	       ", 0xfffffffc /* RSEQ_FLAG_??? */, %#x) = %s\n",
	       "RSEQ_CPU_ID_UNINITIALIZED",
	       "RSEQ_CPU_ID_UNINITIALIZED",
	       (uintmax_t) rseq->rseq_cs,
	       "RSEQ_CS_FLAG_NO_RESTART_ON_PREEMPT|"
		"RSEQ_CS_FLAG_NO_RESTART_ON_SIGNAL|"
		"RSEQ_CS_FLAG_NO_RESTART_ON_MIGRATE|"
		"RSEQ_CS_FLAG_SLICE_EXT_AVAILABLE|"
		"RSEQ_CS_FLAG_SLICE_EXT_ENABLED|"
		"0xffffffc8",
	       rseq->node_id,
	       rseq->mm_cid,
	       rseq->slice_ctrl.request,
	       rseq->slice_ctrl.granted,
	       rseq->slice_ctrl.__reserved,
	       RSEQ_TEST_MIN, sig, errstr);

	memset(rseq, 0, sizeof(*rseq));
	rseq->rseq_cs = (uintptr_t) efault;
	rseq->flags = RSEQ_CS_FLAG_NO_RESTART_ON_PREEMPT;
	rseq->__reserved = 0xad;
	k_rseq(rseq, sizeof(*rseq), RSEQ_FLAG_UNREGISTER, ++sig);
	printf("rseq({cpu_id_start=0, cpu_id=0, rseq_cs=%p, flags=%s"
	       ", node_id=0, mm_cid=0, slice_ctrl={request=0, granted=0"
	       ", __reserved=0}, __reserved=%#x}, %u, RSEQ_FLAG_UNREGISTER"
	       ", %#x) = %s\n",
	       efault, "RSEQ_CS_FLAG_NO_RESTART_ON_PREEMPT",
	       rseq->__reserved, (unsigned int) sizeof(*rseq), sig, errstr);

	if (F8ILL_KULONG_SUPPORTED) {
		memset(rseq, 0, sizeof(*rseq));
		rseq->rseq_cs = (typeof(rseq->rseq_cs)) F8ILL_KULONG_MASK;
		k_rseq(rseq, RSEQ_TEST_MIN, RSEQ_FLAG_UNREGISTER, ++sig);
		printf("rseq({cpu_id_start=0, cpu_id=0, rseq_cs=%#llx, flags=0"
		       ", node_id=0, mm_cid=0, slice_ctrl={request=0, granted=0"
		       ", __reserved=0}, __reserved=0}, %u"
		       ", RSEQ_FLAG_UNREGISTER, %#x) = %s\n",
		       (unsigned long long) (uint64_t) F8ILL_KULONG_MASK,
		       RSEQ_TEST_MIN, sig, errstr);
	}

	memset(rseq, 0, sizeof(*rseq));
	TAIL_ALLOC_OBJECT_CONST_PTR(struct rseq_cs, cs);
	fill_memory(cs, sizeof(*cs));
	cs->flags = 0x37;
	rseq->rseq_cs = (uintptr_t) cs;
	k_rseq(rseq, RSEQ_TEST_MIN, RSEQ_FLAG_UNREGISTER, ++sig);
	printf("rseq({cpu_id_start=%u, cpu_id=%u"
	       ", rseq_cs={version=%u, flags=%s, start_ip=%#jx"
	       ", post_commit_offset=%#jx, abort_ip=%#jx} /* %p */"
	       ", flags=0, node_id=0, mm_cid=0"
	       ", slice_ctrl={request=0, granted=0, __reserved=0}"
	       ", __reserved=0}, %u, RSEQ_FLAG_UNREGISTER, %#x) = %s\n",
	       rseq->cpu_id_start, rseq->cpu_id,
	       cs->version,
	       "RSEQ_CS_FLAG_NO_RESTART_ON_PREEMPT"
	        "|RSEQ_CS_FLAG_NO_RESTART_ON_SIGNAL"
		"|RSEQ_CS_FLAG_NO_RESTART_ON_MIGRATE"
		"|RSEQ_CS_FLAG_SLICE_EXT_AVAILABLE"
		"|RSEQ_CS_FLAG_SLICE_EXT_ENABLED",
	       (uintmax_t) cs->start_ip,
	       (uintmax_t) cs->post_commit_offset,
	       (uintmax_t) cs->abort_ip,
	       cs,
	       RSEQ_TEST_MIN, sig, errstr);

	static const struct strval32 cpu_id_atoms[] = {
		{ ARG_STR(RSEQ_CPU_ID_UNINITIALIZED) },
		{ ARG_STR(RSEQ_CPU_ID_REGISTRATION_FAILED) },
		{ ARG_STR(0) },
		{ ARG_STR(4294967293) },
	};

	memset(rseq, 0, sizeof(*rseq));
	rseq->flags = 0xffffffc8;
	for (size_t si = 0; si < ARRAY_SIZE(cpu_id_atoms); si++) {
		for (size_t ci = 0; ci < ARRAY_SIZE(cpu_id_atoms); ci++) {
			const struct strval32 *const s = &cpu_id_atoms[si];
			const struct strval32 *const c = &cpu_id_atoms[ci];

			rseq->cpu_id_start = s->val;
			rseq->cpu_id = c->val;
			k_rseq(rseq, RSEQ_TEST_MIN, RSEQ_FLAG_UNREGISTER, ++sig);
			printf("rseq({cpu_id_start=%s, cpu_id=%s, rseq_cs=NULL"
			       ", flags=%#x /* RSEQ_CS_FLAG_??? */"
			       ", node_id=0, mm_cid=0, slice_ctrl="
			       "{request=0, granted=0, __reserved=0}"
			       ", __reserved=0}, %u, RSEQ_FLAG_UNREGISTER"
			       ", %#x) = %s\n",
			       s->str, c->str, rseq->flags,
			       RSEQ_TEST_MIN, sig, errstr);
		}
	}

	puts("+++ exited with 0 +++");
	return 0;
}
