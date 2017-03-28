static int
arch_set_scno(struct tcb *tcp, kernel_ulong_t scno)
{
	i386_regs.orig_eax = scno;
	return set_regs(tcp->pid);
}
