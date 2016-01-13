/* Return codes: 1 - ok, 0 - ignore, other - error. */
static int
arch_get_scno(struct tcb *tcp)
{
	tcp->scno = s390_regset.gprs[2] ?
		    s390_regset.gprs[2] : s390_regset.gprs[1];
	return 1;
}
