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
#include <linux/netlink_diag.h>
#include <linux/rtnetlink.h>
#include <linux/sock_diag.h>

static void
init_netlink_diag_msg(struct nlmsghdr *nlh, unsigned int msg_len)
{
	struct netlink_diag_msg *msg;

	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = msg_len,
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_DUMP
	);

	msg = NLMSG_DATA(nlh);
	SET_STRUCT(struct netlink_diag_msg, msg,
		.ndiag_family = AF_NETLINK,
		.ndiag_type = SOCK_RAW,
		.ndiag_protocol = NETLINK_ROUTE,
		.ndiag_state = NETLINK_CONNECTED
	);
}

static void
test_netlink_diag_groups(const int fd)
{
	const int hdrlen = sizeof(struct netlink_diag_msg);
	struct nlmsghdr *nlh;
	struct nlattr *nla;
	unsigned int nla_len;
	unsigned int msg_len;
	void *const nlh0 = tail_alloc(NLMSG_SPACE(hdrlen));
	long rc;

	/* len < sizeof(unsigned long) */
	nla_len = NLA_HDRLEN + 2;
	msg_len = NLMSG_SPACE(hdrlen) + nla_len;
	nlh = nlh0 - nla_len;
	init_netlink_diag_msg(nlh, msg_len);

	nla = NLMSG_ATTR(nlh, hdrlen);
	SET_STRUCT(struct nlattr, nla,
		.nla_len = nla_len,
		.nla_type = NETLINK_DIAG_GROUPS
	);
	memcpy(RTA_DATA(nla), "12", 2);

	rc = sendto(fd, nlh, msg_len, MSG_DONTWAIT, NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {ndiag_family=AF_NETLINK"
	       ", ndiag_type=SOCK_RAW, ndiag_protocol=NETLINK_ROUTE"
	       ", ndiag_state=NETLINK_CONNECTED, ndiag_portid=0"
	       ", ndiag_dst_portid=0, ndiag_dst_group=0, ndiag_ino=0"
	       ", ndiag_cookie=[0, 0]}, {{nla_len=%u"
	       ", nla_type=NETLINK_DIAG_GROUPS}, \"12\"}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, nla_len, msg_len, sprintrc(rc));

	/* len == sizeof(unsigned long) * 2 - 1 */
	nla_len = NLA_HDRLEN + sizeof(unsigned long) * 2 - 1;
	msg_len = NLMSG_SPACE(hdrlen) + nla_len;
	nlh = nlh0 - nla_len;
	init_netlink_diag_msg(nlh, msg_len);

	nla = NLMSG_ATTR(nlh, hdrlen);
	SET_STRUCT(struct nlattr, nla,
		.nla_len = nla_len,
		.nla_type = NETLINK_DIAG_GROUPS
	);
	static const unsigned long groups[] = {
		(unsigned long) 0xdeadbeefbadc0dedULL,
		(unsigned long) 0xdeadbeefbadc0dedULL
	};
	memcpy(RTA_DATA(nla), groups, sizeof(groups[0]));

	rc = sendto(fd, nlh, msg_len, MSG_DONTWAIT, NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {ndiag_family=AF_NETLINK"
	       ", ndiag_type=SOCK_RAW, ndiag_protocol=NETLINK_ROUTE"
	       ", ndiag_state=NETLINK_CONNECTED, ndiag_portid=0"
	       ", ndiag_dst_portid=0, ndiag_dst_group=0, ndiag_ino=0"
	       ", ndiag_cookie=[0, 0]}, {{nla_len=%u"
	       ", nla_type=NETLINK_DIAG_GROUPS}, [%#lx]}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, nla_len, groups[0], msg_len, sprintrc(rc));

	/* len < sizeof(unsigned long) * 2 */
	nla_len = NLA_HDRLEN + sizeof(unsigned long) * 2;
	msg_len = NLMSG_SPACE(hdrlen) + nla_len;
	nlh = nlh0 - nla_len;
	init_netlink_diag_msg(nlh, msg_len);

	nla = NLMSG_ATTR(nlh, hdrlen);
	SET_STRUCT(struct nlattr, nla,
		.nla_len = nla_len,
		.nla_type = NETLINK_DIAG_GROUPS
	);
	memcpy(RTA_DATA(nla), groups, sizeof(groups));

	rc = sendto(fd, nlh, msg_len, MSG_DONTWAIT, NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {ndiag_family=AF_NETLINK"
	       ", ndiag_type=SOCK_RAW, ndiag_protocol=NETLINK_ROUTE"
	       ", ndiag_state=NETLINK_CONNECTED, ndiag_portid=0"
	       ", ndiag_dst_portid=0, ndiag_dst_group=0, ndiag_ino=0"
	       ", ndiag_cookie=[0, 0]}, {{nla_len=%u"
	       ", nla_type=NETLINK_DIAG_GROUPS}, [%#lx, %#lx]}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, nla_len, groups[0],
	       groups[1], msg_len, sprintrc(rc));
}

static void
test_netlink_diag_rx_ring(const int fd)
{
	const int hdrlen = sizeof(struct netlink_diag_msg);
	struct nlmsghdr *nlh;
	struct nlattr *nla;
	unsigned int nla_len;
	unsigned int msg_len;
	void *const nlh0 = tail_alloc(NLMSG_SPACE(hdrlen));
	long rc;

	/* len < sizeof(struct netlink_diag_ring) */
	nla_len = NLA_HDRLEN + 2;
	msg_len = NLMSG_SPACE(hdrlen) + nla_len;
	nlh = nlh0 - nla_len;
	init_netlink_diag_msg(nlh, msg_len);

	nla = NLMSG_ATTR(nlh, hdrlen);
	SET_STRUCT(struct nlattr, nla,
		.nla_len = nla_len,
		.nla_type = NETLINK_DIAG_RX_RING
	);
	memcpy(RTA_DATA(nla), "12", 2);

	rc = sendto(fd, nlh, msg_len, MSG_DONTWAIT, NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {ndiag_family=AF_NETLINK"
	       ", ndiag_type=SOCK_RAW, ndiag_protocol=NETLINK_ROUTE"
	       ", ndiag_state=NETLINK_CONNECTED, ndiag_portid=0"
	       ", ndiag_dst_portid=0, ndiag_dst_group=0, ndiag_ino=0"
	       ", ndiag_cookie=[0, 0]}, {{nla_len=%u"
	       ", nla_type=NETLINK_DIAG_RX_RING}, \"12\"}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, nla_len, msg_len, sprintrc(rc));

	/* short read of netlink_diag_ring */
	nla_len = NLA_HDRLEN + sizeof(struct netlink_diag_ring);
	msg_len = NLMSG_SPACE(hdrlen) + nla_len;
	nlh = nlh0 - (nla_len - 1);
	init_netlink_diag_msg(nlh, msg_len);

	nla = NLMSG_ATTR(nlh, hdrlen);
	SET_STRUCT(struct nlattr, nla,
		.nla_len = nla_len,
		.nla_type = NETLINK_DIAG_RX_RING
	);

	rc = sendto(fd, nlh, msg_len, MSG_DONTWAIT, NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {ndiag_family=AF_NETLINK"
	       ", ndiag_type=SOCK_RAW, ndiag_protocol=NETLINK_ROUTE"
	       ", ndiag_state=NETLINK_CONNECTED, ndiag_portid=0"
	       ", ndiag_dst_portid=0, ndiag_dst_group=0, ndiag_ino=0"
	       ", ndiag_cookie=[0, 0]}, {{nla_len=%u"
	       ", nla_type=NETLINK_DIAG_RX_RING}, %p}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, nla_len, RTA_DATA(nla), msg_len, sprintrc(rc));

	/* netlink_diag_ring */
	nla_len = NLA_HDRLEN + sizeof(struct netlink_diag_ring);
	msg_len = NLMSG_SPACE(hdrlen) + nla_len;
	nlh = nlh0 - nla_len;
	init_netlink_diag_msg(nlh, msg_len);

	nla = NLMSG_ATTR(nlh, hdrlen);
	SET_STRUCT(struct nlattr, nla,
		.nla_len = nla_len,
		.nla_type = NETLINK_DIAG_RX_RING
	);
	static const struct netlink_diag_ring ndr = {
		.ndr_block_size = 0xfabfabdc,
		.ndr_block_nr = 0xabcdabda,
		.ndr_frame_size = 0xcbadbafa,
		.ndr_frame_nr = 0xdbcafadb
	};
	memcpy(RTA_DATA(nla), &ndr, sizeof(ndr));

	rc = sendto(fd, nlh, msg_len, MSG_DONTWAIT, NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {ndiag_family=AF_NETLINK"
	       ", ndiag_type=SOCK_RAW, ndiag_protocol=NETLINK_ROUTE"
	       ", ndiag_state=NETLINK_CONNECTED, ndiag_portid=0"
	       ", ndiag_dst_portid=0, ndiag_dst_group=0, ndiag_ino=0"
	       ", ndiag_cookie=[0, 0]}, {{nla_len=%u"
	       ", nla_type=NETLINK_DIAG_RX_RING}, {ndr_block_size=%u"
	       ", ndr_block_nr=%u, ndr_frame_size=%u, ndr_frame_nr=%u}}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, nla_len, ndr.ndr_block_size,
	       ndr.ndr_block_nr, ndr.ndr_frame_size,
	       ndr.ndr_frame_nr, msg_len, sprintrc(rc));
}

static void
test_netlink_diag_flags(const int fd)
{
	const int hdrlen = sizeof(struct netlink_diag_msg);
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
	init_netlink_diag_msg(nlh, msg_len);

	nla = NLMSG_ATTR(nlh, hdrlen);
	SET_STRUCT(struct nlattr, nla,
		.nla_len = nla_len,
		.nla_type = NETLINK_DIAG_FLAGS
	);
	memcpy(RTA_DATA(nla), "12", 2);

	rc = sendto(fd, nlh, msg_len, MSG_DONTWAIT, NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {ndiag_family=AF_NETLINK"
	       ", ndiag_type=SOCK_RAW, ndiag_protocol=NETLINK_ROUTE"
	       ", ndiag_state=NETLINK_CONNECTED, ndiag_portid=0"
	       ", ndiag_dst_portid=0, ndiag_dst_group=0, ndiag_ino=0"
	       ", ndiag_cookie=[0, 0]}, {{nla_len=%u"
	       ", nla_type=NETLINK_DIAG_FLAGS}, \"12\"}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, nla_len, msg_len, sprintrc(rc));

	/* short read of flags */
	nla_len = NLA_HDRLEN + sizeof(uint32_t);
	msg_len = NLMSG_SPACE(hdrlen) + nla_len;
	nlh = nlh0 - (nla_len - 1);
	init_netlink_diag_msg(nlh, msg_len);

	nla = NLMSG_ATTR(nlh, hdrlen);
	SET_STRUCT(struct nlattr, nla,
		.nla_len = nla_len,
		.nla_type = NETLINK_DIAG_FLAGS
	);

	rc = sendto(fd, nlh, msg_len, MSG_DONTWAIT, NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {ndiag_family=AF_NETLINK"
	       ", ndiag_type=SOCK_RAW, ndiag_protocol=NETLINK_ROUTE"
	       ", ndiag_state=NETLINK_CONNECTED, ndiag_portid=0"
	       ", ndiag_dst_portid=0, ndiag_dst_group=0, ndiag_ino=0"
	       ", ndiag_cookie=[0, 0]}, {{nla_len=%u"
	       ", nla_type=NETLINK_DIAG_FLAGS}, %p}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, nla_len, RTA_DATA(nla), msg_len, sprintrc(rc));

	/* flags */
	nla_len = NLA_HDRLEN + sizeof(uint32_t);
	msg_len = NLMSG_SPACE(hdrlen) + nla_len;
	nlh = nlh0 - nla_len;
	init_netlink_diag_msg(nlh, msg_len);

	nla = NLMSG_ATTR(nlh, hdrlen);
	SET_STRUCT(struct nlattr, nla,
		.nla_len = nla_len,
		.nla_type = NETLINK_DIAG_FLAGS
	);
	static const uint32_t flags =
		NDIAG_FLAG_CB_RUNNING | NDIAG_FLAG_PKTINFO;
	memcpy(RTA_DATA(nla), &flags, sizeof(flags));

	rc = sendto(fd, nlh, msg_len, MSG_DONTWAIT, NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {ndiag_family=AF_NETLINK"
	       ", ndiag_type=SOCK_RAW, ndiag_protocol=NETLINK_ROUTE"
	       ", ndiag_state=NETLINK_CONNECTED, ndiag_portid=0"
	       ", ndiag_dst_portid=0, ndiag_dst_group=0, ndiag_ino=0"
	       ", ndiag_cookie=[0, 0]}, {{nla_len=%u"
	       ", nla_type=NETLINK_DIAG_FLAGS}"
	       ", NDIAG_FLAG_CB_RUNNING|NDIAG_FLAG_PKTINFO}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, nla_len, msg_len, sprintrc(rc));
}

int main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	int fd = create_nl_socket(NETLINK_SOCK_DIAG);

	test_netlink_diag_groups(fd);
	test_netlink_diag_rx_ring(fd);
	test_netlink_diag_flags(fd);

	printf("+++ exited with 0 +++\n");

	return 0;
}
