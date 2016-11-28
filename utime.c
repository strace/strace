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
		tprintf("{actime=%s,", sprinttime(u.actime));
		tprintf(" modtime=%s}", sprinttime(u.modtime));
	}

	return RVAL_DECODED;
}
