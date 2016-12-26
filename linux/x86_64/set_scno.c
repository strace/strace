#ifndef HAVE_GETREGS_OLD
# define arch_set_scno i386_set_scno
# include "i386/set_scno.c"
# undef arch_set_scno
#endif /* !HAVE_GETREGS_OLD */

static int
arch_set_scno(struct tcb *tcp, kernel_ulong_t scno)
{
#ifdef HAVE_GETREGS_OLD
	return upoke(tcp->pid, 8 * ORIG_RAX, scno);
#else
	if (x86_io.iov_len == sizeof(i386_regs))
		return i386_set_scno(tcp, scno);

	x86_64_regs.orig_rax = scno;
	return set_regs(tcp->pid);
#endif
}
