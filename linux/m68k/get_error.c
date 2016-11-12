static void
get_error(struct tcb *tcp, const bool check_errno)
{
	if (check_errno && is_negated_errno(m68k_regs.d0)) {
		tcp->u_rval = -1;
		tcp->u_error = -m68k_regs.d0;
	} else {
		tcp->u_rval = m68k_regs.d0;
	}
}
