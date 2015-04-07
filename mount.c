#include "defs.h"

#define MS_RDONLY	 1	/* Mount read-only */
#define MS_NOSUID	 2	/* Ignore suid and sgid bits */
#define MS_NODEV	 4	/* Disallow access to device special files */
#define MS_NOEXEC	 8	/* Disallow program execution */
#define MS_SYNCHRONOUS	16	/* Writes are synced at once */
#define MS_REMOUNT	32	/* Alter flags of a mounted FS */
#define MS_MANDLOCK	64	/* Allow mandatory locks on an FS */
#define MS_DIRSYNC	128	/* Directory modifications are synchronous */
#define MS_NOATIME	1024	/* Do not update access times. */
#define MS_NODIRATIME	2048	/* Do not update directory access times */
#define MS_BIND		4096
#define MS_MOVE		8192
#define MS_REC		16384
#define MS_SILENT	32768
#define MS_POSIXACL	(1<<16)	/* VFS does not apply the umask */
#define MS_UNBINDABLE	(1<<17)	/* change to unbindable */
#define MS_PRIVATE	(1<<18)	/* change to private */
#define MS_SLAVE	(1<<19)	/* change to slave */
#define MS_SHARED	(1<<20)	/* change to shared */
#define MS_RELATIME	(1<<21)
#define MS_KERNMOUNT	(1<<22)
#define MS_I_VERSION	(1<<23)
#define MS_STRICTATIME	(1<<24)
#define MS_NOSEC	(1<<28)
#define MS_BORN		(1<<29)
#define MS_ACTIVE	(1<<30)
#define MS_NOUSER	(1<<31)
#define MS_MGC_VAL	0xc0ed0000	/* Magic flag number */
#define MS_MGC_MSK	0xffff0000	/* Magic flag mask */

#include "xlat/mount_flags.h"

SYS_FUNC(mount)
{
	if (entering(tcp)) {
		int ignore_type = 0, ignore_data = 0;
		unsigned long flags = tcp->u_arg[3];

		/* Discard magic */
		if ((flags & MS_MGC_MSK) == MS_MGC_VAL)
			flags &= ~MS_MGC_MSK;

		if (flags & MS_REMOUNT)
			ignore_type = 1;
		else if (flags & (MS_BIND | MS_MOVE))
			ignore_type = ignore_data = 1;

		printpath(tcp, tcp->u_arg[0]);
		tprints(", ");

		printpath(tcp, tcp->u_arg[1]);
		tprints(", ");

		if (ignore_type && tcp->u_arg[2])
			tprintf("%#lx", tcp->u_arg[2]);
		else
			printstr(tcp, tcp->u_arg[2], -1);
		tprints(", ");

		printflags(mount_flags, tcp->u_arg[3], "MS_???");
		tprints(", ");

		if (ignore_data && tcp->u_arg[4])
			tprintf("%#lx", tcp->u_arg[4]);
		else
			printstr(tcp, tcp->u_arg[4], -1);
	}
	return 0;
}
