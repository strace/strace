static int
arch_set_error(struct tcb *tcp)
{
	ppc_regs.gpr[3] = tcp->u_error;
#ifdef HAVE_GETREGS_OLD
	return upoke(tcp->pid, sizeof(long) * (PT_R0 + 3), ppc_regs.gpr[3]);
#else
	return set_regs(tcp->pid);
#endif
}

static int
arch_set_success(struct tcb *tcp)
{
	ppc_regs.gpr[3] = tcp->u_rval;
	ppc_regs.ccr &= ~0x10000000;
#ifdef HAVE_GETREGS_OLD
	return upoke(tcp->pid, sizeof(long) * PT_CCR, ppc_regs.ccr) ||
	       upoke(tcp->pid, sizeof(long) * (PT_R0 + 3), ppc_regs.gpr[3]);
#else
	return set_regs(tcp->pid);
#endif
}
