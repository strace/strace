/*
 * Check decoding of netlink attribute.
 *
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include "netlink.h"
#include <linux/rtnetlink.h>
#include <linux/sock_diag.h>
#include <linux/unix_diag.h>

static void
test_nlattr(const int fd)
{
	static const struct msg {
		struct nlmsghdr nlh;
		struct unix_diag_msg udm;
	} c_msg = {
		.nlh = {
			.nlmsg_len = sizeof(struct msg),
			.nlmsg_type = SOCK_DIAG_BY_FAMILY,
			.nlmsg_flags = NLM_F_DUMP
		},
		.udm = {
			.udiag_family = AF_UNIX,
			.udiag_type = SOCK_STREAM,
			.udiag_state = TCP_FIN_WAIT1
		}
	};
	struct msg *msg;
	struct nlattr *nla;
	unsigned int msg_len;
	long rc;

	/* fetch fail: len < sizeof(struct nlattr) */
	msg_len = NLMSG_SPACE(sizeof(msg->udm)) + 2;
	msg = tail_memdup(&c_msg, msg_len);
	memcpy(&msg->nlh.nlmsg_len, &msg_len, sizeof(msg_len));
	nla = NLMSG_ATTR(msg, sizeof(msg->udm));
	memcpy(nla, "12", 2);
	rc = sendto(fd, msg, msg_len, MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {udiag_family=AF_UNIX"
	       ", udiag_type=SOCK_STREAM, udiag_state=TCP_FIN_WAIT1"
	       ", udiag_ino=0, udiag_cookie=[0, 0]}, \"\\x31\\x32\"}, %u"
	       ", MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, msg_len, sprintrc(rc));

	/* fetch fail: short read */
	msg_len = NLMSG_SPACE(sizeof(msg->udm)) + sizeof(*nla);
	msg = tail_memdup(&c_msg, msg_len - 1);
	memcpy(&msg->nlh.nlmsg_len, &msg_len, sizeof(msg_len));
	rc = sendto(fd, msg, msg_len, MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {udiag_family=AF_UNIX"
	       ", udiag_type=SOCK_STREAM, udiag_state=TCP_FIN_WAIT1"
	       ", udiag_ino=0, udiag_cookie=[0, 0]}, %p}, %u"
	       ", MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, (void *) msg + NLMSG_SPACE(sizeof(msg->udm)),
	       msg_len, sprintrc(rc));

	/* print one struct nlattr */
	msg_len = NLMSG_SPACE(sizeof(msg->udm)) + sizeof(*nla);
	msg = tail_memdup(&c_msg, msg_len);
	memcpy(&msg->nlh.nlmsg_len, &msg_len, sizeof(msg_len));
	nla = NLMSG_ATTR(msg, sizeof(msg->udm));
	*nla = (struct nlattr) {
		.nla_len = sizeof(*nla),
		.nla_type = UNIX_DIAG_NAME
	};
	rc = sendto(fd, msg, msg_len, MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {udiag_family=AF_UNIX"
	       ", udiag_type=SOCK_STREAM, udiag_state=TCP_FIN_WAIT1"
	       ", udiag_ino=0, udiag_cookie=[0, 0]}, {nla_len=%u"
	       ", nla_type=UNIX_DIAG_NAME}}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, nla->nla_len, msg_len, sprintrc(rc));

	/* print one struct nlattr with nla_len out of msg_len bounds */
	nla->nla_len += 8;
	rc = sendto(fd, msg, msg_len, MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {udiag_family=AF_UNIX"
	       ", udiag_type=SOCK_STREAM, udiag_state=TCP_FIN_WAIT1"
	       ", udiag_ino=0, udiag_cookie=[0, 0]}, {nla_len=%u"
	       ", nla_type=UNIX_DIAG_NAME}}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, nla->nla_len, msg_len, sprintrc(rc));

	/* print one struct nlattr and some data */
	msg_len = NLMSG_SPACE(sizeof(msg->udm)) + NLA_HDRLEN + 4;
	msg = tail_memdup(&c_msg, msg_len);
	memcpy(&msg->nlh.nlmsg_len, &msg_len, sizeof(msg_len));
	nla = NLMSG_ATTR(msg, sizeof(msg->udm));
	*nla = (struct nlattr) {
		.nla_len = NLA_HDRLEN + 4,
		.nla_type = UNIX_DIAG_FIRST_UNUSED
	};
	memcpy(RTA_DATA(nla), "1234", 4);
	rc = sendto(fd, msg, msg_len, MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {udiag_family=AF_UNIX"
	       ", udiag_type=SOCK_STREAM, udiag_state=TCP_FIN_WAIT1"
	       ", udiag_ino=0, udiag_cookie=[0, 0]}, {{nla_len=%u"
	       ", nla_type=%#x /* UNIX_DIAG_??? */}"
	       ", \"\\x31\\x32\\x33\\x34\"}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, nla->nla_len, UNIX_DIAG_FIRST_UNUSED,
	       msg_len, sprintrc(rc));

	/* print one struct nlattr and fetch fail second struct nlattr */
	msg_len = NLMSG_SPACE(sizeof(msg->udm)) + NLA_HDRLEN + 2;
	msg = tail_memdup(&c_msg, msg_len);
	memcpy(&msg->nlh.nlmsg_len, &msg_len, sizeof(msg_len));
	nla = NLMSG_ATTR(msg, sizeof(msg->udm));
	SET_STRUCT(struct nlattr, nla,
		.nla_len = NLA_HDRLEN,
		.nla_type = UNIX_DIAG_NAME
	);
	memcpy(nla + 1, "12", 2);
	rc = sendto(fd, msg, msg_len, MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {udiag_family=AF_UNIX"
	       ", udiag_type=SOCK_STREAM, udiag_state=TCP_FIN_WAIT1"
	       ", udiag_ino=0, udiag_cookie=[0, 0]}, [{nla_len=%u"
	       ", nla_type=UNIX_DIAG_NAME}, \"\\x31\\x32\"]}, %u"
	       ", MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, NLA_HDRLEN, msg_len, sprintrc(rc));

	/* print one struct nlattr and short read of second struct nlattr */
	msg_len = NLMSG_SPACE(sizeof(msg->udm)) + NLA_HDRLEN * 2;
	msg = tail_memdup(&c_msg, msg_len - 1);
	memcpy(&msg->nlh.nlmsg_len, &msg_len, sizeof(msg_len));
	nla = NLMSG_ATTR(msg, sizeof(msg->udm));
	SET_STRUCT(struct nlattr, nla,
		.nla_len = NLA_HDRLEN,
		.nla_type = UNIX_DIAG_NAME
	);
	rc = sendto(fd, msg, msg_len, MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {udiag_family=AF_UNIX"
	       ", udiag_type=SOCK_STREAM, udiag_state=TCP_FIN_WAIT1"
	       ", udiag_ino=0, udiag_cookie=[0, 0]}, [{nla_len=%u"
	       ", nla_type=UNIX_DIAG_NAME}, ... /* %p */]}, %u"
	       ", MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, NLA_HDRLEN, nla + 1, msg_len, sprintrc(rc));

	/* print two struct nlattr */
	msg_len = NLMSG_SPACE(sizeof(msg->udm)) + NLA_HDRLEN * 2;
	msg = tail_memdup(&c_msg, msg_len);
	memcpy(&msg->nlh.nlmsg_len, &msg_len, sizeof(msg_len));
	nla = NLMSG_ATTR(msg, sizeof(msg->udm));
	*nla = (struct nlattr) {
		.nla_len = NLA_HDRLEN,
		.nla_type = UNIX_DIAG_NAME
	};
	*(nla + 1) = (struct nlattr) {
		.nla_len = NLA_HDRLEN,
		.nla_type = UNIX_DIAG_PEER
	};
	rc = sendto(fd, msg, msg_len, MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {udiag_family=AF_UNIX"
	       ", udiag_type=SOCK_STREAM, udiag_state=TCP_FIN_WAIT1"
	       ", udiag_ino=0, udiag_cookie=[0, 0]}, [{nla_len=%u"
	       ", nla_type=UNIX_DIAG_NAME}, {nla_len=%u"
	       ", nla_type=UNIX_DIAG_PEER}]}, %u"
	       ", MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, nla->nla_len, nla->nla_len,
	       msg_len, sprintrc(rc));

	/* print first nlattr only when its nla_len is less than NLA_HDRLEN */
	nla->nla_len = NLA_HDRLEN - 1;
	rc = sendto(fd, msg, msg_len, MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {udiag_family=AF_UNIX"
	       ", udiag_type=SOCK_STREAM, udiag_state=TCP_FIN_WAIT1"
	       ", udiag_ino=0, udiag_cookie=[0, 0]}, {nla_len=%u"
	       ", nla_type=UNIX_DIAG_NAME}}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, nla->nla_len, msg_len, sprintrc(rc));

	/* unrecognized attribute data, abbreviated output */
#define ABBREV_LEN (DEFAULT_STRLEN + 1)
	msg_len = NLMSG_SPACE(sizeof(msg->udm)) + NLA_HDRLEN * ABBREV_LEN * 2;
	msg = tail_alloc(msg_len);
	memcpy(msg, &c_msg, sizeof(c_msg));
	msg->nlh.nlmsg_len = msg_len;
	unsigned int i;
	nla = NLMSG_ATTR(msg, sizeof(msg->udm));
	for (i = 0; i < ABBREV_LEN; ++i) {
		nla[i * 2] = (struct nlattr) {
			.nla_len = NLA_HDRLEN * 2 - 1,
			.nla_type = UNIX_DIAG_FIRST_UNUSED + i
		};
		fill_memory_ex(&nla[i * 2 + 1], NLA_HDRLEN,
			       '0' + i, '~' - '0' - i);
	}

	rc = sendto(fd, msg, msg_len, MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}"
	       ", {udiag_family=AF_UNIX, udiag_type=SOCK_STREAM"
	       ", udiag_state=TCP_FIN_WAIT1, udiag_ino=0"
	       ", udiag_cookie=[0, 0]}, [",
	       fd, msg_len);
	for (i = 0; i < DEFAULT_STRLEN; ++i) {
		if (i)
			printf(", ");
		printf("{{nla_len=%u, nla_type=%#x /* UNIX_DIAG_??? */}, ",
		       nla->nla_len, UNIX_DIAG_FIRST_UNUSED + i);
		print_quoted_hex(&nla[i * 2 + 1], NLA_HDRLEN - 1);
		printf("}");
	}
	printf(", ...]}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       msg_len, sprintrc(rc));
}

static void
test_nla_type(const int fd)
{
	static const struct msg {
		struct nlmsghdr nlh;
		struct unix_diag_msg udm;
	} c_msg = {
		.nlh = {
			.nlmsg_len = sizeof(struct msg),
			.nlmsg_type = SOCK_DIAG_BY_FAMILY,
			.nlmsg_flags = NLM_F_DUMP
		},
		.udm = {
			.udiag_family = AF_UNIX,
			.udiag_type = SOCK_STREAM,
			.udiag_state = TCP_FIN_WAIT1
		}
	};
	struct msg *msg;
	struct nlattr *nla;
	unsigned int msg_len;
	long rc;

	msg_len = NLMSG_SPACE(sizeof(msg->udm)) + sizeof(*nla);
	msg = tail_memdup(&c_msg, msg_len);
	memcpy(&msg->nlh.nlmsg_len, &msg_len, sizeof(msg_len));
	nla = NLMSG_ATTR(msg, sizeof(msg->udm));
	*nla = (struct nlattr) {
		.nla_len = sizeof(*nla),
		.nla_type = NLA_F_NESTED | UNIX_DIAG_NAME
	};
	rc = sendto(fd, msg, msg_len, MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {udiag_family=AF_UNIX"
	       ", udiag_type=SOCK_STREAM, udiag_state=TCP_FIN_WAIT1"
	       ", udiag_ino=0, udiag_cookie=[0, 0]}, {nla_len=%u"
	       ", nla_type=NLA_F_NESTED|UNIX_DIAG_NAME}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, nla->nla_len, msg_len, sprintrc(rc));

	nla->nla_type = NLA_F_NET_BYTEORDER | UNIX_DIAG_NAME;
	rc = sendto(fd, msg, msg_len, MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {udiag_family=AF_UNIX"
	       ", udiag_type=SOCK_STREAM, udiag_state=TCP_FIN_WAIT1"
	       ", udiag_ino=0, udiag_cookie=[0, 0]}, {nla_len=%u"
	       ", nla_type=NLA_F_NET_BYTEORDER|UNIX_DIAG_NAME}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, nla->nla_len, msg_len, sprintrc(rc));

	nla->nla_type = NLA_F_NESTED | NLA_F_NET_BYTEORDER | UNIX_DIAG_NAME;
	rc = sendto(fd, msg, msg_len, MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {udiag_family=AF_UNIX"
	       ", udiag_type=SOCK_STREAM, udiag_state=TCP_FIN_WAIT1"
	       ", udiag_ino=0, udiag_cookie=[0, 0]}, {nla_len=%u"
	       ", nla_type=NLA_F_NESTED|NLA_F_NET_BYTEORDER|UNIX_DIAG_NAME}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, nla->nla_len, msg_len, sprintrc(rc));

	nla->nla_type = NLA_F_NESTED | (UNIX_DIAG_FIRST_UNUSED);
	rc = sendto(fd, msg, msg->nlh.nlmsg_len, MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {udiag_family=AF_UNIX"
	       ", udiag_type=SOCK_STREAM, udiag_state=TCP_FIN_WAIT1"
	       ", udiag_ino=0, udiag_cookie=[0, 0]}, {nla_len=%u"
	       ", nla_type=NLA_F_NESTED|%#x /* UNIX_DIAG_??? */}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg->nlh.nlmsg_len, nla->nla_len, UNIX_DIAG_FIRST_UNUSED,
	       msg->nlh.nlmsg_len, sprintrc(rc));
}

int main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	const int fd = create_nl_socket(NETLINK_SOCK_DIAG);

	test_nlattr(fd);
	test_nla_type(fd);

	puts("+++ exited with 0 +++");

	return 0;
}
