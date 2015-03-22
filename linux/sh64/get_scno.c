if (upeek(tcp->pid, REG_SYSCALL, &scno) < 0)
	return -1;
scno &= 0xFFFF;
