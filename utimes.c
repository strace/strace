#include "defs.h"

SYS_FUNC(utimes)
{
	printpath(tcp, tcp->u_arg[0]);
	tprints(", ");
	print_timeval_pair(tcp, tcp->u_arg[1]);

	return RVAL_DECODED;
}

SYS_FUNC(futimesat)
{
	print_dirfd(tcp, tcp->u_arg[0]);
	printpath(tcp, tcp->u_arg[1]);
	tprints(", ");
	print_timeval_pair(tcp, tcp->u_arg[2]);

	return RVAL_DECODED;
}

SYS_FUNC(utimensat)
{
	print_dirfd(tcp, tcp->u_arg[0]);
	printpath(tcp, tcp->u_arg[1]);
	tprints(", ");
	print_timespec_utime_pair(tcp, tcp->u_arg[2]);
	tprints(", ");
	printflags(at_flags, tcp->u_arg[3], "AT_???");

	return RVAL_DECODED;
}

#ifdef ALPHA
SYS_FUNC(osf_utimes)
{
	printpath(tcp, tcp->u_arg[0]);
	tprints(", ");
	print_timeval32_pair(tcp, tcp->u_arg[1]);

	return RVAL_DECODED;
}
#endif /* ALPHA */
