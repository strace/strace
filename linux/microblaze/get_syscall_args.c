unsigned int i;

for (i = 0; i < tcp->s_ent->nargs; ++i)
	if (upeek(tcp->pid, (5 + i) * 4, &tcp->u_arg[i]) < 0)
		return -1;
