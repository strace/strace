if (check_errno && is_negated_errno(m68k_d0)) {
	tcp->u_rval = -1;
	tcp->u_error = -m68k_d0;
} else {
	tcp->u_rval = m68k_d0;
}
