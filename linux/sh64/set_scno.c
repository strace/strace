static int
arch_set_scno(struct tcb *tcp, kernel_ulong_t scno)
{
	return upoke(tcp, REG_SYSCALL, scno);
}
