static void
get_error(struct tcb *tcp, const bool check_errno)
{
	if (check_errno && is_negated_errno(riscv_regs.a0)) {
		tcp->u_rval = -1;
		tcp->u_error = -riscv_regs.a0;
	} else {
		tcp->u_rval = riscv_regs.a0;
	}
}
