/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/sysmacros.h>
#include <netinet/tcp.h>
#include "test_nlattr.h"
#include <linux/sock_diag.h>
#include <linux/unix_diag.h>

static void
init_unix_diag_msg(struct nlmsghdr *const nlh, const unsigned int msg_len)
{
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = msg_len,
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_DUMP
	);

	struct unix_diag_msg *const msg = NLMSG_DATA(nlh);
	SET_STRUCT(struct unix_diag_msg, msg,
		.udiag_family = AF_UNIX,
		.udiag_type = SOCK_STREAM,
		.udiag_state = TCP_ESTABLISHED
	);
}

static void
print_unix_diag_msg(const unsigned int msg_len)
{
	printf("{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {udiag_family=AF_UNIX"
	       ", udiag_type=SOCK_STREAM, udiag_state=TCP_ESTABLISHED"
	       ", udiag_ino=0, udiag_cookie=[0, 0]}",
	       msg_len);
}

static void
print_uint(const unsigned int *p, size_t i)
{
	printf("%u", *p);
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	static const struct unix_diag_vfs uv = {
		.udiag_vfs_dev = 0xabcddafa,
		.udiag_vfs_ino = 0xbafabcda
	};
	static const struct unix_diag_rqlen rql = {
		.udiag_rqueue = 0xfabdcdad,
		.udiag_wqueue = 0xbacdadcf
	};
	static const uint32_t inode[] = { 0xadbcadbc, 0xfabdcdac };

	const int fd = create_nl_socket(NETLINK_SOCK_DIAG);
	const unsigned int hdrlen = sizeof(struct unix_diag_msg);
	void *const nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen),
					 NLA_HDRLEN + sizeof(inode));

	static char pattern[4096];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);

	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_unix_diag_msg, print_unix_diag_msg,
			   UNIX_DIAG_VFS, pattern, uv,
			   printf("{udiag_vfs_dev=makedev(%#x, %#x)",
				  major(uv.udiag_vfs_dev),
				  minor(uv.udiag_vfs_dev));
			   PRINT_FIELD_U(", ", uv, udiag_vfs_ino);
			   printf("}"));

	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_unix_diag_msg, print_unix_diag_msg,
			   UNIX_DIAG_RQLEN, pattern, rql,
			   PRINT_FIELD_U("{", rql, udiag_rqueue);
			   PRINT_FIELD_U(", ", rql, udiag_wqueue);
			   printf("}"));

	TEST_NLATTR_ARRAY(fd, nlh0, hdrlen,
			  init_unix_diag_msg, print_unix_diag_msg,
			  UNIX_DIAG_ICONS, pattern, inode, print_uint);

	puts("+++ exited with 0 +++");
	return 0;
}
