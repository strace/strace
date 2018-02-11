static int
arch_set_scno(struct tcb *tcp, kernel_ulong_t scno)
{
	ia64_regs.gr[15] = scno;

	return set_regs(tcp->pid);
}
