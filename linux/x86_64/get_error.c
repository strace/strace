/*
 * In X32, return value is 64-bit (llseek uses one).
 * Using merely "long rax" would not work.
 */
long long rax;

if (x86_io.iov_len == sizeof(i386_regs)) {
	/* Sign extend from 32 bits */
	rax = (int32_t) i386_regs.eax;
} else {
	rax = x86_64_regs.rax;
}

if (check_errno && is_negated_errno(rax)) {
	tcp->u_rval = -1;
	tcp->u_error = -rax;
} else {
	tcp->u_rval = rax;
# ifdef X32
	/* tcp->u_rval contains a truncated value */
	tcp->u_lrval = rax;
# endif
}
