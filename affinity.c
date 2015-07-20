#include "defs.h"

static void
print_affinitylist(struct tcb *tcp, const unsigned long addr, const unsigned int len)
{
	unsigned long w;
	const unsigned int size = len * sizeof(w);
	const unsigned long end = addr + size;
	unsigned long cur, abbrev_end;

	if (!verbose(tcp) || (exiting(tcp) && syserror(tcp)) ||
	    !addr || !len || size / sizeof(w) != len || end < addr) {
		printaddr(addr);
		return;
	}

	if (abbrev(tcp)) {
		abbrev_end = addr + max_strlen *  sizeof(w);
		if (abbrev_end < addr)
			abbrev_end = end;
	} else {
		abbrev_end = end;
	}

	tprints("[");
	for (cur = addr; cur < end; cur += sizeof(w)) {
		if (cur > addr)
			tprints(", ");
		if (cur >= abbrev_end) {
			tprints("...");
			break;
		}
		if (umove_or_printaddr(tcp, cur, &w))
			break;
		tprintf("%lx", w);
	}
	tprints("]");
}

SYS_FUNC(sched_setaffinity)
{
	tprintf("%ld, %lu, ", tcp->u_arg[0], tcp->u_arg[1]);
	print_affinitylist(tcp, tcp->u_arg[2], tcp->u_arg[1]);

	return RVAL_DECODED;
}

SYS_FUNC(sched_getaffinity)
{
	if (entering(tcp)) {
		tprintf("%ld, %lu, ", tcp->u_arg[0], tcp->u_arg[1]);
	} else {
		print_affinitylist(tcp, tcp->u_arg[2], tcp->u_rval);
	}
	return 0;
}
