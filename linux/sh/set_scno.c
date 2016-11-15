static int
arch_set_scno(struct tcb *tcp, long scno)
{
	return upoke(tcp->pid, 4 * (REG_REG0 + 3), scno);
}
