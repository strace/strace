static void
arch_sigreturn(struct tcb *tcp)
{
	long mask[NSIG / 8 / sizeof(long)];
	const long addr = *s390_frame_ptr + __SIGNAL_FRAMESIZE;

	if (umove(tcp, addr, &mask) < 0) {
		tprintf("{mask=%#lx}", addr);
	} else {
		tprintsigmask_addr("{mask=", mask);
		tprints("}");
	}
}
