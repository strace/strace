static int
arch_set_scno(struct tcb *tcp, long scno)
{
	arc_regs.scratch.r8 = scno;
	return set_regs(tcp->pid);
}
