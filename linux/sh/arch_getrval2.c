long
getrval2(struct tcb *tcp)
{
	unsigned long val;
	if (upeek(tcp->pid, 4*(REG_REG0+1), &val) < 0)
		return -1;
	return val;
}
