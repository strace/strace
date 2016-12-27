static void
arch_sigreturn(struct tcb *tcp)
{
	unsigned long addr = ppc_regs.gpr[1];
	struct sigcontext sc;

	/* Skip dummy stack frame. */
#ifdef POWERPC64
	if (current_personality == 0)
		addr += 128;
	else
#endif
		addr += 64;

	if (umove(tcp, addr, &sc) < 0) {
		tprintf("{mask=%#lx}", addr);
	} else {
		unsigned long mask[NSIG_BYTES / sizeof(long)];
#ifdef POWERPC64
		mask[0] = sc.oldmask | (sc._unused[3] << 32);
#else
		mask[0] = sc.oldmask;
		mask[1] = sc._unused[3];
#endif
		tprintsigmask_addr("{mask=", mask);
		tprints("}");
	}
}
