static int
get_syscall_result_regs(struct tcb *tcp)
{
	return upeek(tcp->pid, PT_R0, &bfin_r0) < 0 ? -1 : 0;
}
