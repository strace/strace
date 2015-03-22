long pc;
if (upeek(tcp->pid, PT_IAOQ0, &pc) < 0) {
	PRINTBADPC;
	return;
}
tprintf(fmt, pc);
