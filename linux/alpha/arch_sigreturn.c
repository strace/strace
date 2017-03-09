static void
arch_sigreturn(struct tcb *tcp)
{
	unsigned long addr;

	if (upeek(tcp->pid, REG_FP, &addr) < 0)
		return;
	addr += offsetof(struct sigcontext, sc_mask);

	tprints("{mask=");
	print_sigset_addr(tcp, addr);
	tprints("}");
}
