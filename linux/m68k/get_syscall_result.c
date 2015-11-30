static int
get_syscall_result_regs(struct tcb *tcp)
{
	return upeek(tcp->pid, 4 * PT_D0, &m68k_d0) < 0 ? -1 : 0;
}
