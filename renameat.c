#include "defs.h"

static void
decode_renameat(struct tcb *tcp)
{
	print_dirfd(tcp, tcp->u_arg[0]);
	printpath(tcp, tcp->u_arg[1]);
	tprints(", ");
	print_dirfd(tcp, tcp->u_arg[2]);
	printpath(tcp, tcp->u_arg[3]);
}

int
sys_renameat(struct tcb *tcp)
{
	if (entering(tcp)) {
		decode_renameat(tcp);
	}
	return 0;
}

#include <linux/fs.h>
#include "xlat/rename_flags.h"

int
sys_renameat2(struct tcb *tcp)
{
	if (entering(tcp)) {
		decode_renameat(tcp);
		tprints(", ");
		printflags(rename_flags, tcp->u_arg[4], "RENAME_??");
	}
	return 0;
}
