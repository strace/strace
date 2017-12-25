long
getrval2(struct tcb *tcp)
{
	unsigned long r20;
	if (upeek(tcp, 20, &r20) < 0)
		return -1;
	return r20;
}
