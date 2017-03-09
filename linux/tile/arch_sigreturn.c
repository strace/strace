static void
arch_sigreturn(struct tcb *tcp)
{
	/* offset of ucontext in the kernel's sigframe structure */
#define SIGFRAME_UC_OFFSET C_ABI_SAVE_AREA_SIZE + sizeof(siginfo_t)
	const unsigned long addr = tile_regs.sp + SIGFRAME_UC_OFFSET +
				   offsetof(struct ucontext, uc_sigmask);

	tprints("{mask=");
	print_sigset_addr(tcp, addr);
	tprints("}");
}
