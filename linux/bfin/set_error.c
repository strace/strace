static int
arch_set_error(struct tcb *tcp)
{
	bfin_r0 = -tcp->u_error;
	return upoke(tcp->pid, PT_R0, bfin_r0);
}

static int
arch_set_success(struct tcb *tcp)
{
	bfin_r0 = tcp->u_rval;
	return upoke(tcp->pid, PT_R0, bfin_r0);
}
