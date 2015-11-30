/* Return -1 on error or 1 on success (never 0!). */
static int
get_syscall_args(struct tcb *tcp)
{
	tcp->u_arg[0] = ppc_regs.orig_gpr3;
	tcp->u_arg[1] = ppc_regs.gpr[4];
	tcp->u_arg[2] = ppc_regs.gpr[5];
	tcp->u_arg[3] = ppc_regs.gpr[6];
	tcp->u_arg[4] = ppc_regs.gpr[7];
	tcp->u_arg[5] = ppc_regs.gpr[8];
	return 1;
}
