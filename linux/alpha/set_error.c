static int
arch_set_error(struct tcb *tcp)
{
	alpha_r0 = tcp->u_error;
	return upoke(tcp->pid, REG_R0, alpha_r0);
}

static int
arch_set_success(struct tcb *tcp)
{
	return upoke(tcp->pid, REG_A3, (alpha_a3 = 0))
	       || upoke(tcp->pid, REG_R0, (alpha_r0 = tcp->u_rval));
}
