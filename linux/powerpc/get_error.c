if (check_errno && (ppc_regs.ccr & 0x10000000)) {
	tcp->u_rval = -1;
	tcp->u_error = ppc_regs.gpr[3];
} else {
	tcp->u_rval = ppc_regs.gpr[3];
}
