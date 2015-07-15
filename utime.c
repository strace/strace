#include "defs.h"

SYS_FUNC(utime)
{
	union {
		long utl[2];
		int uti[2];
		long paranoia_for_huge_wordsize[4];
	} u;
	unsigned wordsize;

	printpath(tcp, tcp->u_arg[0]);
	tprints(", ");

	wordsize = current_wordsize;
	if (umoven_or_printaddr(tcp, tcp->u_arg[1], 2 * wordsize, &u))
		;
	else if (wordsize == sizeof u.utl[0]) {
		tprintf("[%s,", sprinttime(u.utl[0]));
		tprintf(" %s]", sprinttime(u.utl[1]));
	}
	else if (wordsize == sizeof u.uti[0]) {
		tprintf("[%s,", sprinttime(u.uti[0]));
		tprintf(" %s]", sprinttime(u.uti[1]));
	}
	else
		tprintf("<decode error: unsupported wordsize %d>",
			wordsize);

	return RVAL_DECODED;
}
