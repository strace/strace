static void
arch_sigreturn(struct tcb *tcp)
{
	/* Skip dummy stack frame. */
	const unsigned long addr = ppc_regs.gpr[1] + 64;

#ifdef POWERPC64
	/* The only sigreturn on ppc64 is compat_sys_sigreturn. */
	typedef struct {
		unsigned int _unused[4];
		int signal;
		unsigned int handler;
		unsigned int oldmask;
		/* all the rest is irrelevant */
	} sigreturn_context;
#else
	typedef struct sigcontext sigreturn_context;
#endif

	sigreturn_context sc;

	if (umove(tcp, addr, &sc) < 0) {
		tprintf("{mask=%#lx}", addr);
	} else {
		const unsigned int mask[NSIG_BYTES / sizeof(int)] = {
			sc.oldmask,
			sc._unused[3]
		};

		tprintsigmask_addr("{mask=", mask);
		tprints("}");
	}
}
