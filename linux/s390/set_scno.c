static int
arch_set_scno(struct tcb *tcp, long scno)
{
	s390_regset.gprs[2] = scno;
	return set_regs(tcp->pid);
}
