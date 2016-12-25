/* Return -1 on error or 1 on success (never 0!). */
static int
get_syscall_args(struct tcb *tcp)
{
	if (!ia64_ia32mode) {
		unsigned long *rbs_end =
			(unsigned long *) ia64_regs.ar[PT_AUR_BSP];
		unsigned long sof = (ia64_regs.cfm >> 0) & 0x7f;
		unsigned long sol = (ia64_regs.cfm >> 7) & 0x7f;
		unsigned long *out0 = ia64_rse_skip_regs(rbs_end, -sof + sol);
		unsigned int i;

		for (i = 0; i < tcp->s_ent->nargs; ++i) {
			if (umove(tcp,
				  (unsigned long) ia64_rse_skip_regs(out0, i),
				  &tcp->u_arg[i]) < 0)
				return -1;
		}
	} else {
		/* truncate away IVE sign-extension */
		tcp->u_arg[0] = 0xffffffff & ia64_regs.gr[11]; /* EBX */
		tcp->u_arg[1] = 0xffffffff & ia64_regs.gr[ 9]; /* ECX */
		tcp->u_arg[2] = 0xffffffff & ia64_regs.gr[10]; /* EDX */
		tcp->u_arg[3] = 0xffffffff & ia64_regs.gr[14]; /* ESI */
		tcp->u_arg[4] = 0xffffffff & ia64_regs.gr[15]; /* EDI */
		tcp->u_arg[5] = 0xffffffff & ia64_regs.gr[13]; /* EBP */
	}
	return 1;
}
