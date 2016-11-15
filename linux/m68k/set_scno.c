static int
arch_set_scno(struct tcb *tcp, long scno)
{
	m68k_regs.orig_d0 = scno;
	return set_regs(tcp->pid);
}
