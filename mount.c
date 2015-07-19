#include "defs.h"

#define MS_MGC_VAL	0xc0ed0000	/* old magic mount flag number */
#define MS_MGC_MSK	0xffff0000	/* old magic mount flag mask */

#include "xlat/mount_flags.h"

SYS_FUNC(mount)
{
	bool ignore_type = false;
	bool ignore_data = false;
	bool old_magic = false;
	unsigned long flags = tcp->u_arg[3];

	/* Discard magic */
	if ((flags & MS_MGC_MSK) == MS_MGC_VAL) {
		flags &= ~MS_MGC_MSK;
		old_magic = true;
	}

	if (flags & MS_REMOUNT)
		ignore_type = true;
	else if (flags & (MS_BIND | MS_MOVE | MS_SHARED
			  | MS_PRIVATE | MS_SLAVE | MS_UNBINDABLE))
		ignore_type = ignore_data = true;

	printpath(tcp, tcp->u_arg[0]);
	tprints(", ");

	printpath(tcp, tcp->u_arg[1]);
	tprints(", ");

	if (ignore_type)
		printaddr(tcp->u_arg[2]);
	else
		printstr(tcp, tcp->u_arg[2], -1);
	tprints(", ");

	if (old_magic) {
		tprints("MS_MGC_VAL");
		if (flags)
			tprints("|");
	}
	if (flags || !old_magic)
		printflags(mount_flags, flags, "MS_???");
	tprints(", ");

	if (ignore_data)
		printaddr(tcp->u_arg[4]);
	else
		printstr(tcp, tcp->u_arg[4], -1);

	return RVAL_DECODED;
}
