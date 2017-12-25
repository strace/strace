static int
get_syscall_result_regs(struct tcb *tcp)
{
	return upeek(tcp, 3 * 4, &microblaze_r3) < 0 ? -1 : 0;
}
