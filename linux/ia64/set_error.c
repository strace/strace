static int
arch_set_error(struct tcb *tcp)
{
	if (ia64_ia32mode) {
		ia64_regs.gr[8] = -tcp->u_error;
	} else {
		ia64_regs.gr[8] = tcp->u_error;
		ia64_regs.gr[10] = -1;
	}
	return set_regs(tcp->pid);
}

static int
arch_set_success(struct tcb *tcp)
{
	ia64_regs.gr[8] = tcp->u_rval;
	if (!ia64_ia32mode) {
		ia64_regs.gr[10] = 0;
	}
	return set_regs(tcp->pid);
}
