static int
arch_set_error(struct tcb *tcp)
{
	microblaze_r3 = -tcp->u_error;
	return upoke(tcp, 3 * 4, microblaze_r3);
}

static int
arch_set_success(struct tcb *tcp)
{
	microblaze_r3 = tcp->u_rval;
	return upoke(tcp, 3 * 4, microblaze_r3);
}
