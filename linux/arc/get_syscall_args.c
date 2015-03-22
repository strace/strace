long *arc_args = &arc_regs.scratch.r0;
unsigned int i;

for (i = 0; i < MAX_ARGS; ++i)
	tcp->u_arg[i] = *arc_args--;
