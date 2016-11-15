static int
arch_set_error(struct tcb *tcp)
{
	alpha_r0 = tcp->u_error;
	return upoke(tcp->pid, REG_R0, alpha_r0);
}
