static int
arch_set_error(struct tcb *tcp)
{
	i386_regs.eax = -tcp->u_error;
#ifdef HAVE_GETREGS_OLD
	return upoke(tcp->pid, 4 * EAX, i386_regs.eax);
#else
	return set_regs(tcp->pid);
#endif
}
