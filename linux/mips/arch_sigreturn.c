static void
arch_sigreturn(struct tcb *tcp)
{
#if defined LINUX_MIPSO32
	/*
	 * offsetof(struct sigframe, sf_mask) ==
	 * sizeof(sf_ass) + sizeof(sf_pad) + sizeof(struct sigcontext)
	 */
	const kernel_ulong_t addr = mips_REG_SP + 6 * 4 +
				   sizeof(struct sigcontext);
#else
	/*
	 * This decodes rt_sigreturn.
	 * The 64-bit ABIs do not have sigreturn.
	 *
	 * offsetof(struct rt_sigframe, rs_uc) ==
	 * sizeof(sf_ass) + sizeof(sf_pad) + sizeof(struct siginfo)
	 */
	const kernel_ulong_t addr = mips_REG_SP + 6 * 4 + 128 +
				   offsetof(struct ucontext, uc_sigmask);
#endif

	tprints("{mask=");
	print_sigset_addr_len(tcp, addr, NSIG_BYTES);
	tprints("}");
}
