static void
arch_sigreturn(struct tcb *tcp)
{
#define SIZEOF_STRUCT_SIGINFO 128
#define SIZEOF_STRUCT_SIGCONTEXT (21 * 4)
#define OFFSETOF_STRUCT_UCONTEXT_UC_SIGMASK (5 * 4 + SIZEOF_STRUCT_SIGCONTEXT)

	const unsigned long addr =
#ifdef AARCH64
		tcp->currpers == 0 ?
			(*aarch64_sp_ptr + SIZEOF_STRUCT_SIGINFO +
			 offsetof(struct ucontext, uc_sigmask)) :
#endif
			(*arm_sp_ptr +
			 OFFSETOF_STRUCT_UCONTEXT_UC_SIGMASK);
	tprints("{mask=");
	print_sigset_addr(tcp, addr);
	tprints("}");
}
