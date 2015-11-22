#include "defs.h"
#include "xlat/kcmp_types.h"

SYS_FUNC(kcmp)
{
	pid_t pid1 = tcp->u_arg[0];
	pid_t pid2 = tcp->u_arg[1];
	int type = tcp->u_arg[2];
	unsigned long idx1 = tcp->u_arg[3];
	unsigned long idx2 = tcp->u_arg[4];

	tprintf("%d, %d, ", pid1, pid2);
	printxval(kcmp_types, type, "KCMP_???");

	switch(type) {
		case KCMP_FILE:
			tprintf(", %u, %u", (unsigned) idx1, (unsigned) idx2);
			break;
		case KCMP_FILES:
		case KCMP_FS:
		case KCMP_IO:
		case KCMP_SIGHAND:
		case KCMP_SYSVSEM:
		case KCMP_VM:
			break;
		default:
			tprintf(", %#lx, %#lx", idx1, idx2);
	}

	return RVAL_DECODED;
}
