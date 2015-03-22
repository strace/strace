static const int crisregs[MAX_ARGS] = {
	4*PT_ORIG_R10, 4*PT_R11, 4*PT_R12,
	4*PT_R13     , 4*PT_MOF, 4*PT_SRP
};
unsigned int i;

for (i = 0; i < tcp->s_ent->nargs; ++i)
	if (upeek(tcp->pid, crisregs[i], &tcp->u_arg[i]) < 0)
		return -1;
