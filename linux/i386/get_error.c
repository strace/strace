static void
get_error(struct tcb *tcp, const bool check_errno)
{
	if (check_errno && is_negated_errno(i386_regs.eax)) {
		tcp->u_rval = -1;
		tcp->u_error = -i386_regs.eax;
	} else {
		tcp->u_rval = i386_regs.eax;
	}
}
