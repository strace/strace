#include "defs.h"

static void
decode_utimes(struct tcb *tcp, int offset, int special)
{
	printpath(tcp, tcp->u_arg[offset]);
	tprints(", ");
	if (tcp->u_arg[offset + 1] == 0)
		tprints("NULL");
	else {
		tprints("[");
		printtv_bitness(tcp, tcp->u_arg[offset + 1],
				BITNESS_CURRENT, special);
		tprints(", ");
		printtv_bitness(tcp, tcp->u_arg[offset + 1]
				+ sizeof(struct timeval),
				BITNESS_CURRENT, special);
		tprints("]");
	}
}

SYS_FUNC(utimes)
{
	decode_utimes(tcp, 0, 0);

	return RVAL_DECODED;
}

SYS_FUNC(futimesat)
{
	print_dirfd(tcp, tcp->u_arg[0]);
	decode_utimes(tcp, 1, 0);

	return RVAL_DECODED;
}

SYS_FUNC(utimensat)
{
	print_dirfd(tcp, tcp->u_arg[0]);
	decode_utimes(tcp, 1, 1);
	tprints(", ");
	printflags(at_flags, tcp->u_arg[3], "AT_???");

	return RVAL_DECODED;
}

#ifdef ALPHA
SYS_FUNC(osf_utimes)
{
	printpath(tcp, tcp->u_arg[0]);
	tprints(", ");
	printtv_bitness(tcp, tcp->u_arg[1], BITNESS_32,  0);

	return RVAL_DECODED;
}
#endif /* ALPHA */
