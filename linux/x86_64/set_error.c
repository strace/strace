#define arch_set_error i386_set_error
#define arch_set_success i386_set_success
#include "i386/set_error.c"
#undef arch_set_success
#undef arch_set_error

static int
arch_set_error(struct tcb *tcp)
{
	if (x86_io.iov_len == sizeof(i386_regs))
		return i386_set_error(tcp);

	x86_64_regs.rax = - (long long) tcp->u_error;
	return set_regs(tcp->pid);
}

static int
arch_set_success(struct tcb *tcp)
{
	if (x86_io.iov_len == sizeof(i386_regs))
		return i386_set_success(tcp);

	x86_64_regs.rax = (long long) tcp->u_rval;
	return set_regs(tcp->pid);
}
