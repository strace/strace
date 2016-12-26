static int
arch_set_scno(struct tcb *tcp, kernel_ulong_t scno)
{
	m68k_regs.orig_d0 = scno;
	return set_regs(tcp->pid);
}
