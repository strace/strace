/* Return codes: 1 - ok, 0 - ignore, other - error. */
static int
arch_get_scno(struct tcb *tcp)
{
	if (upeek(tcp->pid, REG_SYSCALL, &tcp->scno) < 0)
		return -1;
	tcp->scno &= 0xffff;
	return 1;
}
