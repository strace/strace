static int
arch_set_error(struct tcb *tcp)
{
	sh64_r9 = -tcp->u_error;
	return upoke(tcp->pid, REG_GENERAL(9), sh64_r9);
}

static int
arch_set_success(struct tcb *tcp)
{
	sh64_r9 = tcp->u_rval;
	return upoke(tcp->pid, REG_GENERAL(9), sh64_r9);
}
