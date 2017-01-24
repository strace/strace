static int
arch_set_error(struct tcb *tcp)
{
	sh_r0 = -tcp->u_error;
	return upoke(tcp->pid, 4 * REG_REG0, sh_r0);
}

static int
arch_set_success(struct tcb *tcp)
{
	sh_r0 = tcp->u_rval;
	return upoke(tcp->pid, 4 * REG_REG0, sh_r0);
}
