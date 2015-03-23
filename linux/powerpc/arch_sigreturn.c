long esp = ppc_regs.gpr[1];
struct sigcontext sc;

/* Skip dummy stack frame. */
#ifdef POWERPC64
if (current_personality == 0)
	esp += 128;
else
#endif
	esp += 64;

if (umove(tcp, esp, &sc) < 0) {
	tprintf("{mask=%#lx}", esp);
} else {
	unsigned long mask[NSIG / 8 / sizeof(long)];
#ifdef POWERPC64
	mask[0] = sc.oldmask | (sc._unused[3] << 32);
#else
	mask[0] = sc.oldmask;
	mask[1] = sc._unused[3];
#endif
	tprintsigmask_addr("{mask=", mask);
	tprints("}");
}
