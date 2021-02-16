/*
 * Copyright (c) 2018-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_NETLINK_KOBJECT_UEVENT_H
# define STRACE_NETLINK_KOBJECT_UEVENT_H

struct udev_monitor_netlink_header {
	/* "libudev" prefix to distinguish libudev and kernel messages */
	char prefix[8];
	unsigned int magic;
	unsigned int header_size;
	unsigned int properties_off;
	unsigned int properties_len;
	unsigned int filter_subsystem_hash;
	unsigned int filter_devtype_hash;
	unsigned int filter_tag_bloom_hi;
	unsigned int filter_tag_bloom_lo;
};

#endif /* !STRACE_NETLINK_KOBJECT_UEVENT_H */
