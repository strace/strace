/* Return -1 on error or 1 on success (never 0!). */
static int
get_syscall_args(struct tcb *tcp)
{
	tcp->u_arg[0] = nios2_regs.regs[4];
	tcp->u_arg[1] = nios2_regs.regs[5];
	tcp->u_arg[2] = nios2_regs.regs[6];
	tcp->u_arg[3] = nios2_regs.regs[7];
	tcp->u_arg[4] = nios2_regs.regs[8];
	tcp->u_arg[5] = nios2_regs.regs[9];
	return 1;
}
