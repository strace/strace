static int
arch_set_scno(struct tcb *tcp, kernel_ulong_t scno)
{
	ppc_regs.gpr[0] = scno;
	return set_regs(tcp->pid);
}
