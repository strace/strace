static int
arch_set_error(struct tcb *tcp)
{
	ppc_regs.gpr[3] = tcp->u_error;
	return set_regs(tcp->pid);
}

static int
arch_set_success(struct tcb *tcp)
{
	ppc_regs.gpr[3] = tcp->u_rval;
	ppc_regs.ccr &= ~0x10000000;
	return set_regs(tcp->pid);
}
