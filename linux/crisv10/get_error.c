static void
get_error(struct tcb *tcp, const bool check_errno)
{
	if (check_errno && is_negated_errno(cris_r10)) {
		tcp->u_rval = -1;
		tcp->u_error = -cris_r10;
	} else {
		tcp->u_rval = cris_r10;
	}
}
