/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017 The strace developers.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
print_uint(const unsigned int *p)
{
	printf("%u", *p);
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	const int fd = create_nl_socket(NETLINK_SOCK_DIAG);
	const unsigned int hdrlen = sizeof(struct unix_diag_msg);
	void *const nlh0 = tail_alloc(NLMSG_SPACE(hdrlen));

	static char pattern[4096];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);

	static const struct unix_diag_vfs uv = {
		.udiag_vfs_dev = 0xabcddafa,
		.udiag_vfs_ino = 0xbafabcda
	};
	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_unix_diag_msg, print_unix_diag_msg,
			   UNIX_DIAG_VFS, pattern, uv,
			   printf("{udiag_vfs_dev=makedev(%u, %u)",
				  major(uv.udiag_vfs_dev),
				  minor(uv.udiag_vfs_dev));
			   PRINT_FIELD_U(", ", uv, udiag_vfs_ino);
			   printf("}"));

	static const struct unix_diag_rqlen rql = {
		.udiag_rqueue = 0xfabdcdad,
		.udiag_wqueue = 0xbacdadcf
	};
	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_unix_diag_msg, print_unix_diag_msg,
			   UNIX_DIAG_RQLEN, pattern, rql,
			   PRINT_FIELD_U("{", rql, udiag_rqueue);
			   PRINT_FIELD_U(", ", rql, udiag_wqueue);
			   printf("}"));

	static const uint32_t inode[] = { 0xadbcadbc, 0xfabdcdac };
	TEST_NLATTR_ARRAY(fd, nlh0, hdrlen,
			  init_unix_diag_msg, print_unix_diag_msg,
			  UNIX_DIAG_ICONS, pattern, inode, print_uint);

	puts("+++ exited with 0 +++");
	return 0;
}
