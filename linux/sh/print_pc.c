long pc;
if (upeek(tcp->pid, 4*REG_PC, &pc) < 0) {
	PRINTBADPC;
	return;
}
tprintf(fmt, pc);
