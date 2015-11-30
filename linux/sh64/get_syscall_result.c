static int
get_syscall_result_regs(struct tcb *tcp)
{
	/* ABI defines result returned in r9 */
	return upeek(tcp->pid, REG_GENERAL(9), (long *)&sh64_r9) < 0 ? -1 : 0;
}
