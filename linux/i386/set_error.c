static int
arch_set_error(struct tcb *tcp)
{
	i386_regs.eax = -tcp->u_error;
	return upoke(tcp, 4 * EAX, i386_regs.eax);
}

static int
arch_set_success(struct tcb *tcp)
{
	i386_regs.eax = tcp->u_rval;
	return upoke(tcp, 4 * EAX, i386_regs.eax);
}
