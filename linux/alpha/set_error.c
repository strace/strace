static int
arch_set_error(struct tcb *tcp)
{
	return upoke(tcp, REG_A3, (alpha_a3 = 1))
	       || upoke(tcp, REG_R0, (alpha_r0 = tcp->u_error));
}

static int
arch_set_success(struct tcb *tcp)
{
	return upoke(tcp, REG_A3, (alpha_a3 = 0))
	       || upoke(tcp, REG_R0, (alpha_r0 = tcp->u_rval));
}
