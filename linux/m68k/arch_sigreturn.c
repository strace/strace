static void
arch_sigreturn(struct tcb *tcp)
{
	unsigned long addr;

	/* Fetch pointer to struct sigcontext.  */
	if (umove(tcp, *m68k_usp_ptr + 2 * sizeof(int), &addr) < 0)
		return;

	unsigned long mask[NSIG_BYTES / sizeof(long)];
	/* Fetch first word of signal mask.  */
	if (umove(tcp, addr, &mask[0]) < 0)
		return;

	/* Fetch remaining words of signal mask, located immediately before.  */
	addr -= sizeof(mask) - sizeof(long);
	if (umoven(tcp, addr, sizeof(mask) - sizeof(long), &mask[1]) < 0)
		return;

	tprintsigmask_addr("{mask=", mask);
	tprints("}");
}
