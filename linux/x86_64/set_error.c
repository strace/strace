#ifndef HAVE_GETREGS_OLD
# define arch_set_error i386_set_error
# define arch_set_success i386_set_success
# include "i386/set_error.c"
# undef arch_set_success
# undef arch_set_error
#endif /* !HAVE_GETREGS_OLD */

static int
arch_set_error(struct tcb *tcp)
{
#ifdef HAVE_GETREGS_OLD
	kernel_ulong_t	rval = -(kernel_long_t) tcp->u_error;

	if (x86_io.iov_len == sizeof(i386_regs))
		i386_regs.eax = rval;
	else
		x86_64_regs.rax = rval;

	return upoke(tcp, 8 * RAX, rval);
#else
	if (x86_io.iov_len == sizeof(i386_regs))
		return i386_set_error(tcp);

	x86_64_regs.rax = -(kernel_long_t) tcp->u_error;
	return set_regs(tcp->pid);
#endif
}

static int
arch_set_success(struct tcb *tcp)
{
#ifdef HAVE_GETREGS_OLD
	kernel_ulong_t  rval = (kernel_ulong_t) tcp->u_rval;

	if (x86_io.iov_len == sizeof(i386_regs))
		i386_regs.eax = rval;
	else
		x86_64_regs.rax = rval;

	return upoke(tcp, 8 * RAX, rval);
#else
	if (x86_io.iov_len == sizeof(i386_regs))
		return i386_set_success(tcp);

	x86_64_regs.rax = (kernel_ulong_t) tcp->u_rval;
	return set_regs(tcp->pid);
#endif
}
