/* Return codes: 1 - ok, 0 - ignore, other - error. */
static int
arch_get_scno(struct tcb *tcp)
{
	return upeek(tcp->pid, 4 * PT_R9, &tcp->scno) < 0 ? -1 : 1;
}
