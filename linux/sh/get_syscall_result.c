static int
get_syscall_result_regs(struct tcb *tcp)
{
	/* new syscall ABI returns result in R0 */
	return upeek(tcp->pid, 4 * REG_REG0, &sh_r0) < 0 ? -1 : 0;
}
