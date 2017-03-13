static void
arch_sigreturn(struct tcb *tcp)
{
	/* 64-bit ABIs do not have old sigreturn. */
#ifdef LINUX_MIPSO32
	/*
	 * offsetof(struct sigframe, sf_mask) ==
	 * sizeof(sf_ass) + sizeof(sf_pad) + sizeof(struct sigcontext)
	 */
	const kernel_ulong_t addr = mips_REG_SP + 6 * 4 +
				   sizeof(struct sigcontext);

	tprints("{mask=");
	print_sigset_addr(tcp, addr);
	tprints("}");
#endif
}
