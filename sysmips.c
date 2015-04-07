#include "defs.h"

#ifdef MIPS

#ifdef HAVE_LINUX_UTSNAME_H
# include <linux/utsname.h>
#endif
#ifdef HAVE_ASM_SYSMIPS_H
# include <asm/sysmips.h>
#endif

#ifndef __NEW_UTS_LEN
# define __NEW_UTS_LEN 64
#endif

#include "xlat/sysmips_operations.h"

SYS_FUNC(sysmips)
{
	if (entering(tcp)) {
		printxval(sysmips_operations, tcp->u_arg[0], "???");
		if (!verbose(tcp)) {
			tprintf("%ld, %ld, %ld", tcp->u_arg[1], tcp->u_arg[2], tcp->u_arg[3]);
		} else if (tcp->u_arg[0] == SETNAME) {
			char nodename[__NEW_UTS_LEN + 1];
			tprints(", ");
			if (umovestr(tcp, tcp->u_arg[1], (__NEW_UTS_LEN + 1),
				     nodename) < 0) {
				tprintf("%#lx", tcp->u_arg[1]);
			} else {
				print_quoted_string(nodename, __NEW_UTS_LEN + 1,
						    QUOTE_0_TERMINATED);
			}
		} else if (tcp->u_arg[0] == MIPS_ATOMIC_SET) {
			tprintf(", %#lx, 0x%lx", tcp->u_arg[1], tcp->u_arg[2]);
		} else if (tcp->u_arg[0] == MIPS_FIXADE) {
			tprintf(", 0x%lx", tcp->u_arg[1]);
		} else {
			tprintf("%ld, %ld, %ld", tcp->u_arg[1], tcp->u_arg[2], tcp->u_arg[3]);
		}
	}

	return 0;
}

#endif /* MIPS */
