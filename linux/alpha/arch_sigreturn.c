long addr;

if (upeek(tcp->pid, REG_FP, &addr) < 0)
	return 0;
addr += offsetof(struct sigcontext, sc_mask);

tprints("{mask=");
print_sigset_addr_len(tcp, addr, NSIG / 8);
tprints("}");
