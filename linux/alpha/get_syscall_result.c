if (upeek(tcp->pid, REG_A3, &alpha_a3) < 0)
	return -1;
if (upeek(tcp->pid, REG_R0, &alpha_r0) < 0)
	return -1;
