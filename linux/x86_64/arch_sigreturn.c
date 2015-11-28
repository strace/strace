#define	arch_sigreturn	i386_arch_sigreturn
#include "i386/arch_sigreturn.c"
#undef arch_sigreturn

static void
arch_sigreturn(struct tcb *tcp)
{
	if (current_personality == 1) {
		i386_arch_sigreturn(tcp);
		return;
	}

	typedef struct {
		uint32_t flags, link, stack[3], pad;
		struct sigcontext mcontext;
	} ucontext_x32_header;

#define	X86_64_SIGMASK_OFFSET	offsetof(struct ucontext, uc_sigmask)
#define	X32_SIGMASK_OFFSET	sizeof(ucontext_x32_header)

	const unsigned long offset =
#ifdef X32
		X32_SIGMASK_OFFSET;
#else
		current_personality == 2 ? X32_SIGMASK_OFFSET :
					   X86_64_SIGMASK_OFFSET;
#endif
	const unsigned long addr = (unsigned long) *x86_64_rsp_ptr + offset;
	tprints("{mask=");
	print_sigset_addr_len(tcp, addr, NSIG / 8);
	tprints("}");
}
