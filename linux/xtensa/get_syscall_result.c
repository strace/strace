static int
get_syscall_result_regs(struct tcb *tcp)
{
	return upeek(tcp->pid, REG_A_BASE + 2, &xtensa_a2) < 0 ? -1 : 0;
}
