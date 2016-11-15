static int
arch_set_scno(struct tcb *tcp, long scno)
{
	return upoke(tcp->pid, REG_SYSCALL, scno);
}
