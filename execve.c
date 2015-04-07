#include "defs.h"

static void
printargv(struct tcb *tcp, long addr)
{
	union {
		unsigned int p32;
		unsigned long p64;
		char data[sizeof(long)];
	} cp;
	const char *sep;
	unsigned int n = 0;
	unsigned wordsize = current_wordsize;

	cp.p64 = 1;
	for (sep = ""; !abbrev(tcp) || n < max_strlen / 2; sep = ", ", ++n) {
		if (umoven(tcp, addr, wordsize, cp.data) < 0) {
			tprintf("%#lx", addr);
			return;
		}
		if (wordsize == 4)
			cp.p64 = cp.p32;
		if (cp.p64 == 0)
			break;
		tprints(sep);
		printstr(tcp, cp.p64, -1);
		addr += wordsize;
	}
	if (cp.p64)
		tprintf("%s...", sep);
}

static void
printargc(const char *fmt, struct tcb *tcp, long addr)
{
	int count;
	char *cp;

	for (count = 0; umove(tcp, addr, &cp) >= 0 && cp != NULL; count++) {
		addr += sizeof(char *);
	}
	tprintf(fmt, count, count == 1 ? "" : "s");
}

SYS_FUNC(execve)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		if (!verbose(tcp))
			tprintf(", %#lx", tcp->u_arg[1]);
		else {
			tprints(", [");
			printargv(tcp, tcp->u_arg[1]);
			tprints("]");
		}
		if (!verbose(tcp))
			tprintf(", %#lx", tcp->u_arg[2]);
		else if (abbrev(tcp))
			printargc(", [/* %d var%s */]", tcp, tcp->u_arg[2]);
		else {
			tprints(", [");
			printargv(tcp, tcp->u_arg[2]);
			tprints("]");
		}
	}
	return 0;
}

#if defined(SPARC) || defined(SPARC64)
SYS_FUNC(execv)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		if (!verbose(tcp))
			tprintf(", %#lx", tcp->u_arg[1]);
		else {
			tprints(", [");
			printargv(tcp, tcp->u_arg[1]);
			tprints("]");
		}
	}
	return 0;
}
#endif /* SPARC || SPARC64 */
