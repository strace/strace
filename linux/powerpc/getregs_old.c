/*
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/*
 * PTRACE_GETREGS was added to the PowerPC kernel in v2.6.23,
 * we provide a slow fallback for old kernels.
 */
static int
getregs_old(struct tcb *tcp)
{
	int i;
	long r;

	if (iflag) {
		r = upeek(tcp, sizeof(long) * PT_NIP, &ppc_regs.nip);
		if (r)
			goto out;
	}
#ifdef POWERPC64 /* else we never use it */
	r = upeek(tcp, sizeof(long) * PT_MSR, &ppc_regs.msr);
	if (r)
		goto out;
#endif
	r = upeek(tcp, sizeof(long) * PT_CCR, &ppc_regs.ccr);
	if (r)
		goto out;
	r = upeek(tcp, sizeof(long) * PT_ORIG_R3, &ppc_regs.orig_gpr3);
	if (r)
		goto out;
	for (i = 0; i <= 8; i++) {
		r = upeek(tcp, sizeof(long) * (PT_R0 + i),
			  &ppc_regs.gpr[i]);
		if (r)
			goto out;
	}
 out:
	return r;
}
