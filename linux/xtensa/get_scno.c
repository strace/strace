if (upeek(tcp->pid, SYSCALL_NR, &scno) < 0)
	return -1;
