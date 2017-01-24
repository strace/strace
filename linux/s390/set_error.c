static int
arch_set_error(struct tcb *tcp)
{
	s390_regset.gprs[2] = -tcp->u_error;
	return set_regs(tcp->pid);
}

static int
arch_set_success(struct tcb *tcp)
{
	s390_regset.gprs[2] = tcp->u_rval;
	return set_regs(tcp->pid);
}
