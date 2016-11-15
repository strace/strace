static int
arch_set_scno(struct tcb *tcp, long scno)
{
	metag_regs.dx[0][1] = scno;
	return set_regs(tcp->pid);
}
