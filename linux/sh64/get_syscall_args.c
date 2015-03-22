/* Registers used by SH5 Linux system calls for parameters */
static const int syscall_regs[MAX_ARGS] = { 2, 3, 4, 5, 6, 7 };
unsigned int i;

for (i = 0; i < tcp->s_ent->nargs; ++i)
	if (upeek(tcp->pid, REG_GENERAL(syscall_regs[i]), &tcp->u_arg[i]) < 0)
		return -1;
