/*
 * Copyright (c) 2002-2004 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2009-2016 Dmitry V. Levin <ldv@altlinux.org>
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
		 */
		cpuset_size = 128;
		while (cpuset_size &&
		       sched_getaffinity(0, cpuset_size, NULL) == -1 &&
		       EINVAL == errno) {
			cpuset_size <<= 1;
		}
		if (!cpuset_size)
			cpuset_size = 128;
	}

	return cpuset_size;
}

static void
print_affinitylist(struct tcb *const tcp, const kernel_ulong_t addr,
		   const unsigned int len)
{
	const unsigned int max_size = get_cpuset_size();
	const unsigned int umove_size = len < max_size ? len : max_size;
	const unsigned int size =
		(umove_size + current_wordsize - 1) & -current_wordsize;
	const unsigned int ncpu = size * 8;
	void *cpu;

	if (!verbose(tcp) || (exiting(tcp) && syserror(tcp)) ||
	    !addr || !len || !(cpu = calloc(size, 1))) {
		printaddr(addr);
		return;
	}

	if (!umoven_or_printaddr(tcp, addr, umove_size, cpu)) {
		int i = 0;
		const char *sep = "";

		tprints("[");
		for (;; i++) {
			i = next_set_bit(cpu, i, ncpu);
			if (i < 0)
				break;
			tprintf("%s%d", sep, i);
			sep = ", ";
		}
		if (size < len)
			tprintf("%s...", sep);
		tprints("]");
	}

	free(cpu);
}

SYS_FUNC(sched_setaffinity)
{
	const int pid = tcp->u_arg[0];
	const unsigned int len = tcp->u_arg[1];

	tprintf("%d, %u, ", pid, len);
	print_affinitylist(tcp, tcp->u_arg[2], len);

	return RVAL_DECODED;
}

SYS_FUNC(sched_getaffinity)
{
	const int pid = tcp->u_arg[0];
	const unsigned int len = tcp->u_arg[1];

	if (entering(tcp)) {
		tprintf("%d, %u, ", pid, len);
	} else {
		print_affinitylist(tcp, tcp->u_arg[2], tcp->u_rval);
	}
	return 0;
}
