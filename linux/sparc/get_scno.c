/* Return codes: 1 - ok, 0 - ignore, other - error. */
static int
arch_get_scno(struct tcb *tcp)
{
	tcp->scno = sparc_regs.u_regs[U_REG_G1];
	return 1;
}
