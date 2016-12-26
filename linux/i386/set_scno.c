static int
arch_set_scno(struct tcb *tcp, kernel_ulong_t scno)
{
#ifdef HAVE_GETREGS_OLD
	return upoke(tcp->pid, 4 * ORIG_EAX, scno);
#else
	i386_regs.orig_eax = scno;
	return set_regs(tcp->pid);
#endif
}
