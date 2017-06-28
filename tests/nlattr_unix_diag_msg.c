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
#include "netlink.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <linux/rtnetlink.h>
#include <linux/sock_diag.h>
#include <linux/unix_diag.h>

static void
init_unix_diag_msg(struct nlmsghdr *nlh, unsigned int msg_len)
{
	struct unix_diag_msg *msg;

	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = msg_len,
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_DUMP
	);

	msg = NLMSG_DATA(nlh);
	SET_STRUCT(struct unix_diag_msg, msg,
		.udiag_family = AF_UNIX,
		.udiag_type = SOCK_STREAM,
		.udiag_state = TCP_FIN_WAIT1
	);
}

static void
test_unix_diag_vfs(const int fd)
{
	const int hdrlen = sizeof(struct unix_diag_msg);
	struct nlmsghdr *nlh;
	struct nlattr *nla;
	unsigned int nla_len;
	unsigned int msg_len;
	void *const nlh0 = tail_alloc(NLMSG_SPACE(hdrlen));
	long rc;

	/* len < sizeof(struct unix_diag_vfs) */
	nla_len = NLA_HDRLEN + 2;
	msg_len = NLMSG_SPACE(hdrlen) + nla_len;
	nlh = nlh0 - nla_len;
	init_unix_diag_msg(nlh, msg_len);

	nla = NLMSG_ATTR(nlh, hdrlen);
	SET_STRUCT(struct nlattr, nla,
		.nla_len = nla_len,
		.nla_type = UNIX_DIAG_VFS
	);
	memcpy(RTA_DATA(nla), "12", 2);

	rc = sendto(fd, nlh, msg_len, MSG_DONTWAIT, NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {udiag_family=AF_UNIX"
	       ", udiag_type=SOCK_STREAM, udiag_state=TCP_FIN_WAIT1"
	       ", udiag_ino=0, udiag_cookie=[0, 0]}, {{nla_len=%u"
	       ", nla_type=UNIX_DIAG_VFS}, \"12\"}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, nla_len, msg_len, sprintrc(rc));

	/* short read of unix_diag_vfs */
	nla_len = NLA_HDRLEN + sizeof(struct unix_diag_vfs);
	msg_len = NLMSG_SPACE(hdrlen) + nla_len;
	nlh = nlh0 - (nla_len - 1);
	init_unix_diag_msg(nlh, msg_len);

	nla = NLMSG_ATTR(nlh, hdrlen);
	SET_STRUCT(struct nlattr, nla,
		.nla_len = nla_len,
		.nla_type = UNIX_DIAG_VFS
	);

	rc = sendto(fd, nlh, msg_len, MSG_DONTWAIT, NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {udiag_family=AF_UNIX"
	       ", udiag_type=SOCK_STREAM, udiag_state=TCP_FIN_WAIT1"
	       ", udiag_ino=0, udiag_cookie=[0, 0]}, {{nla_len=%u"
	       ", nla_type=UNIX_DIAG_VFS}, %p}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, nla_len, RTA_DATA(nla), msg_len, sprintrc(rc));

	/* unix_diag_vfs */
	nla_len = NLA_HDRLEN + sizeof(struct unix_diag_vfs);
	msg_len = NLMSG_SPACE(hdrlen) + nla_len;
	nlh = nlh0 - nla_len;
	init_unix_diag_msg(nlh, msg_len);

	nla = NLMSG_ATTR(nlh, hdrlen);
	SET_STRUCT(struct nlattr, nla,
		.nla_len = nla_len,
		.nla_type = UNIX_DIAG_VFS
	);
	static const struct unix_diag_vfs uv = {
		.udiag_vfs_dev = 0xabcddafa,
		.udiag_vfs_ino = 0xbafabcda
	};
	memcpy(RTA_DATA(nla), &uv, sizeof(uv));

	rc = sendto(fd, nlh, msg_len, MSG_DONTWAIT, NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {udiag_family=AF_UNIX"
	       ", udiag_type=SOCK_STREAM, udiag_state=TCP_FIN_WAIT1"
	       ", udiag_ino=0, udiag_cookie=[0, 0]}, {{nla_len=%u"
	       ", nla_type=UNIX_DIAG_VFS}, {udiag_vfs_dev=makedev(%u, %u)"
	       ", udiag_vfs_ino=%u}}}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, nla_len, major(uv.udiag_vfs_dev),
	       minor(uv.udiag_vfs_dev), uv.udiag_vfs_ino,
	       msg_len, sprintrc(rc));
}

static void
test_unix_diag_icons(const int fd)
{
	const int hdrlen = sizeof(struct unix_diag_msg);
	struct nlmsghdr *nlh;
	struct nlattr *nla;
	unsigned int nla_len;
	unsigned int msg_len;
	void *const nlh0 = tail_alloc(NLMSG_SPACE(hdrlen));
	long rc;

	/* len < sizeof(uint32_t) */
	nla_len = NLA_HDRLEN + 2;
	msg_len = NLMSG_SPACE(hdrlen) + nla_len;
	nlh = nlh0 - nla_len;
	init_unix_diag_msg(nlh, msg_len);

	nla = NLMSG_ATTR(nlh, hdrlen);
	SET_STRUCT(struct nlattr, nla,
		.nla_len = nla_len,
		.nla_type = UNIX_DIAG_ICONS
	);
	memcpy(RTA_DATA(nla), "12", 2);

	rc = sendto(fd, nlh, msg_len, MSG_DONTWAIT, NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {udiag_family=AF_UNIX"
	       ", udiag_type=SOCK_STREAM, udiag_state=TCP_FIN_WAIT1"
	       ", udiag_ino=0, udiag_cookie=[0, 0]}, {{nla_len=%u"
	       ", nla_type=UNIX_DIAG_ICONS}, \"12\"}}, %u"
	       ", MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, nla_len, msg_len, sprintrc(rc));

	/* len == sizeof(uint32_t) * 2 - 1 */
	nla_len = NLA_HDRLEN + sizeof(uint32_t) * 2 - 1;
	msg_len = NLMSG_SPACE(hdrlen) + nla_len;
	nlh = nlh0 - nla_len;
	init_unix_diag_msg(nlh, msg_len);

	nla = NLMSG_ATTR(nlh, hdrlen);
	SET_STRUCT(struct nlattr, nla,
		.nla_len = nla_len,
		.nla_type = UNIX_DIAG_ICONS
	);
	static const uint32_t inode[] = { 0xadbcadbc, 0xfabdcdac };
	memcpy(RTA_DATA(nla), inode, sizeof(inode[0]));

	rc = sendto(fd, nlh, msg_len, MSG_DONTWAIT, NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {udiag_family=AF_UNIX"
	       ", udiag_type=SOCK_STREAM, udiag_state=TCP_FIN_WAIT1"
	       ", udiag_ino=0, udiag_cookie=[0, 0]}, {{nla_len=%u"
	       ", nla_type=UNIX_DIAG_ICONS}, [%u]}}, %u"
	       ", MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, nla_len, inode[0], msg_len, sprintrc(rc));

	/* len == sizeof(uint32_t) * 2 */
	nla_len = NLA_HDRLEN + sizeof(uint32_t) * 2;
	msg_len = NLMSG_SPACE(hdrlen) + nla_len;
	nlh = nlh0 - nla_len;
	init_unix_diag_msg(nlh, msg_len);

	nla = NLMSG_ATTR(nlh, hdrlen);
	SET_STRUCT(struct nlattr, nla,
		.nla_len = nla_len,
		.nla_type = UNIX_DIAG_ICONS
	);
	memcpy(RTA_DATA(nla), inode, sizeof(inode));

	rc = sendto(fd, nlh, msg_len, MSG_DONTWAIT, NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {udiag_family=AF_UNIX"
	       ", udiag_type=SOCK_STREAM, udiag_state=TCP_FIN_WAIT1"
	       ", udiag_ino=0, udiag_cookie=[0, 0]}, {{nla_len=%u"
	       ", nla_type=UNIX_DIAG_ICONS}, [%u, %u]}}, %u"
	       ", MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, nla_len,
	       inode[0], inode[1], msg_len, sprintrc(rc));
}

static void
test_unix_diag_rqlen(const int fd)
{
	const int hdrlen = sizeof(struct unix_diag_msg);
	struct nlmsghdr *nlh;
	struct nlattr *nla;
	unsigned int nla_len;
	unsigned int msg_len;
	void *const nlh0 = tail_alloc(NLMSG_SPACE(hdrlen));
	long rc;

	/* len < sizeof(struct unix_diag_rqlen) */
	nla_len = NLA_HDRLEN + 2;
	msg_len = NLMSG_SPACE(hdrlen) + nla_len;
	nlh = nlh0 - nla_len;
	init_unix_diag_msg(nlh, msg_len);

	nla = NLMSG_ATTR(nlh, hdrlen);
	SET_STRUCT(struct nlattr, nla,
		.nla_len = nla_len,
		.nla_type = UNIX_DIAG_RQLEN
	);
	memcpy(RTA_DATA(nla), "12", 2);

	rc = sendto(fd, nlh, msg_len, MSG_DONTWAIT, NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {udiag_family=AF_UNIX"
	       ", udiag_type=SOCK_STREAM, udiag_state=TCP_FIN_WAIT1"
	       ", udiag_ino=0, udiag_cookie=[0, 0]}, {{nla_len=%u"
	       ", nla_type=UNIX_DIAG_RQLEN}, \"12\"}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, nla_len, msg_len, sprintrc(rc));

	/* short read of unix_diag_rqlen */
	nla_len = NLA_HDRLEN + sizeof(struct unix_diag_rqlen);
	msg_len = NLMSG_SPACE(hdrlen) + nla_len;
	nlh = nlh0 - (nla_len - 1);
	init_unix_diag_msg(nlh, msg_len);

	nla = NLMSG_ATTR(nlh, hdrlen);
	SET_STRUCT(struct nlattr, nla,
		.nla_len = nla_len,
		.nla_type = UNIX_DIAG_RQLEN
	);

	rc = sendto(fd, nlh, msg_len, MSG_DONTWAIT, NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {udiag_family=AF_UNIX"
	       ", udiag_type=SOCK_STREAM, udiag_state=TCP_FIN_WAIT1"
	       ", udiag_ino=0, udiag_cookie=[0, 0]}, {{nla_len=%u"
	       ", nla_type=UNIX_DIAG_RQLEN}, %p}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, nla_len, RTA_DATA(nla),
	       msg_len, sprintrc(rc));

	/* unix_diag_rqlen */
	nla_len = NLA_HDRLEN + sizeof(struct unix_diag_rqlen);
	msg_len = NLMSG_SPACE(hdrlen) + nla_len;
	nlh = nlh0 - nla_len;
	init_unix_diag_msg(nlh, msg_len);

	nla = NLMSG_ATTR(nlh, hdrlen);
	SET_STRUCT(struct nlattr, nla,
		.nla_len = nla_len,
		.nla_type = UNIX_DIAG_RQLEN
	);
	static const struct unix_diag_rqlen rql = {
		.udiag_rqueue = 0xfabdcdad,
		.udiag_wqueue = 0xbacdadcf
	};
	memcpy(RTA_DATA(nla), &rql, sizeof(rql));

	rc = sendto(fd, nlh, msg_len, MSG_DONTWAIT, NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {udiag_family=AF_UNIX"
	       ", udiag_type=SOCK_STREAM, udiag_state=TCP_FIN_WAIT1"
	       ", udiag_ino=0, udiag_cookie=[0, 0]}, {{nla_len=%u"
	       ", nla_type=UNIX_DIAG_RQLEN}, {udiag_rqueue=%u"
	       ", udiag_wqueue=%u}}}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, nla_len, rql.udiag_rqueue,
	       rql.udiag_wqueue, msg_len, sprintrc(rc));
}

int main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	int fd = create_nl_socket(NETLINK_SOCK_DIAG);

	test_unix_diag_vfs(fd);
	test_unix_diag_icons(fd);
	test_unix_diag_rqlen(fd);

	printf("+++ exited with 0 +++\n");

	return 0;
}
