static void
arch_sigreturn(struct tcb *tcp)
{
	unsigned long regs[PT_MAX + 1];

	if (ptrace(PTRACE_GETREGS, tcp->pid, NULL, regs) < 0) {
		perror_msg("sigreturn: PTRACE_GETREGS");
		return;
	}
	const unsigned long addr =
		regs[PT_USP] + offsetof(struct sigcontext, oldmask);

	tprints("{mask=");
	print_sigset_addr(tcp, addr);
	tprints("}");
}
