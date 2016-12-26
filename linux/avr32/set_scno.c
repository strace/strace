static int
arch_set_scno(struct tcb *tcp, kernel_ulong_t scno)
{
	avr32_regs.r8 = scno;
	return set_regs(tcp->pid);
}
