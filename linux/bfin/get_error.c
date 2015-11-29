static void
get_error(struct tcb *tcp, const bool check_errno)
{
	if (check_errno && is_negated_errno(bfin_r0)) {
		tcp->u_rval = -1;
		tcp->u_error = -bfin_r0;
	} else {
		tcp->u_rval = bfin_r0;
	}
}
