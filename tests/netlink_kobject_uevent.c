/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "netlink.h"
#include "netlink_kobject_uevent.h"

static const char *errstr;

static ssize_t
sys_send(const int fd, const void *const buf, const size_t len)
{
	const ssize_t rc = sendto(fd, buf, len, MSG_DONTWAIT, NULL, 0);
	errstr = sprintrc(rc);
	return rc;
}

static void
test_nlmsg_type_udev(const int fd)
{
	static const char extra[] = "12345678";
	struct udev_monitor_netlink_header uh = {
		.prefix = "libudev",
		.magic = htonl(0xfeedcafe),
		.header_size = sizeof(uh),
		.properties_off = 40,
		.properties_len = 299,
		.filter_subsystem_hash = htonl(0xc370b302),
		.filter_devtype_hash = htonl(0x10800000),
		.filter_tag_bloom_hi = htonl(0x2000400),
		.filter_tag_bloom_lo = htonl(0x10800000),
	};
	const unsigned int extra_len = LENGTH_OF(extra);
	const unsigned int uh_len = sizeof(uh);

	char *const buf = tail_alloc(uh_len + extra_len);
	memcpy(buf + extra_len, &uh, uh_len);

	sys_send(fd, buf + extra_len, uh_len);
	printf("sendto(%d, {prefix=\"%s\", magic=htonl(%#x)"
	       ", header_size=%u, properties_off=%u, properties_len=%u"
	       ", filter_subsystem_hash=htonl(%#x)"
	       ", filter_devtype_hash=htonl(%#x)"
	       ", filter_tag_bloom_hi=htonl(%#x)"
	       ", filter_tag_bloom_lo=htonl(%#x)}, %u, MSG_DONTWAIT, NULL, "
	       "0) = %s\n"
	       , fd, uh.prefix,
	       ntohl(uh.magic), uh.header_size, uh.properties_off,
	       uh.properties_len, ntohl(uh.filter_subsystem_hash),
	       ntohl(uh.filter_devtype_hash), ntohl(uh.filter_tag_bloom_hi),
	       ntohl(uh.filter_tag_bloom_lo), uh_len, errstr);

	memcpy(buf, &uh, uh_len);
	memcpy(buf + uh_len, extra, extra_len);
	sys_send(fd, buf, uh_len + extra_len);
	printf("sendto(%d, [{prefix=\"%s\", magic=htonl(%#x)"
	       ", header_size=%u, properties_off=%u, properties_len=%u"
	       ", filter_subsystem_hash=htonl(%#x)"
	       ", filter_devtype_hash=htonl(%#x)"
	       ", filter_tag_bloom_hi=htonl(%#x)"
	       ", filter_tag_bloom_lo=htonl(%#x)}, "
	       , fd, uh.prefix,
	       ntohl(uh.magic), uh.header_size, uh.properties_off,
	       uh.properties_len, ntohl(uh.filter_subsystem_hash),
	       ntohl(uh.filter_devtype_hash), ntohl(uh.filter_tag_bloom_hi),
	       ntohl(uh.filter_tag_bloom_lo));
	print_quoted_memory(buf + uh_len, extra_len);
	printf("], %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       uh_len + extra_len, errstr);

	memcpy(buf + extra_len + 1, &uh, uh_len - 1);
	sys_send(fd, buf + extra_len + 1, uh_len);
	printf("sendto(%d, ", fd);
	print_quoted_memory(&uh, MIN(uh_len - 1, DEFAULT_STRLEN));
	printf("%s, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       (uh_len - 1 > DEFAULT_STRLEN ? "..." : ""),
	       uh_len, errstr);
}

static void
test_nlmsg_type_kernel(const int fd)
{
	struct udev_monitor_netlink_header uh = {
		.prefix = "change@",
		.magic = htonl(0xfeedcafe),
		.header_size = sizeof(uh),
		.properties_off = 10,
		.properties_len = 299,
		.filter_subsystem_hash = htonl(0xfffffff),
		.filter_devtype_hash = htonl(0x10000000),
		.filter_tag_bloom_hi = htonl(0x2000400),
	};
	const unsigned int uh_len = sizeof(uh);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct udev_monitor_netlink_header, p);
	memcpy(p, &uh, uh_len);

	sys_send(fd, p, uh_len);
	printf("sendto(%d, ", fd);
	print_quoted_memory(&uh, MIN(uh_len, DEFAULT_STRLEN));
	printf("%s, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       (uh_len > DEFAULT_STRLEN ? "..." : ""),
	       uh_len, errstr);
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	int fd = create_nl_socket(NETLINK_KOBJECT_UEVENT);

	test_nlmsg_type_udev(fd);
	test_nlmsg_type_kernel(fd);
	/* test using data that looks like a zero-length C string */
	char *const buf = tail_alloc(DEFAULT_STRLEN + 1);
	buf[0] = '=';
	fill_memory_ex(buf + 1, DEFAULT_STRLEN, 0, DEFAULT_STRLEN);

	sys_send(fd, buf + 1, DEFAULT_STRLEN);
	printf("sendto(%d, ", fd);
	print_quoted_memory(buf + 1, DEFAULT_STRLEN);
	printf(", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       DEFAULT_STRLEN, errstr);

	sys_send(fd, buf, DEFAULT_STRLEN + 1);
	printf("sendto(%d, ", fd);
	print_quoted_memory(buf, DEFAULT_STRLEN);
	printf("..., %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       DEFAULT_STRLEN + 1, errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
