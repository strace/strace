if (current_personality == 0) {
	const unsigned long addr =
		(unsigned long) *x86_64_rsp_ptr +
		offsetof(struct ucontext, uc_sigmask);
	tprints("{mask=");
	print_sigset_addr_len(tcp, addr, NSIG / 8);
	tprints("}");
	return;
}
#include "x32/arch_sigreturn.c"
