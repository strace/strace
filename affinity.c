#include "defs.h"

static void
print_affinitylist(struct tcb *tcp, long list, unsigned int len)
{
	int first = 1;
	unsigned long w, min_len;

	if (abbrev(tcp) && len / sizeof(w) > max_strlen)
		min_len = len - max_strlen * sizeof(w);
	else
		min_len = 0;
	for (; len >= sizeof(w) && len > min_len;
	     len -= sizeof(w), list += sizeof(w)) {
		if (umove(tcp, list, &w) < 0)
			break;
		if (first)
			tprints("{");
		else
			tprints(", ");
		first = 0;
		tprintf("%lx", w);
	}
	if (len) {
		if (first)
			tprintf("%#lx", list);
		else
			tprintf(", %s}", (len >= sizeof(w) && len > min_len ?
				"???" : "..."));
	} else {
		tprints(first ? "{}" : "}");
	}
}

SYS_FUNC(sched_setaffinity)
{
	if (entering(tcp)) {
		tprintf("%ld, %lu, ", tcp->u_arg[0], tcp->u_arg[1]);
		print_affinitylist(tcp, tcp->u_arg[2], tcp->u_arg[1]);
	}
	return 0;
}

SYS_FUNC(sched_getaffinity)
{
	if (entering(tcp)) {
		tprintf("%ld, %lu, ", tcp->u_arg[0], tcp->u_arg[1]);
	} else {
		if (tcp->u_rval == -1)
			tprintf("%#lx", tcp->u_arg[2]);
		else
			print_affinitylist(tcp, tcp->u_arg[2], tcp->u_rval);
	}
	return 0;
}
