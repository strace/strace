static void
get_error(struct tcb *tcp, const bool check_errno)
{
	if (mips_REG_A3) {
		tcp->u_rval = -1;
		tcp->u_error = mips_REG_V0;
	} else {
		tcp->u_rval = mips_REG_V0;
	}
}
