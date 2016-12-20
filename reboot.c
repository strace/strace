#include "defs.h"

#include "xlat/bootflags1.h"
#include "xlat/bootflags2.h"
#include "xlat/bootflags3.h"

SYS_FUNC(reboot)
{
	const unsigned int magic1 = tcp->u_arg[0];
	const unsigned int magic2 = tcp->u_arg[1];
	const unsigned int cmd = tcp->u_arg[2];

	printflags(bootflags1, magic1, "LINUX_REBOOT_MAGIC_???");
	tprints(", ");
	printflags(bootflags2, magic2, "LINUX_REBOOT_MAGIC_???");
	tprints(", ");
	printflags(bootflags3, cmd, "LINUX_REBOOT_CMD_???");
	if (cmd == LINUX_REBOOT_CMD_RESTART2) {
		tprints(", ");
		printstr(tcp, tcp->u_arg[3]);
	}
	return RVAL_DECODED;
}
