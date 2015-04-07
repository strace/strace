#include "defs.h"

SYS_FUNC(exit)
{
	if (exiting(tcp)) {
		fprintf(stderr, "_exit returned!\n");
		return -1;
	}
	/* special case: we stop tracing this process, finish line now */
	tprintf("%ld) ", tcp->u_arg[0]);
	tabto();
	tprints("= ?\n");
	line_ended();
	return 0;
}
