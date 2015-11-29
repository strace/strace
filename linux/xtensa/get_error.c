static void
get_error(struct tcb *tcp, const bool check_errno)
{
	if (check_errno && is_negated_errno(xtensa_a2)) {
		tcp->u_rval = -1;
		tcp->u_error = -xtensa_a2;
	} else {
		tcp->u_rval = xtensa_a2;
	}
}
