static int
arch_set_scno(struct tcb *tcp, long scno)
{
	return upoke(tcp->pid, PT_GR20, scno);
}
