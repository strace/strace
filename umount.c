#include "defs.h"
#include "xlat/umount_flags.h"

SYS_FUNC(umount2)
{
	printpath(tcp, tcp->u_arg[0]);
	tprints(", ");
	printflags(umount_flags, tcp->u_arg[1], "MNT_???");

	return RVAL_DECODED;
}
