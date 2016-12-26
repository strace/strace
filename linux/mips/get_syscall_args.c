/* Return -1 on error or 1 on success (never 0!). */
static int
get_syscall_args(struct tcb *tcp)
{
#if defined LINUX_MIPSN64 || defined LINUX_MIPSN32
	tcp->u_arg[0] = mips_REG_A0;
	tcp->u_arg[1] = mips_REG_A1;
	tcp->u_arg[2] = mips_REG_A2;
	tcp->u_arg[3] = mips_REG_A3;
	tcp->u_arg[4] = mips_REG_A4;
	tcp->u_arg[5] = mips_REG_A5;
#elif defined LINUX_MIPSO32
	tcp->u_arg[0] = mips_REG_A0;
	tcp->u_arg[1] = mips_REG_A1;
	tcp->u_arg[2] = mips_REG_A2;
	tcp->u_arg[3] = mips_REG_A3;
	if (tcp->s_ent->nargs > 4) {
		if (umoven(tcp, mips_REG_SP + 4 * 4,
			   (tcp->s_ent->nargs - 4) * sizeof(tcp->u_arg[0]),
			   &tcp->u_arg[4]) < 0)
			return -1;
	}
#else
# error unsupported mips abi
#endif
	return 1;
}
