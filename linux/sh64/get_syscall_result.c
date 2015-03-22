/* ABI defines result returned in r9 */
if (upeek(tcp->pid, REG_GENERAL(9), (long *)&sh64_r9) < 0)
	return -1;
