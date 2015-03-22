unsigned int i;

for (i = 0; i < tcp->s_ent->nargs; ++i)
	if (upeek(tcp->pid, (i < 5 ? i : i + 2)*4, &tcp->u_arg[i]) < 0)
		return -1;
