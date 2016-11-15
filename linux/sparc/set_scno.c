static int
arch_set_scno(struct tcb *tcp, long scno)
{
	sparc_regs.u_regs[U_REG_G1] = scno;
	return set_regs(tcp->pid);
}
