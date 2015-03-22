/* new syscall ABI returns result in R0 */
if (upeek(tcp->pid, 4*REG_REG0, (long *)&sh_r0) < 0)
	return -1;
