/*
 * Reloading the syscall number from %g1 register is supported
 * by linux kernel starting with commit v4.5-rc7~35^2~3.
 */

static int
arch_set_scno(struct tcb *tcp, kernel_ulong_t scno)
{
	sparc_regs.u_regs[U_REG_G1] = scno;
	return set_regs(tcp->pid);
}
