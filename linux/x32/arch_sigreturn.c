if (current_personality != 1) {
	typedef struct {
		uint32_t flags, link, stack[3], pad;
		struct sigcontext mcontext;
	} ucontext_x32_header;

	const unsigned long addr =
		(unsigned long) *x86_64_rsp_ptr +
		sizeof(ucontext_x32_header);
	tprints("{mask=");
	print_sigset_addr_len(tcp, addr, NSIG / 8);
	tprints("}");
	return;
}
#include "i386/arch_sigreturn.c"
