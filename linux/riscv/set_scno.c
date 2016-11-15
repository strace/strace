static int
arch_set_scno(struct tcb *tcp, long scno)
{
	riscv_regs.a7 = scno;
	return set_regs(tcp->pid);
}
