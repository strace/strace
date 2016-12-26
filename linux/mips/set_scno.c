static int
arch_set_scno(struct tcb *tcp, kernel_ulong_t scno)
{
	mips_REG_V0 = scno;
	return set_regs(tcp->pid);
}
