/* Return codes: 1 - ok, 0 - ignore, other - error. */
static int
arch_get_scno(struct tcb *tcp)
{
	tcp->scno = ia64_regs.gr[15];
	return 1;
}
