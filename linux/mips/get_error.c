if (check_errno && mips_REG_A3) {
	tcp->u_rval = -1;
	tcp->u_error = mips_REG_V0;
} else {
# if defined LINUX_MIPSN32
	tcp->u_lrval = mips_REG_V0;
# endif
	tcp->u_rval = mips_REG_V0;
}
