#include "defs.h"

#include <fcntl.h>

#ifndef AT_SYMLINK_NOFOLLOW
# define AT_SYMLINK_NOFOLLOW	0x100
#endif
#ifndef AT_REMOVEDIR
# define AT_REMOVEDIR		0x200
#endif
#ifndef AT_SYMLINK_FOLLOW
# define AT_SYMLINK_FOLLOW	0x400
#endif
#ifndef AT_NO_AUTOMOUNT
# define AT_NO_AUTOMOUNT	0x800
#endif
#ifndef AT_EMPTY_PATH
# define AT_EMPTY_PATH		0x1000
#endif

#include "xlat/at_flags.h"

int
sys_link(struct tcb *tcp)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprints(", ");
		printpath(tcp, tcp->u_arg[1]);
	}
	return 0;
}

int
sys_linkat(struct tcb *tcp)
{
	if (entering(tcp)) {
		print_dirfd(tcp, tcp->u_arg[0]);
		printpath(tcp, tcp->u_arg[1]);
		tprints(", ");
		print_dirfd(tcp, tcp->u_arg[2]);
		printpath(tcp, tcp->u_arg[3]);
		tprints(", ");
		printflags(at_flags, tcp->u_arg[4], "AT_???");
	}
	return 0;
}

int
sys_unlinkat(struct tcb *tcp)
{
	if (entering(tcp)) {
		print_dirfd(tcp, tcp->u_arg[0]);
		printpath(tcp, tcp->u_arg[1]);
		tprints(", ");
		printflags(at_flags, tcp->u_arg[2], "AT_???");
	}
	return 0;
}

int
sys_symlinkat(struct tcb *tcp)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprints(", ");
		print_dirfd(tcp, tcp->u_arg[1]);
		printpath(tcp, tcp->u_arg[2]);
	}
	return 0;
}
