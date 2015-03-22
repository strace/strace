long pc;
if (upeek(tcp->pid, 4*PT_PC, &pc) < 0) {
	PRINTBADPC;
	return;
}
tprintf(fmt, pc);
