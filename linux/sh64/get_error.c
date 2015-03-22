if (check_errno && is_negated_errno(sh64_r9)) {
	tcp->u_rval = -1;
	tcp->u_error = -sh64_r9;
} else {
	tcp->u_rval = sh64_r9;
}
