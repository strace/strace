#include "defs.h"
#include <sys/times.h>

SYS_FUNC(times)
{
	struct tms tbuf;

	if (entering(tcp))
		return 0;

	if (!umove_or_printaddr(tcp, tcp->u_arg[0], &tbuf)) {
		tprintf("{tms_utime=%llu, tms_stime=%llu, ",
			(unsigned long long) tbuf.tms_utime,
			(unsigned long long) tbuf.tms_stime);
		tprintf("tms_cutime=%llu, tms_cstime=%llu}",
			(unsigned long long) tbuf.tms_cutime,
			(unsigned long long) tbuf.tms_cstime);
	}

	return syserror(tcp) ? RVAL_DECIMAL : RVAL_UDECIMAL;
}
