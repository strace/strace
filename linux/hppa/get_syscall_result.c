static int
get_syscall_result_regs(struct tcb *tcp)
{
	return upeek(tcp->pid, PT_GR28, &hppa_r28) < 0 ? -1 : 0;
}
