if (check_errno && is_negated_errno(or1k_regs.gpr[11])) {
	tcp->u_rval = -1;
	tcp->u_error = -or1k_regs.gpr[11];
} else {
	tcp->u_rval = or1k_regs.gpr[11];
}
