if (check_errno && is_negated_errno(avr32_regs.r12)) {
	tcp->u_rval = -1;
	tcp->u_error = -avr32_regs.r12;
} else {
	tcp->u_rval = avr32_regs.r12;
}
