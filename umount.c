#include "defs.h"

#define MNT_FORCE	0x00000001	/* Attempt to forcibily umount */
#define MNT_DETACH	0x00000002	/* Just detach from the tree */
#define MNT_EXPIRE	0x00000004	/* Mark for expiry */

#include "xlat/umount_flags.h"

SYS_FUNC(umount2)
{
	if (entering(tcp)) {
		printstr(tcp, tcp->u_arg[0], -1);
		tprints(", ");
		printflags(umount_flags, tcp->u_arg[1], "MNT_???");
	}
	return 0;
}
