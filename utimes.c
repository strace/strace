#include "defs.h"

static int
decode_utimes(struct tcb *tcp, int offset, int special)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[offset]);
		tprints(", ");
		if (tcp->u_arg[offset + 1] == 0)
			tprints("NULL");
		else {
			tprints("{");
			printtv_bitness(tcp, tcp->u_arg[offset + 1],
					BITNESS_CURRENT, special);
			tprints(", ");
			printtv_bitness(tcp, tcp->u_arg[offset + 1]
					+ sizeof(struct timeval),
					BITNESS_CURRENT, special);
			tprints("}");
		}
	}
	return 0;
}

int
sys_utimes(struct tcb *tcp)
{
	return decode_utimes(tcp, 0, 0);
}

int
sys_futimesat(struct tcb *tcp)
{
	if (entering(tcp))
		print_dirfd(tcp, tcp->u_arg[0]);
	return decode_utimes(tcp, 1, 0);
}

int
sys_utimensat(struct tcb *tcp)
{
	if (entering(tcp)) {
		print_dirfd(tcp, tcp->u_arg[0]);
		decode_utimes(tcp, 1, 1);
		tprints(", ");
		printflags(at_flags, tcp->u_arg[3], "AT_???");
	}
	return 0;
}

#ifdef ALPHA
int
sys_osf_utimes(struct tcb *tcp)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprints(", ");
		printtv_bitness(tcp, tcp->u_arg[1], BITNESS_32,  0);
	}
	return 0;
}
#endif /* ALPHA */
