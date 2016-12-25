/* Return -1 on error or 1 on success (never 0!). */
static int
get_syscall_args(struct tcb *tcp)
{
	unsigned int i;

	for (i = 0; i < MAX_ARGS; i++) {
		/* arguments go backwards from D1Ar1 (D1.3) */
		tcp->u_arg[i] = (&metag_regs.dx[3][1])[-i];
	}
	return 1;
}
