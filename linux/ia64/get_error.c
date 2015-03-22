if (ia64_ia32mode) {
	int err = ia64_regs.gr[8];
	if (check_errno && is_negated_errno(err)) {
		tcp->u_rval = -1;
		tcp->u_error = -err;
	} else {
		tcp->u_rval = err;
	}
} else {
	if (check_errno && ia64_regs.gr[10]) {
		tcp->u_rval = -1;
		tcp->u_error = ia64_regs.gr[8];
	} else {
		tcp->u_rval = ia64_regs.gr[8];
	}
}
