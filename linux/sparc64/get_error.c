if (check_errno && sparc_regs.tstate & 0x1100000000UL) {
	tcp->u_rval = -1;
	tcp->u_error = sparc_regs.u_regs[U_REG_O0];
} else {
	tcp->u_rval = sparc_regs.u_regs[U_REG_O0];
}
