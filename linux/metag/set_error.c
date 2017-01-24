static int
arch_set_error(struct tcb *tcp)
{
	metag_regs.dx[0][0] = -tcp->u_error;
	return set_regs(tcp->pid);
}

static int
arch_set_success(struct tcb *tcp)
{
	metag_regs.dx[0][0] = tcp->u_rval;
	return set_regs(tcp->pid);
}
