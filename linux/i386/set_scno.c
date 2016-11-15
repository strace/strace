static int
arch_set_scno(struct tcb *tcp, long scno)
{
#ifdef HAVE_GETREGS_OLD
	return upoke(tcp->pid, 4 * ORIG_EAX, scno);
#else
	i386_regs.orig_eax = scno;
	return set_regs(tcp->pid);
#endif
}
