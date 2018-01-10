#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_reboot

# include <stdio.h>
# include <linux/reboot.h>
# include <unistd.h>

# define INVALID_MAGIC 319887762
# define INVALID_CMD 0x01234568

int
main(void)
{
	static const char buf[] = "reboot";
	long rc = syscall(__NR_reboot, LINUX_REBOOT_MAGIC1,
			  INVALID_MAGIC, LINUX_REBOOT_CMD_RESTART2, buf);
	printf("reboot(LINUX_REBOOT_MAGIC1, %#x /* LINUX_REBOOT_MAGIC_??? */,"
	       " LINUX_REBOOT_CMD_RESTART2, \"%s\") = %s\n",
	       INVALID_MAGIC, buf, sprintrc(rc));

	rc = syscall(__NR_reboot, LINUX_REBOOT_MAGIC1,
		     LINUX_REBOOT_MAGIC2, INVALID_CMD);
	printf("reboot(LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2,"
	       " %#x /* LINUX_REBOOT_CMD_??? */) = %s\n",
	       INVALID_CMD, sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_reboot")

#endif
