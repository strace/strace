#include "defs.h"

SYS_FUNC(utimes)
{
	printpath(tcp, tcp->u_arg[0]);
	tprints(", ");
	MPERS_PRINTER_NAME(print_timeval_pair)(tcp, tcp->u_arg[1]);

	return RVAL_DECODED;
}

SYS_FUNC(futimesat)
{
	print_dirfd(tcp, tcp->u_arg[0]);
	printpath(tcp, tcp->u_arg[1]);
	tprints(", ");
	MPERS_PRINTER_NAME(print_timeval_pair)(tcp, tcp->u_arg[2]);

	return RVAL_DECODED;
}

SYS_FUNC(utimensat)
{
	print_dirfd(tcp, tcp->u_arg[0]);
	printpath(tcp, tcp->u_arg[1]);
	tprints(", ");
	MPERS_PRINTER_NAME(print_timespec_utime_pair)(tcp, tcp->u_arg[2]);
	tprints(", ");
	printflags(at_flags, tcp->u_arg[3], "AT_???");

	return RVAL_DECODED;
}

#ifdef ALPHA
SYS_FUNC(osf_utimes)
{
	printpath(tcp, tcp->u_arg[0]);
	tprints(", ");
	printtv_bitness(tcp, tcp->u_arg[1], BITNESS_32, 0);

	return RVAL_DECODED;
}
#endif /* ALPHA */
