/* Return -1 on error or 1 on success (never 0!). */
static int
get_syscall_args(struct tcb *tcp)
{
	unsigned int i;

	for (i = 0; i < tcp->s_ent->nargs; ++i)
		if (upeek(tcp->pid, PT_GR26-4*i, &tcp->u_arg[i]) < 0)
			return -1;
	return 1;
}
