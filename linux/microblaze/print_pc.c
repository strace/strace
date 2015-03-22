long pc;
if (upeek(tcp->pid, PT_PC, &pc) < 0) {
	PRINTBADPC;
	return;
}
tprintf(fmt, pc);
