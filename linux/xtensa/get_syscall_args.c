/* arg0: a6, arg1: a3, arg2: a4, arg3: a5, arg4: a8, arg5: a9 */
static const int xtensaregs[MAX_ARGS] = { 6, 3, 4, 5, 8, 9 };
unsigned int i;

for (i = 0; i < tcp->s_ent->nargs; ++i)
	if (upeek(tcp->pid, REG_A_BASE + xtensaregs[i], &tcp->u_arg[i]) < 0)
		return -1;
