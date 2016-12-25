/*
 * PTRACE_GETREGS was added to the PowerPC kernel in v2.6.23,
 * we provide a slow fallback for old kernels.
 */
static int
getregs_old(pid_t pid)
{
	int i;
	long r;

	if (iflag) {
		r = upeek(pid, sizeof(long) * PT_NIP, &ppc_regs.nip);
		if (r)
			goto out;
	}
#ifdef POWERPC64 /* else we never use it */
	r = upeek(pid, sizeof(long) * PT_MSR, &ppc_regs.msr);
	if (r)
		goto out;
#endif
	r = upeek(pid, sizeof(long) * PT_CCR, &ppc_regs.ccr);
	if (r)
		goto out;
	r = upeek(pid, sizeof(long) * PT_ORIG_R3, &ppc_regs.orig_gpr3);
	if (r)
		goto out;
	for (i = 0; i <= 8; i++) {
		r = upeek(pid, sizeof(long) * (PT_R0 + i),
			  &ppc_regs.gpr[i]);
		if (r)
			goto out;
	}
 out:
	return r;
}
