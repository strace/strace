/*
 * Copyright (c) 2002-2004 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2009-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2014-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <sched.h>

static unsigned int
get_cpuset_size(void)
{
	static unsigned int cpuset_size;

	if (!cpuset_size) {
		/*
		 * If the cpuset size passed to sched_getaffinity is less
		 * than necessary to store the bitmask, the kernel does not
		 * look at the mask pointer and fails with EINVAL.
		 *
		 * If the cpuset size is large enough, the kernel fails with
		 * EFAULT on inaccessible mask pointers.
		 *
		 * This undocumented kernel feature can be used to probe
		 * the kernel and find out the minimal valid cpuset size
		 * without allocating any memory for the CPU affinity mask.
		 *
		 */
		cpuset_size = 128;
		/* 
		 * As an extra kludge, use a dummy variable in order
		 * to dodge the _Nonnull pointer attribute from
		 * sched_getaffinity()'s declaration
		 */
		cpu_set_t *null_ptr = NULL;
		while (cpuset_size &&
		       sched_getaffinity(0, cpuset_size, null_ptr) == -1 &&
		       EINVAL == errno) {
			cpuset_size <<= 1;
		}
		if (!cpuset_size)
			cpuset_size = 128;
	}

	return cpuset_size;
}

void
print_affinitylist(struct tcb *const tcp, const kernel_ulong_t addr,
		   const unsigned int len)
{
	const unsigned int max_size = get_cpuset_size();
	const unsigned int umove_size = MIN(len, max_size);
	const unsigned int size = ROUNDUP(umove_size, current_wordsize);
	const unsigned int ncpu = size * 8;
	void *cpu;

	if (!verbose(tcp) || (exiting(tcp) && syserror(tcp)) ||
	    !addr) {
		printaddr(addr);
		return;
	}

	if (!len) {
		tprint_array_begin();
		tprint_array_end();
		return;
	}

	if (!(cpu = calloc(size, 1))) {
		printaddr(addr);
		return;
	}

	if (!umoven_or_printaddr(tcp, addr, umove_size, cpu)) {
		bool printed = false;

		tprint_bitset_begin();
		for (int i = 0;; i++) {
			i = next_set_bit(cpu, i, ncpu);
			if (i < 0)
				break;
			if (printed)
				tprint_bitset_next();
			else
				printed = true;
			PRINT_VAL_D(i);
		}
		if (size < len) {
			if (printed)
				tprint_bitset_next();
			tprint_more_data_follows();
		}
		tprint_bitset_end();
	}

	free(cpu);
}

SYS_FUNC(sched_setaffinity)
{
	/* pid */
	tprints_arg_name("pid");
	const int pid = tcp->u_arg[0];
	printpid(tcp, pid, PT_TGID);

	/* cpusetsize */
	tprints_arg_next_name("cpusetsize");
	const unsigned int len = tcp->u_arg[1];
	PRINT_VAL_U(len);

	/* mask */
	tprints_arg_next_name("mask");
	print_affinitylist(tcp, tcp->u_arg[2], len);

	return RVAL_DECODED;
}

SYS_FUNC(sched_getaffinity)
{
	if (entering(tcp)) {
		/* pid */
		tprints_arg_name("pid");
		const int pid = tcp->u_arg[0];
		printpid(tcp, pid, PT_TGID);

		/* cpusetsize */
		tprints_arg_next_name("cpusetsize");
		const unsigned int len = tcp->u_arg[1];
		PRINT_VAL_U(len);
	} else {
		/* mask */
		tprints_arg_next_name("mask");
		print_affinitylist(tcp, tcp->u_arg[2], tcp->u_rval);
	}
	return 0;
}
