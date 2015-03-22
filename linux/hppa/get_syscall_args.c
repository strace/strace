unsigned int i;

for (i = 0; i < tcp->s_ent->nargs; ++i)
	if (upeek(tcp->pid, PT_GR26-4*i, &tcp->u_arg[i]) < 0)
		return -1;
