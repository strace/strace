static int
arch_set_scno(struct tcb *tcp, kernel_ulong_t scno)
{
	if (ia64_ia32mode)
		ia64_regs.gr[0] = scno;
	else
		ia64_regs.gr[15] = scno;

	return set_regs(tcp->pid);
}
