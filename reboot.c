#include "defs.h"

#include "xlat/bootflags1.h"
#include "xlat/bootflags2.h"
#include "xlat/bootflags3.h"

SYS_FUNC(reboot)
{
	if (exiting(tcp))
		return 0;

	printflags(bootflags1, tcp->u_arg[0], "LINUX_REBOOT_MAGIC_???");
	tprints(", ");
	printflags(bootflags2, tcp->u_arg[1], "LINUX_REBOOT_MAGIC_???");
	tprints(", ");
	printflags(bootflags3, tcp->u_arg[2], "LINUX_REBOOT_CMD_???");
	if (tcp->u_arg[2] == (long) LINUX_REBOOT_CMD_RESTART2) {
		tprints(", ");
		printstr(tcp, tcp->u_arg[3], -1);
	}
	return 0;
}
