static int
arch_set_error(struct tcb *tcp)
{
	sparc_regs.tstate |= 0x1100000000UL;
	sparc_regs.u_regs[U_REG_O0] = tcp->u_error;
	return set_regs(tcp->pid);
}

static int
arch_set_success(struct tcb *tcp)
{
	sparc_regs.tstate &= ~0x1100000000UL;
	sparc_regs.u_regs[U_REG_O0] = tcp->u_rval;
	return set_regs(tcp->pid);
}
