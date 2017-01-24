static int
arch_set_error(struct tcb *tcp)
{
	xtensa_a2 = -tcp->u_error;
	return upoke(tcp->pid, REG_A_BASE + 2, xtensa_a2);
}

static int
arch_set_success(struct tcb *tcp)
{
	xtensa_a2 = tcp->u_rval;
	return upoke(tcp->pid, REG_A_BASE + 2, xtensa_a2);
}
