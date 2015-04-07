#include "defs.h"

SYS_FUNC(utime)
{
	union {
		long utl[2];
		int uti[2];
		long paranoia_for_huge_wordsize[4];
	} u;
	unsigned wordsize;

	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprints(", ");

		wordsize = current_wordsize;
		if (!tcp->u_arg[1])
			tprints("NULL");
		else if (!verbose(tcp))
			tprintf("%#lx", tcp->u_arg[1]);
		else if (umoven(tcp, tcp->u_arg[1], 2 * wordsize, &u) < 0)
			tprints("[?, ?]");
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
	}
	return 0;
}
