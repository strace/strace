#include "defs.h"
#include <linux/reboot.h>

#include "xlat/bootflags1.h"
#include "xlat/bootflags2.h"
#include "xlat/bootflags3.h"

int
sys_reboot(struct tcb *tcp)
{
	if (exiting(tcp))
		return 0;

	printflags(bootflags1, tcp->u_arg[0], "LINUX_REBOOT_MAGIC_???");
	tprints(", ");
	printflags(bootflags2, tcp->u_arg[1], "LINUX_REBOOT_MAGIC_???");
	tprints(", ");
	printflags(bootflags3, tcp->u_arg[2], "LINUX_REBOOT_CMD_???");
	if (tcp->u_arg[2] == LINUX_REBOOT_CMD_RESTART2) {
		tprints(", ");
		printstr(tcp, tcp->u_arg[3], -1);
	}
	return 0;
}
