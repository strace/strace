static void
get_error(struct tcb *tcp, const bool check_errno)
{
	if (check_errno && is_negated_errno(arm_regs.ARM_r0)) {
		tcp->u_rval = -1;
		tcp->u_error = -arm_regs.ARM_r0;
	} else {
		tcp->u_rval = arm_regs.ARM_r0;
	}
}
