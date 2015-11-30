/* Return -1 on error or 1 on success (never 0!). */
static int
get_syscall_args(struct tcb *tcp)
{
	static const int syscall_regs[MAX_ARGS] = {
		4 * (REG_REG0+4),
		4 * (REG_REG0+5),
		4 * (REG_REG0+6),
		4 * (REG_REG0+7),
		4 * (REG_REG0  ),
		4 * (REG_REG0+1)
	};
	unsigned int i;

	for (i = 0; i < tcp->s_ent->nargs; ++i)
		if (upeek(tcp->pid, syscall_regs[i], &tcp->u_arg[i]) < 0)
			return -1;
	return 1;
}
