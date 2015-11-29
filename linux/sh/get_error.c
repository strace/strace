static void
get_error(struct tcb *tcp, const bool check_errno)
{
	if (check_errno && is_negated_errno(sh_r0)) {
		tcp->u_rval = -1;
		tcp->u_error = -sh_r0;
	} else {
		tcp->u_rval = sh_r0;
	}
}
