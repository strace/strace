/*
 * Copyright (c) 2016 Fabien Siron <fabien.siron@epita.fr>
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include "test_netlink.h"
#include <linux/if_ether.h>
#include <linux/inet_diag.h>
#include <linux/netlink_diag.h>
#include <linux/packet_diag.h>
#ifdef AF_SMC
# include <linux/smc_diag.h>
#endif
#include <linux/sock_diag.h>
#include <linux/unix_diag.h>

#define SMC_ACTIVE 1

#define TEST_SOCK_DIAG(fd_, nlh0_,					\
		       family_, type_, flags_,				\
		       obj_, print_family_, ...)			\
									\
	do {								\
		/* family only */					\
		uint8_t family = (family_);				\
		TEST_NETLINK_((fd_), (nlh0_),				\
			      type_, #type_,				\
			      flags_, #flags_,				\
			      sizeof(family), &family, sizeof(family),	\
			      printf("{family=%s}", #family_));		\
									\
		/* family and string */					\
		char buf[sizeof(family) + 4];				\
		memcpy(buf, &family, sizeof(family));			\
		memcpy(buf + sizeof(family), "1234", 4);		\
		TEST_NETLINK_((fd_), (nlh0_),				\
			      type_, #type_,				\
			      flags_, #flags_,				\
			      sizeof(buf), buf, sizeof(buf),		\
			      (print_family_);				\
			      printf(", ...}"));			\
									\
		/* sizeof(obj_) */					\
		TEST_NETLINK_((fd_), (nlh0_),				\
			      type_, #type_,				\
			      flags_, #flags_,				\
			      sizeof(obj_), &(obj_), sizeof(obj_),	\
			      (print_family_);				\
			      __VA_ARGS__);				\
									\
		/* short read of sizeof(obj_) */			\
		TEST_NETLINK_((fd_), (nlh0_),				\
			      type_, #type_,				\
			      flags_, #flags_,				\
			      sizeof(obj_), &(obj_), sizeof(obj_) - 1,	\
			      (print_family_);				\
			      printf(", %p}",				\
				     NLMSG_DATA(TEST_NETLINK_nlh) + 1));\
	} while (0)

static void
test_nlmsg_type(const int fd)
{
	long rc;
	struct nlmsghdr nlh = {
		.nlmsg_len = sizeof(nlh),
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_REQUEST,
	};

	rc = sendto(fd, &nlh, sizeof(nlh), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {nlmsg_len=%u, nlmsg_type=SOCK_DIAG_BY_FAMILY"
	       ", nlmsg_flags=NLM_F_REQUEST, nlmsg_seq=0, nlmsg_pid=0}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh.nlmsg_len, (unsigned) sizeof(nlh), sprintrc(rc));
}

static void
test_nlmsg_flags(const int fd)
{
	long rc;
	struct nlmsghdr nlh = {
		.nlmsg_len = sizeof(nlh),
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP,
	};

	rc = sendto(fd, &nlh, sizeof(nlh), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {nlmsg_len=%u, nlmsg_type=SOCK_DIAG_BY_FAMILY"
	       ", nlmsg_flags=NLM_F_REQUEST|NLM_F_DUMP, nlmsg_seq=0"
	       ", nlmsg_pid=0}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh.nlmsg_len, (unsigned) sizeof(nlh), sprintrc(rc));
}

static void
test_odd_family_req(const int fd)
{
	uint8_t family = 0;
	char buf[sizeof(family) + 4];
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(buf));

	/* unspecified family only */
	TEST_NETLINK(fd, nlh0,
		     SOCK_DIAG_BY_FAMILY,
		     NLM_F_REQUEST,
		     sizeof(family), &family, sizeof(family),
		     printf("{family=AF_UNSPEC}"));

	/* unknown family only */
	family = 0xff;
	TEST_NETLINK(fd, nlh0,
		     SOCK_DIAG_BY_FAMILY,
		     NLM_F_REQUEST,
		     sizeof(family), &family, sizeof(family),
		     printf("{family=%#x /* AF_??? */}", family));

	/* short read of family */
	TEST_NETLINK(fd, nlh0,
		     SOCK_DIAG_BY_FAMILY,
		     NLM_F_REQUEST,
		     sizeof(family), &family, sizeof(family) - 1,
		     printf("%p", NLMSG_DATA(TEST_NETLINK_nlh)));

	/* unspecified family and string */
	family = 0;
	memcpy(buf, &family, sizeof(family));
	memcpy(buf + sizeof(family), "1234", 4);
	TEST_NETLINK(fd, nlh0,
		     SOCK_DIAG_BY_FAMILY,
		     NLM_F_REQUEST,
		     sizeof(buf), buf, sizeof(buf),
		     printf("{family=AF_UNSPEC, data=\"\\x31\\x32\\x33\\x34\"}"));

	/* unknown family and string */
	family = 0xfd;
	memcpy(buf, &family, sizeof(family));
	TEST_NETLINK(fd, nlh0,
		     SOCK_DIAG_BY_FAMILY,
		     NLM_F_REQUEST,
		     sizeof(buf), buf, sizeof(buf),
		     printf("{family=%#x /* AF_??? */"
			    ", data=\"\\x31\\x32\\x33\\x34\"}", family));
}

static void
test_odd_family_msg(const int fd)
{
	uint8_t family = 0;
	char buf[sizeof(family) + 4];
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(buf));

	/* unspecified family only */
	TEST_NETLINK(fd, nlh0,
		     SOCK_DIAG_BY_FAMILY, NLM_F_DUMP,
		     sizeof(family), &family, sizeof(family),
		     printf("{family=AF_UNSPEC}"));

	/* unknown family only */
	family = 0xff;
	TEST_NETLINK(fd, nlh0,
		     SOCK_DIAG_BY_FAMILY, NLM_F_DUMP,
		     sizeof(family), &family, sizeof(family),
		     printf("{family=%#x /* AF_??? */}", family));

	/* short read of family */
	TEST_NETLINK(fd, nlh0,
		     SOCK_DIAG_BY_FAMILY, NLM_F_DUMP,
		     sizeof(family), &family, sizeof(family) - 1,
		     printf("%p", NLMSG_DATA(TEST_NETLINK_nlh)));

	/* unspecified family and string */
	family = 0;
	memcpy(buf, &family, sizeof(family));
	memcpy(buf + sizeof(family), "1234", 4);
	TEST_NETLINK(fd, nlh0,
		     SOCK_DIAG_BY_FAMILY, NLM_F_DUMP,
		     sizeof(buf), buf, sizeof(buf),
		     printf("{family=AF_UNSPEC, data=\"\\x31\\x32\\x33\\x34\"}"));

	/* unknown family and string */
	family = 0xfd;
	memcpy(buf, &family, sizeof(family));
	TEST_NETLINK(fd, nlh0,
		     SOCK_DIAG_BY_FAMILY, NLM_F_DUMP,
		     sizeof(buf), buf, sizeof(buf),
		     printf("{family=%#x /* AF_??? */"
			    ", data=\"\\x31\\x32\\x33\\x34\"}", family));
}

static void
test_unix_diag_req(const int fd)
{
	static const struct unix_diag_req req = {
		.sdiag_family = AF_UNIX,
		.sdiag_protocol = 253,
		.udiag_states = 1 << TCP_ESTABLISHED | 1 << TCP_LISTEN,
		.udiag_ino = 0xfacefeed,
		.udiag_show = UDIAG_SHOW_NAME,
		.udiag_cookie = { 0xdeadbeef, 0xbadc0ded }
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(req));
	TEST_SOCK_DIAG(fd, nlh0, AF_UNIX,
		       SOCK_DIAG_BY_FAMILY, NLM_F_REQUEST, req,
		       printf("{sdiag_family=AF_UNIX"),
		       printf(", ");
		       PRINT_FIELD_U(req, sdiag_protocol);
		       printf(", udiag_states=1<<TCP_ESTABLISHED|1<<TCP_LISTEN");
		       printf(", ");
		       PRINT_FIELD_U(req, udiag_ino);
		       printf(", udiag_show=UDIAG_SHOW_NAME");
		       printf(", ");
		       PRINT_FIELD_COOKIE(req, udiag_cookie);
		       printf("}"));
}

static void
test_unix_diag_msg(const int fd)
{
	static const struct unix_diag_msg msg = {
		.udiag_family = AF_UNIX,
		.udiag_type = SOCK_STREAM,
		.udiag_state = TCP_FIN_WAIT1,
		.udiag_ino = 0xfacefeed,
		.udiag_cookie = { 0xdeadbeef, 0xbadc0ded }
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(msg));
	TEST_SOCK_DIAG(fd, nlh0, AF_UNIX,
		       SOCK_DIAG_BY_FAMILY, NLM_F_DUMP, msg,
		       printf("{udiag_family=AF_UNIX"),
		       printf(", udiag_type=SOCK_STREAM"
			      ", udiag_state=TCP_FIN_WAIT1");
		       printf(", ");
		       PRINT_FIELD_U(msg, udiag_ino);
		       printf(", ");
		       PRINT_FIELD_COOKIE(msg, udiag_cookie);
		       printf("}"));
}

static void
test_netlink_diag_req(const int fd)
{
	struct netlink_diag_req req = {
		.sdiag_family = AF_NETLINK,
		.sdiag_protocol = NDIAG_PROTO_ALL,
		.ndiag_ino = 0xfacefeed,
		.ndiag_show = NDIAG_SHOW_MEMINFO,
		.ndiag_cookie = { 0xdeadbeef, 0xbadc0ded }
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(req));
	TEST_SOCK_DIAG(fd, nlh0, AF_NETLINK,
		       SOCK_DIAG_BY_FAMILY, NLM_F_REQUEST, req,
		       printf("{sdiag_family=AF_NETLINK"),
		       printf(", sdiag_protocol=NDIAG_PROTO_ALL");
		       printf(", ");
		       PRINT_FIELD_U(req, ndiag_ino);
		       printf(", ndiag_show=NDIAG_SHOW_MEMINFO");
		       printf(", ");
		       PRINT_FIELD_COOKIE(req, ndiag_cookie);
		       printf("}"));

	req.sdiag_protocol = NETLINK_ROUTE;
	req.ndiag_show = NDIAG_SHOW_GROUPS;
	TEST_SOCK_DIAG(fd, nlh0, AF_NETLINK,
		       SOCK_DIAG_BY_FAMILY, NLM_F_REQUEST, req,
		       printf("{sdiag_family=AF_NETLINK"),
		       printf(", sdiag_protocol=NETLINK_ROUTE");
		       printf(", ");
		       PRINT_FIELD_U(req, ndiag_ino);
		       printf(", ndiag_show=NDIAG_SHOW_GROUPS");
		       printf(", ");
		       PRINT_FIELD_COOKIE(req, ndiag_cookie);
		       printf("}"));
}

static void
test_netlink_diag_msg(const int fd)
{
	static const struct netlink_diag_msg msg = {
		.ndiag_family = AF_NETLINK,
		.ndiag_type = SOCK_RAW,
		.ndiag_protocol = NETLINK_ROUTE,
		.ndiag_state = NETLINK_CONNECTED,
		.ndiag_portid = 0xbadc0ded,
		.ndiag_dst_portid = 0xdeadbeef,
		.ndiag_dst_group = 0xfacefeed,
		.ndiag_ino = 0xdaeefacd,
		.ndiag_cookie = { 0xbadc0ded, 0xdeadbeef }
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(msg));
	TEST_SOCK_DIAG(fd, nlh0, AF_NETLINK,
		       SOCK_DIAG_BY_FAMILY, NLM_F_DUMP, msg,
		       printf("{ndiag_family=AF_NETLINK"),
		       printf(", ndiag_type=SOCK_RAW"
			      ", ndiag_protocol=NETLINK_ROUTE"
			      ", ndiag_state=NETLINK_CONNECTED");
		       printf(", ");
		       PRINT_FIELD_U(msg, ndiag_portid);
		       printf(", ");
		       PRINT_FIELD_U(msg, ndiag_dst_portid);
		       printf(", ");
		       PRINT_FIELD_U(msg, ndiag_dst_group);
		       printf(", ");
		       PRINT_FIELD_U(msg, ndiag_ino);
		       printf(", ");
		       PRINT_FIELD_COOKIE(msg, ndiag_cookie);
		       printf("}"));
}

static void
test_packet_diag_req(const int fd)
{
	static const struct packet_diag_req req = {
		.sdiag_family = AF_PACKET,
		.sdiag_protocol = ETH_P_LOOP,
		.pdiag_ino = 0xfacefeed,
		.pdiag_show = PACKET_SHOW_INFO,
		.pdiag_cookie = { 0xdeadbeef, 0xbadc0ded }
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(req));
	TEST_SOCK_DIAG(fd, nlh0, AF_PACKET,
		       SOCK_DIAG_BY_FAMILY, NLM_F_REQUEST, req,
		       printf("{sdiag_family=AF_PACKET"),
		       printf(", sdiag_protocol=%#x", req.sdiag_protocol);
		       printf(", ");
		       PRINT_FIELD_U(req, pdiag_ino);
		       printf(", pdiag_show=PACKET_SHOW_INFO");
		       printf(", ");
		       PRINT_FIELD_COOKIE(req, pdiag_cookie);
		       printf("}"));
}

static void
test_packet_diag_msg(const int fd)
{
	static const struct packet_diag_msg msg = {
		.pdiag_family = AF_PACKET,
		.pdiag_type = SOCK_STREAM,
		.pdiag_num = 0x9100,
		.pdiag_ino = 0xfacefeed,
		.pdiag_cookie = { 0xdeadbeef, 0xbadc0ded }
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(msg));
	TEST_SOCK_DIAG(fd, nlh0, AF_PACKET,
		       SOCK_DIAG_BY_FAMILY, NLM_F_DUMP, msg,
		       printf("{pdiag_family=AF_PACKET"),
		       printf(", pdiag_type=SOCK_STREAM");
		       printf(", pdiag_num=ETH_P_QINQ1");
		       printf(", ");
		       PRINT_FIELD_U(msg, pdiag_ino);
		       printf(", ");
		       PRINT_FIELD_COOKIE(msg, pdiag_cookie);
		       printf("}"));
}

static void
test_inet_diag_sockid(const int fd)
{
	const char address[] = "12.34.56.78";
	const char address6[] = "12:34:56:78:90:ab:cd:ef";
	struct inet_diag_req_v2 req = {
		.sdiag_family = AF_INET,
		.idiag_ext = 1 << (INET_DIAG_CONG - 1),
		.sdiag_protocol = IPPROTO_TCP,
		.idiag_states = 1 << TCP_CLOSE,
		.id = {
			.idiag_sport = 0xfacd,
			.idiag_dport = 0xdead,
			.idiag_if = ifindex_lo(),
			.idiag_cookie = { 0xdeadbeef, 0xbadc0ded }
		},
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(req));

	if (!inet_pton(AF_INET, address, &req.id.idiag_src) ||
	    !inet_pton(AF_INET, address, &req.id.idiag_dst))
		perror_msg_and_skip("inet_pton");

	TEST_NETLINK(fd, nlh0,
		     SOCK_DIAG_BY_FAMILY, NLM_F_REQUEST,
		     sizeof(req), &req, sizeof(req),
		     printf("{sdiag_family=AF_INET"),
		     printf(", sdiag_protocol=IPPROTO_TCP"
			    ", idiag_ext=1<<(INET_DIAG_CONG-1)"
			    ", idiag_states=1<<TCP_CLOSE"
			    ", id={idiag_sport=htons(%u)"
			    ", idiag_dport=htons(%u)"
			    ", idiag_src=inet_addr(\"%s\")"
			    ", idiag_dst=inet_addr(\"%s\")",
			    ntohs(req.id.idiag_sport),
			    ntohs(req.id.idiag_dport),
			    address, address);
		     printf(", idiag_if=" IFINDEX_LO_STR);
		     printf(", ");
		     PRINT_FIELD_COOKIE(req.id, idiag_cookie);
		     printf("}}"));

	req.sdiag_family = AF_INET6;
	if (!inet_pton(AF_INET6, address6, &req.id.idiag_src) ||
	    !inet_pton(AF_INET6, address6, &req.id.idiag_dst))
		perror_msg_and_skip("inet_pton");

	TEST_NETLINK(fd, nlh0,
		     SOCK_DIAG_BY_FAMILY, NLM_F_REQUEST,
		     sizeof(req), &req, sizeof(req),
		     printf("{sdiag_family=AF_INET6"),
		     printf(", sdiag_protocol=IPPROTO_TCP"
			    ", idiag_ext=1<<(INET_DIAG_CONG-1)"
			    ", idiag_states=1<<TCP_CLOSE"
			    ", id={idiag_sport=htons(%u)"
			    ", idiag_dport=htons(%u)"
			    ", inet_pton(AF_INET6, \"%s\", &idiag_src)"
			    ", inet_pton(AF_INET6, \"%s\", &idiag_dst)",
			    ntohs(req.id.idiag_sport),
			    ntohs(req.id.idiag_dport),
			    address6, address6);
		     printf(", idiag_if=" IFINDEX_LO_STR);
		     printf(", ");
		     PRINT_FIELD_COOKIE(req.id, idiag_cookie);
		     printf("}}"));
}

static void
test_inet_diag_req(const int fd)
{
	const char address[] = "12.34.56.78";
	struct inet_diag_req req = {
		.idiag_family = AF_INET,
		.idiag_src_len = 0xde,
		.idiag_dst_len = 0xba,
		.idiag_ext = 1 << (INET_DIAG_TOS - 1),
		.id = {
			.idiag_sport = 0xdead,
			.idiag_dport = 0xadcd,
			.idiag_if = ifindex_lo(),
			.idiag_cookie = { 0xdeadbeef, 0xbadc0ded }
		},
		.idiag_states = 1 << TCP_LAST_ACK,
		.idiag_dbs = 0xfacefeed,
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(req));

	if (!inet_pton(AF_INET, address, &req.id.idiag_src) ||
	    !inet_pton(AF_INET, address, &req.id.idiag_dst))
		perror_msg_and_skip("inet_pton");

	TEST_SOCK_DIAG(fd, nlh0, AF_INET,
		       TCPDIAG_GETSOCK, NLM_F_REQUEST, req,
		       printf("{idiag_family=AF_INET"),
		       printf(", ");
		       PRINT_FIELD_U(req, idiag_src_len);
		       printf(", ");
		       PRINT_FIELD_U(req, idiag_dst_len);
		       printf(", idiag_ext=1<<(INET_DIAG_TOS-1)");
		       printf(", id={idiag_sport=htons(%u)"
			      ", idiag_dport=htons(%u)"
			      ", idiag_src=inet_addr(\"%s\")"
			      ", idiag_dst=inet_addr(\"%s\")",
			      ntohs(req.id.idiag_sport),
			      ntohs(req.id.idiag_dport),
			      address, address);
		       printf(", idiag_if=" IFINDEX_LO_STR);
		       printf(", ");
		       PRINT_FIELD_COOKIE(req.id, idiag_cookie);
		       printf("}, idiag_states=1<<TCP_LAST_ACK");
		       printf(", ");
		       PRINT_FIELD_U(req, idiag_dbs);
		       printf("}"));
}

static void
test_inet_diag_req_v2(const int fd)
{
	const char address[] = "87.65.43.21";
	struct inet_diag_req_v2 req = {
		.sdiag_family = AF_INET,
		.idiag_ext = 1 << (INET_DIAG_CONG - 1),
		.sdiag_protocol = IPPROTO_TCP,
		.idiag_states = 1 << TCP_CLOSE,
		.id = {
			.idiag_sport = 0xfacd,
			.idiag_dport = 0xdead,
			.idiag_if = ifindex_lo(),
			.idiag_cookie = { 0xdeadbeef, 0xbadc0ded }
		},
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(req));

	if (!inet_pton(AF_INET, address, &req.id.idiag_src) ||
	    !inet_pton(AF_INET, address, &req.id.idiag_dst))
		perror_msg_and_skip("inet_pton");

	TEST_SOCK_DIAG(fd, nlh0, AF_INET,
		       SOCK_DIAG_BY_FAMILY, NLM_F_REQUEST, req,
		       printf("{sdiag_family=AF_INET"),
		       printf(", sdiag_protocol=IPPROTO_TCP"
			      ", idiag_ext=1<<(INET_DIAG_CONG-1)"
			      ", idiag_states=1<<TCP_CLOSE"
			      ", id={idiag_sport=htons(%u)"
			      ", idiag_dport=htons(%u)"
			      ", idiag_src=inet_addr(\"%s\")"
			      ", idiag_dst=inet_addr(\"%s\")",
			      ntohs(req.id.idiag_sport),
			      ntohs(req.id.idiag_dport),
			      address, address);
		       printf(", idiag_if=" IFINDEX_LO_STR);
		       printf(", ");
		       PRINT_FIELD_COOKIE(req.id, idiag_cookie);
		       printf("}}"));
}

static void
test_inet_diag_msg(const int fd)
{
	const char address[] = "11.22.33.44";
	struct inet_diag_msg msg = {
		.idiag_family = AF_INET,
		.idiag_state = TCP_LISTEN,
		.idiag_timer = 0xfa,
		.idiag_retrans = 0xde,
		.id = {
			.idiag_sport = 0xfacf,
			.idiag_dport = 0xdead,
			.idiag_if = ifindex_lo(),
			.idiag_cookie = { 0xdeadbeef, 0xbadc0ded }
		},
		.idiag_expires = 0xfacefeed,
		.idiag_rqueue = 0xdeadbeef,
		.idiag_wqueue = 0xadcdfafc,
		.idiag_uid = 0xdecefaeb,
		.idiag_inode = 0xbadc0ded,
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(msg));

	if (!inet_pton(AF_INET, address, &msg.id.idiag_src) ||
	    !inet_pton(AF_INET, address, &msg.id.idiag_dst))
		perror_msg_and_skip("inet_pton");

	TEST_SOCK_DIAG(fd, nlh0, AF_INET,
		       SOCK_DIAG_BY_FAMILY, NLM_F_DUMP, msg,
		       printf("{idiag_family=AF_INET"),
		       printf(", idiag_state=TCP_LISTEN");
		       printf(", ");
		       PRINT_FIELD_U(msg, idiag_timer);
		       printf(", ");
		       PRINT_FIELD_U(msg, idiag_retrans);
		       printf(", id={idiag_sport=htons(%u)"
			      ", idiag_dport=htons(%u)"
			      ", idiag_src=inet_addr(\"%s\")"
			      ", idiag_dst=inet_addr(\"%s\")",
			      ntohs(msg.id.idiag_sport),
			      ntohs(msg.id.idiag_dport),
			      address, address);
		       printf(", idiag_if=" IFINDEX_LO_STR);
		       printf(", ");
		       PRINT_FIELD_COOKIE(msg.id, idiag_cookie);
		       printf("}, ");
		       PRINT_FIELD_U(msg, idiag_expires);
		       printf(", ");
		       PRINT_FIELD_U(msg, idiag_rqueue);
		       printf(", ");
		       PRINT_FIELD_U(msg, idiag_wqueue);
		       printf(", ");
		       PRINT_FIELD_U(msg, idiag_uid);
		       printf(", ");
		       PRINT_FIELD_U(msg, idiag_inode);
		       printf("}"));
}

#ifdef AF_SMC
static void
test_smc_diag_req(const int fd)
{
	const char address[] = "43.21.56.78";
	struct smc_diag_req req = {
		.diag_family = AF_SMC,
		.diag_ext = 1 << (SMC_DIAG_CONNINFO - 1),
		.id = {
			.idiag_sport = 0xdead,
			.idiag_dport = 0xadcd,
			.idiag_if = ifindex_lo(),
			.idiag_cookie = { 0xdeadbeef, 0xbadc0ded },
		},
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(req));

	if (!inet_pton(AF_INET, address, &req.id.idiag_src) ||
	    !inet_pton(AF_INET, address, &req.id.idiag_dst))
		perror_msg_and_skip("inet_pton");

	TEST_SOCK_DIAG(fd, nlh0, AF_SMC,
		       SOCK_DIAG_BY_FAMILY, NLM_F_REQUEST, req,
		       printf("{diag_family=AF_SMC"),
		       printf(", diag_ext=1<<(SMC_DIAG_CONNINFO-1)");
		       printf(", id={idiag_sport=htons(%u)"
			      ", idiag_dport=htons(%u)"
			      ", idiag_src=inet_addr(\"%s\")"
			      ", idiag_dst=inet_addr(\"%s\")",
			      ntohs(req.id.idiag_sport),
			      ntohs(req.id.idiag_dport),
			      address, address);
		       printf(", idiag_if=" IFINDEX_LO_STR);
		       printf(", ");
		       PRINT_FIELD_COOKIE(req.id, idiag_cookie);
		       printf("}}"));
}

static void
test_smc_diag_msg(const int fd)
{
	const char address[] = "34.87.12.90";
	struct smc_diag_msg msg = {
		.diag_family = AF_SMC,
		.diag_state = SMC_ACTIVE,
		.diag_fallback = 0x1,
		.diag_shutdown = 0xba,
		.id = {
			.idiag_sport = 0xdead,
			.idiag_dport = 0xadcd,
			.idiag_if = ifindex_lo(),
			.idiag_cookie = { 0xdeadbeef, 0xbadc0ded },
		},
		.diag_uid = 0xadcdfafc,
		.diag_inode = 0xbadc0ded,
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(msg));

	if (!inet_pton(AF_INET, address, &msg.id.idiag_src) ||
	    !inet_pton(AF_INET, address, &msg.id.idiag_dst))
		perror_msg_and_skip("inet_pton");

	TEST_SOCK_DIAG(fd, nlh0, AF_SMC,
		       SOCK_DIAG_BY_FAMILY, NLM_F_DUMP, msg,
		       printf("{diag_family=AF_SMC"),
		       printf(", diag_state=SMC_ACTIVE");
		       printf(", diag_fallback=SMC_DIAG_MODE_FALLBACK_TCP");
		       printf(", ");
		       PRINT_FIELD_U(msg, diag_shutdown);
		       printf(", id={idiag_sport=htons(%u)"
			      ", idiag_dport=htons(%u)"
			      ", idiag_src=inet_addr(\"%s\")"
			      ", idiag_dst=inet_addr(\"%s\")",
			      ntohs(msg.id.idiag_sport),
			      ntohs(msg.id.idiag_dport),
			      address, address);
		       printf(", idiag_if=" IFINDEX_LO_STR);
		       printf(", ");
		       PRINT_FIELD_COOKIE(msg.id, idiag_cookie);
		       printf("}, ");
		       PRINT_FIELD_U(msg, diag_uid);
		       printf(", ");
		       PRINT_FIELD_U(msg, diag_inode);
		       printf("}"));
}
#endif

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	int fd = create_nl_socket(NETLINK_SOCK_DIAG);

	test_nlmsg_type(fd);
	test_nlmsg_flags(fd);
	test_odd_family_req(fd);
	test_odd_family_msg(fd);
	test_unix_diag_req(fd);
	test_unix_diag_msg(fd);
	test_netlink_diag_req(fd);
	test_netlink_diag_msg(fd);
	test_packet_diag_req(fd);
	test_packet_diag_msg(fd);
	test_inet_diag_sockid(fd);
	test_inet_diag_req(fd);
	test_inet_diag_req_v2(fd);
	test_inet_diag_msg(fd);
#ifdef AF_SMC
	test_smc_diag_req(fd);
	test_smc_diag_msg(fd);
#endif

	printf("+++ exited with 0 +++\n");

	return 0;
}
