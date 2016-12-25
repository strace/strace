/* Return -1 on error or 1 on success (never 0!). */
static int
get_syscall_args(struct tcb *tcp)
{
	unsigned long *arc_args = &arc_regs.scratch.r0;
	unsigned int i;

	for (i = 0; i < MAX_ARGS; ++i)
		tcp->u_arg[i] = *arc_args--;
	return 1;
}
