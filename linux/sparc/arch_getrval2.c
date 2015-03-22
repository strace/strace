long
getrval2(struct tcb *tcp)
{
	return sparc_regs.u_regs[U_REG_O1];
}
