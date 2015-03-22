static const int argreg[MAX_ARGS] = { PT_R0, PT_R1, PT_R2, PT_R3, PT_R4, PT_R5 };
unsigned int i;

for (i = 0; i < tcp->s_ent->nargs; ++i)
	if (upeek(tcp->pid, argreg[i], &tcp->u_arg[i]) < 0)
		return -1;
