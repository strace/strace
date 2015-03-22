long pc;
if (upeek(tcp->pid, REG_PC, &pc) < 0) {
	PRINTBADPC;
	return;
}
tprintf(fmt, pc);
