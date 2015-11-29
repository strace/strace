static void
get_error(struct tcb *tcp, const bool check_errno)
{
	if (check_errno && is_negated_errno(s390_regset.gprs[2])) {
		tcp->u_rval = -1;
		tcp->u_error = -s390_regset.gprs[2];
	} else {
		tcp->u_rval = s390_regset.gprs[2];
	}
}
