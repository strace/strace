if (current_personality != 1) {
	const unsigned long addr =
		(unsigned long) *x86_64_rsp_ptr +
		offsetof(struct ucontext, uc_sigmask);
	tprints("{mask=");
	print_sigset_addr_len(tcp, addr, NSIG / 8);
	tprints("}");
	return 0;
}
#include "i386/arch_sigreturn.c"
