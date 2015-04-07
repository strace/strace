#include "defs.h"

#include <fcntl.h>
#include <sys/stat.h>

#ifdef MAJOR_IN_SYSMACROS
# include <sys/sysmacros.h>
#endif

#ifdef MAJOR_IN_MKDEV
# include <sys/mkdev.h>
#endif

static int
decode_mknod(struct tcb *tcp, int offset)
{
	int mode = tcp->u_arg[offset + 1];

	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[offset]);
		tprintf(", %s", sprintmode(mode));
		switch (mode & S_IFMT) {
		case S_IFCHR:
		case S_IFBLK:
#if defined(SPARC) || defined(SPARC64)
			if (current_personality == 1)
				tprintf(", makedev(%lu, %lu)",
				(unsigned long) ((tcp->u_arg[offset + 2] >> 18) & 0x3fff),
				(unsigned long) (tcp->u_arg[offset + 2] & 0x3ffff));
			else
#endif /* SPARC || SPARC64 */
				tprintf(", makedev(%lu, %lu)",
				(unsigned long) major(tcp->u_arg[offset + 2]),
				(unsigned long) minor(tcp->u_arg[offset + 2]));
			break;
		default:
			break;
		}
	}
	return 0;
}

SYS_FUNC(mknod)
{
	return decode_mknod(tcp, 0);
}

SYS_FUNC(mknodat)
{
	if (entering(tcp))
		print_dirfd(tcp, tcp->u_arg[0]);
	return decode_mknod(tcp, 1);
}

#if defined(SPARC) || defined(SPARC64)
SYS_FUNC(xmknod)
{
	int mode = tcp->u_arg[2];

	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
		printpath(tcp, tcp->u_arg[1]);
		tprintf(", %s", sprintmode(mode));
		switch (mode & S_IFMT) {
		case S_IFCHR:
		case S_IFBLK:
			tprintf(", makedev(%lu, %lu)",
				(unsigned long) ((tcp->u_arg[3] >> 18) & 0x3fff),
				(unsigned long) (tcp->u_arg[3] & 0x3ffff));
			break;
		default:
			break;
		}
	}
	return 0;
}
#endif /* SPARC || SPARC64 */
