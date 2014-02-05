#include "defs.h"
#include <linux/reboot.h>

static const struct xlat bootflags1[] = {
	XLAT(LINUX_REBOOT_MAGIC1),
	XLAT_END
};

static const struct xlat bootflags2[] = {
	XLAT(LINUX_REBOOT_MAGIC2),
	XLAT(LINUX_REBOOT_MAGIC2A),
	XLAT(LINUX_REBOOT_MAGIC2B),
	XLAT(LINUX_REBOOT_MAGIC2C),
	XLAT_END
};

static const struct xlat bootflags3[] = {
	XLAT(LINUX_REBOOT_CMD_RESTART),
	XLAT(LINUX_REBOOT_CMD_HALT),
	XLAT(LINUX_REBOOT_CMD_CAD_ON),
	XLAT(LINUX_REBOOT_CMD_CAD_OFF),
	XLAT(LINUX_REBOOT_CMD_POWER_OFF),
	XLAT(LINUX_REBOOT_CMD_RESTART2),
	XLAT(LINUX_REBOOT_CMD_SW_SUSPEND),
	XLAT(LINUX_REBOOT_CMD_KEXEC),
	XLAT_END
};

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
