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
	const unsigned wordsize = current_wordsize;

	cp.p64 = 1;
	for (sep = ""; !abbrev(tcp) || n < max_strlen / 2; sep = ", ", ++n) {
		if (umoven_or_printaddr(tcp, addr, wordsize, cp.data))
			return;
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
	char *cp = NULL;

	for (count = 0; !umoven(tcp, addr, current_wordsize, &cp) && cp; count++) {
		addr += current_wordsize;
	}
	tprintf(fmt, count, count == 1 ? "" : "s");
}

static void
decode_execve(struct tcb *tcp, const unsigned int index)
{
	printpath(tcp, tcp->u_arg[index + 0]);
	tprints(", ");

	if (!tcp->u_arg[index + 1] || !verbose(tcp))
		printaddr(tcp->u_arg[index + 1]);
	else {
		tprints("[");
		printargv(tcp, tcp->u_arg[index + 1]);
		tprints("]");
	}
	tprints(", ");

	if (!tcp->u_arg[index + 2] || !verbose(tcp))
		printaddr(tcp->u_arg[index + 2]);
	else if (abbrev(tcp))
		printargc("[/* %d var%s */]", tcp, tcp->u_arg[index + 2]);
	else {
		tprints("[");
		printargv(tcp, tcp->u_arg[index + 2]);
		tprints("]");
	}
}

SYS_FUNC(execve)
{
	decode_execve(tcp, 0);

	return RVAL_DECODED;
}

SYS_FUNC(execveat)
{
	print_dirfd(tcp, tcp->u_arg[0]);
	decode_execve(tcp, 1);
	tprints(", ");
	printflags(at_flags, tcp->u_arg[4], "AT_???");

	return RVAL_DECODED;
}

#if defined(SPARC) || defined(SPARC64)
SYS_FUNC(execv)
{
	printpath(tcp, tcp->u_arg[0]);
	tprints(", ");
	if (!tcp->u_arg[1] || !verbose(tcp))
		printaddr(tcp->u_arg[1]);
	else {
		tprints("[");
		printargv(tcp, tcp->u_arg[1]);
		tprints("]");
	}

	return RVAL_DECODED;
}
#endif /* SPARC || SPARC64 */
