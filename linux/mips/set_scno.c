static int
arch_set_scno(struct tcb *tcp, long scno)
{
	mips_REG_V0 = scno;
	return set_regs(tcp->pid);
}
