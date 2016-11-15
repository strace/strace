static int
arch_set_error(struct tcb *tcp)
{
	mips_REG_V0 = tcp->u_error;
	mips_REG_A3 = -1;
	return set_regs(tcp->pid);
}
