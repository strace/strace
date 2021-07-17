/*
 * Check decoding of reboot syscall.
 *
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <stdio.h>
#include <linux/reboot.h>
#include <unistd.h>

#define INVALID_MAGIC 319887762
#define INVALID_CMD 0x01234568

#define STR32 "AbCdEfGhIjKlMnOpQrStUvWxYz012345"
#define STR128 STR32 STR32 STR32 STR32

int
main(void)
{
	static const kernel_ulong_t bogus_magic1 =
		(kernel_ulong_t) 0xFFFFFFFFFFFFFFFFULL;
	static const kernel_ulong_t bogus_magic2 =
		(kernel_ulong_t) 0xdeadfacefa57beefULL;
	static const kernel_ulong_t bogus_cmd =
		(kernel_ulong_t) 0xbadda7a09caffee1ULL;
	static const char buf[] = "reboot";
	static const char str256_buf[] = STR128 STR128;

	long rc;
	char *str256 = tail_memdup(str256_buf, sizeof(str256_buf) - 1);

	rc = syscall(__NR_reboot, 0, 0, 0, 0);
	printf("reboot(0 /* LINUX_REBOOT_MAGIC_??? */, "
	       "0 /* LINUX_REBOOT_MAGIC_??? */, "
	       "LINUX_REBOOT_CMD_CAD_OFF) = %s\n",
	       sprintrc(rc));

	rc = syscall(__NR_reboot, bogus_magic1, bogus_magic2, bogus_cmd, -1);
	printf("reboot(%#x /* LINUX_REBOOT_MAGIC_??? */, "
	       "%#x /* LINUX_REBOOT_MAGIC_??? */, "
	       "%#x /* LINUX_REBOOT_CMD_??? */) = %s\n",
	       (unsigned int) bogus_magic1, (unsigned int) bogus_magic2,
	       (unsigned int) bogus_cmd, sprintrc(rc));

	rc = syscall(__NR_reboot, LINUX_REBOOT_MAGIC1,
			  INVALID_MAGIC, LINUX_REBOOT_CMD_RESTART2, buf);
	printf("reboot(LINUX_REBOOT_MAGIC1, %#x /* LINUX_REBOOT_MAGIC_??? */,"
	       " LINUX_REBOOT_CMD_RESTART2, \"%s\") = %s\n",
	       INVALID_MAGIC, buf, sprintrc(rc));

	rc = syscall(__NR_reboot, LINUX_REBOOT_MAGIC1,
		     LINUX_REBOOT_MAGIC2, INVALID_CMD);
	printf("reboot(LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2,"
	       " %#x /* LINUX_REBOOT_CMD_??? */) = %s\n",
	       INVALID_CMD, sprintrc(rc));

	rc = syscall(__NR_reboot, INVALID_MAGIC, LINUX_REBOOT_MAGIC2A,
			  LINUX_REBOOT_CMD_RESTART2, str256);
	printf("reboot(%#x /* LINUX_REBOOT_MAGIC_??? */, LINUX_REBOOT_MAGIC2A, "
	       "LINUX_REBOOT_CMD_RESTART2, \"%.255s\"...) = %s\n",
	       INVALID_MAGIC, str256, sprintrc(rc));

	rc = syscall(__NR_reboot, INVALID_MAGIC, LINUX_REBOOT_MAGIC2B,
			  LINUX_REBOOT_CMD_RESTART2, str256 + 1);
	printf("reboot(%#x /* LINUX_REBOOT_MAGIC_??? */, LINUX_REBOOT_MAGIC2B, "
	       "LINUX_REBOOT_CMD_RESTART2, \"%.255s\"...) = %s\n",
	       INVALID_MAGIC, str256 + 1, sprintrc(rc));

	rc = syscall(__NR_reboot, INVALID_MAGIC, LINUX_REBOOT_MAGIC2C,
			  LINUX_REBOOT_CMD_RESTART2, str256 + 2);
	printf("reboot(%#x /* LINUX_REBOOT_MAGIC_??? */, LINUX_REBOOT_MAGIC2C, "
	       "LINUX_REBOOT_CMD_RESTART2, %p) = %s\n",
	       INVALID_MAGIC, str256 + 2, sprintrc(rc));

	str256[255] = '\0';
	rc = syscall(__NR_reboot, INVALID_MAGIC, bogus_magic1,
			  LINUX_REBOOT_CMD_RESTART2, str256);
	printf("reboot(%#x /* LINUX_REBOOT_MAGIC_??? */, "
	       "%#x /* LINUX_REBOOT_MAGIC_??? */, "
	       "LINUX_REBOOT_CMD_RESTART2, \"%.255s\"...) = %s\n",
	       INVALID_MAGIC, (unsigned int) bogus_magic1, str256,
	       sprintrc(rc));

	rc = syscall(__NR_reboot, INVALID_MAGIC, bogus_magic1,
			  LINUX_REBOOT_CMD_RESTART2, str256 + 1);
	printf("reboot(%#x /* LINUX_REBOOT_MAGIC_??? */, "
	       "%#x /* LINUX_REBOOT_MAGIC_??? */, "
	       "LINUX_REBOOT_CMD_RESTART2, \"%.254s\") = %s\n",
	       INVALID_MAGIC, (unsigned int) bogus_magic1, str256 + 1,
	       sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}
