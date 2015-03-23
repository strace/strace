long addr;

if (upeek(tcp->pid, 4*PT_USP, &addr) < 0)
	return 0;
/* Fetch pointer to struct sigcontext.  */
if (umove(tcp, addr + 2 * sizeof(int), &addr) < 0)
	return 0;

unsigned long mask[NSIG / 8 / sizeof(long)];
/* Fetch first word of signal mask.  */
if (umove(tcp, addr, &mask[0]) < 0)
	return 0;

/* Fetch remaining words of signal mask, located immediately before.  */
addr -= sizeof(mask) - sizeof(long);
if (umoven(tcp, addr, sizeof(mask) - sizeof(long), &mask[1]) < 0)
	return 0;

tprintsigmask_addr("{mask=", mask);
tprints("}");
