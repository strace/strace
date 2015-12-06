#include "defs.h"
#include DEF_MPERS_TYPE(tms_t)
#include <sys/times.h>
typedef struct tms tms_t;
#include MPERS_DEFS

SYS_FUNC(times)
{
	tms_t tbuf;

	if (entering(tcp))
		return 0;

	if (!umove_or_printaddr(tcp, tcp->u_arg[0], &tbuf)) {
		tprintf("{tms_utime=%Lu, tms_stime=%Lu, ",
			(unsigned long long) tbuf.tms_utime,
			(unsigned long long) tbuf.tms_stime);
		tprintf("tms_cutime=%Lu, tms_cstime=%Lu}",
			(unsigned long long) tbuf.tms_cutime,
			(unsigned long long) tbuf.tms_cstime);
	}

	return syserror(tcp) ? RVAL_DECIMAL :
#if defined(RVAL_LUDECIMAL) && !defined(IN_MPERS)
			       RVAL_LUDECIMAL;
#else
			       RVAL_UDECIMAL;
#endif
}
