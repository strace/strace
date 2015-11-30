static int
get_syscall_result_regs(struct tcb *tcp)
{
	return upeek(tcp->pid, 4 * PT_R10, &cris_r10) < 0 ? -1 : 0;
}
