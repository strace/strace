/*
 * Check printing of the counter endpoint of /dev/net/tun in -yy mode.
 *
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <linux/sched.h>

int
main(void)
{
	/*
	 * Try a user namespace for CAP_SYS_ADMIN/CAP_NET_ADMIN
	 * before creating a network namespace.
	 */
	(void) syscall(__NR_unshare, CLONE_NEWUSER);

	/*
	 * Now enter a new network namespace
	 * owned by the child user namespace.
	 */
	if (syscall(__NR_unshare, CLONE_NEWNET) < 0)
		perror_msg_and_skip("unshare(CLONE_NEWNET)");

	static const struct {
		char name[IFNAMSIZ];
		const char *expected;
	} names[] = {
		{ "stracetun0", "stracetun0" },
		{ "tun<>\\", "tun\\74\\76\\\\" },
		{ "stracetun123456", "stracetun123456" },
	};
	static const char dev_tun[] = "/dev/net/tun";

	for (unsigned int i = 0; i < ARRAY_SIZE(names); ++i) {
		int tun_fd = open(dev_tun, O_RDWR);
		if (tun_fd < 0)
			perror_msg_and_skip("open: %s", dev_tun);

		struct stat st;
		if (fstat(tun_fd, &st) < 0)
			perror_msg_and_skip("fstat: %s", dev_tun);

		const char *real_tun = get_fd_path(tun_fd);
		struct ifreq ifr = { .ifr_flags = IFF_TUN };
		memcpy(ifr.ifr_name, names[i].name, sizeof(ifr.ifr_name));

		if (ioctl(tun_fd, TUNSETIFF, &ifr) < 0) {
			if (!i)
				perror_msg_and_skip("ioctl: TUNSETIFF");
			perror_msg_and_fail("ioctl: TUNSETIFF: %s", names[i].name);
		}

		int rc = close(tun_fd);
		printf("close(%d<%s<char %u:%u @%s>>) = %d\n",
		       tun_fd, real_tun, major(st.st_rdev), minor(st.st_rdev),
		       names[i].expected, rc);
	}

	puts("+++ exited with 0 +++");
	return 0;
}
