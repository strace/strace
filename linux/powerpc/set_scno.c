static int
arch_set_scno(struct tcb *tcp, long scno)
{
#ifdef HAVE_GETREGS_OLD
	return upoke(tcp->pid, sizeof(long) * PT_R0, scno);
#else
	ppc_regs.gpr[0] = scno;
	return set_regs(tcp->pid);
#endif
}
