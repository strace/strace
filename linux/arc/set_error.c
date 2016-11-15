static int
arch_set_error(struct tcb *tcp)
{
	arc_regs.scratch.r0 = -tcp->u_error;
	return set_regs(tcp->pid);
}
