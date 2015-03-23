/* TODO: Verify that this is correct...  */

long addr;

/* Read r1, the stack pointer.  */
if (upeek(tcp->pid, 1 * 4, &addr) < 0)
	return 0;
addr += offsetof(struct sigcontext, oldmask);

tprints("{mask=");
print_sigset_addr_len(tcp, addr, NSIG / 8);
tprints("}");
