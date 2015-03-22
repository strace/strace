long
getrval2(struct tcb *tcp)
{
	long r20;
	if (upeek(tcp->pid, 20, &r20) < 0)
		return -1;
	return r20;
}
