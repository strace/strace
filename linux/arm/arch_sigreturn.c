#define SIZEOF_STRUCT_SIGINFO 128
#define SIZEOF_STRUCT_SIGCONTEXT (21 * 4)
#define OFFSETOF_STRUCT_UCONTEXT_UC_SIGMASK (5 * 4 + SIZEOF_STRUCT_SIGCONTEXT)

const long addr =
#ifdef AARCH64
	current_personality == 1 ?
		(*aarch64_sp_ptr + SIZEOF_STRUCT_SIGINFO +
		 offsetof(struct ucontext, uc_sigmask)) :
#endif
		(*arm_sp_ptr +
		 OFFSETOF_STRUCT_UCONTEXT_UC_SIGMASK);
tprints("{mask=");
print_sigset_addr_len(tcp, addr, NSIG / 8);
tprints("}");
