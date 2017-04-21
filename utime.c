#include "defs.h"

#include DEF_MPERS_TYPE(utimbuf_t)

#include <utime.h>

typedef struct utimbuf utimbuf_t;

#include MPERS_DEFS

SYS_FUNC(utime)
{
	utimbuf_t u;

	printpath(tcp, tcp->u_arg[0]);
	tprints(", ");
	if (!umove_or_printaddr(tcp, tcp->u_arg[1], &u)) {
		tprintf("{actime=%lld", (long long) u.actime);
		tprints_comment(sprinttime(u.actime));
		tprintf(", modtime=%lld", (long long) u.modtime);
		tprints_comment(sprinttime(u.modtime));
		tprints("}");
	}

	return RVAL_DECODED;
}
