static int
arch_set_scno(struct tcb *tcp, kernel_ulong_t scno)
{
	nios2_regs.regs[2] = scno;
	return set_regs(tcp->pid);
}
