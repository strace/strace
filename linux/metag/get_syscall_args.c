unsigned int i;

for (i = 0; i < MAX_ARGS; i++)
	/* arguments go backwards from D1Ar1 (D1.3) */
	tcp->u_arg[i] = ((unsigned long *)&metag_regs.dx[3][1])[-i];
