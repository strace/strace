/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2022 The strace developers.
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
#include "test_netlink.h"
#include <linux/dcbnl.h>
#include <linux/fib_rules.h>
#include <linux/if_addr.h>
#include <linux/if_addrlabel.h>
#include <linux/if_arp.h>
#include <linux/if_bridge.h>
#include <linux/if_link.h>
#include <linux/ip.h>
#include <linux/neighbour.h>
#include <linux/netconf.h>
#include <linux/rtnetlink.h>
#include <linux/nexthop.h>

#define TEST_NL_ROUTE_(fd_, nlh0_, type_, type_str_, obj_, print_family_, ...) \
	do {								\
		/* family and string */					\
		TEST_NETLINK_((fd_), (nlh0_),				\
			      (type_), (type_str_),			\
			      NLM_F_REQUEST, "NLM_F_REQUEST",		\
			      sizeof(obj_) - 1,				\
			      &(obj_), sizeof(obj_) - 1,		\
			      (print_family_);				\
			      printf(", ...}"));			\
									\
		/* sizeof(obj_) */					\
		TEST_NETLINK_((fd_), (nlh0_),				\
			      (type_), (type_str_),			\
			      NLM_F_REQUEST, "NLM_F_REQUEST",		\
			      sizeof(obj_), &(obj_), sizeof(obj_),	\
			      (print_family_);				\
			       __VA_ARGS__);				\
									\
		/* short read of sizeof(obj_) */			\
		TEST_NETLINK_((fd_), (nlh0_),				\
			      (type_), (type_str_),			\
			      NLM_F_REQUEST, "NLM_F_REQUEST",		\
			      sizeof(obj_), &(obj_), sizeof(obj_) - 1,	\
			      (print_family_);				\
			      printf(", %p}",				\
				     NLMSG_DATA(TEST_NETLINK_nlh) + 1)); \
	} while (0)

#define TEST_NL_ROUTE(fd_, nlh0_, type_, obj_, print_family_, ...)	\
	TEST_NL_ROUTE_((fd_), (nlh0_), (type_), #type_, (obj_),		\
		       (print_family_), __VA_ARGS__)			\
	/* End of TEST_NL_ROUTE definition */

static void
test_nlmsg_type(const int fd)
{
	long rc;
	struct nlmsghdr nlh = {
		.nlmsg_len = sizeof(nlh),
		.nlmsg_type = RTM_GETLINK,
		.nlmsg_flags = NLM_F_REQUEST,
	};

	rc = sendto(fd, &nlh, sizeof(nlh), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {nlmsg_len=%u, nlmsg_type=RTM_GETLINK"
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
	};

	nlh.nlmsg_type = RTM_GETLINK;
	nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	rc = sendto(fd, &nlh, sizeof(nlh), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {nlmsg_len=%u, nlmsg_type=RTM_GETLINK"
	       ", nlmsg_flags=NLM_F_REQUEST|NLM_F_DUMP, nlmsg_seq=0"
	       ", nlmsg_pid=0}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh.nlmsg_len, (unsigned) sizeof(nlh), sprintrc(rc));

	nlh.nlmsg_type = RTM_DELACTION;
	nlh.nlmsg_flags = NLM_F_ROOT;
	rc = sendto(fd, &nlh, sizeof(nlh), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {nlmsg_len=%u, nlmsg_type=RTM_DELACTION"
	       ", nlmsg_flags=NLM_F_ROOT, nlmsg_seq=0, nlmsg_pid=0}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh.nlmsg_len, (unsigned) sizeof(nlh), sprintrc(rc));

	nlh.nlmsg_type = RTM_NEWLINK;
	nlh.nlmsg_flags = NLM_F_ECHO | NLM_F_REPLACE;
	rc = sendto(fd, &nlh, sizeof(nlh), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {nlmsg_len=%u, nlmsg_type=RTM_NEWLINK"
	       ", nlmsg_flags=NLM_F_ECHO|NLM_F_REPLACE, nlmsg_seq=0"
	       ", nlmsg_pid=0}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh.nlmsg_len, (unsigned) sizeof(nlh), sprintrc(rc));

	nlh.nlmsg_type = RTM_DELLINK;
	nlh.nlmsg_flags = NLM_F_NONREC | NLM_F_BULK;
	rc = sendto(fd, &nlh, sizeof(nlh), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {nlmsg_len=%u, nlmsg_type=RTM_DELLINK"
	       ", nlmsg_flags=NLM_F_NONREC|NLM_F_BULK, nlmsg_seq=0"
	       ", nlmsg_pid=0}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh.nlmsg_len, (unsigned) sizeof(nlh), sprintrc(rc));
}

static void
test_nlmsg_done(const int fd)
{
	const int num = 0xabcdefad;
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(num));

	TEST_NETLINK(fd, nlh0, NLMSG_DONE, NLM_F_REQUEST,
		     sizeof(num), &num, sizeof(num),
		     printf("%d", num));
}

static void
test_rtnl_unspec(const int fd)
{
	uint8_t family = 0;
	char buf[sizeof(family) + 4];
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(buf));

	/* unspecified family only */
	TEST_NETLINK_(fd, nlh0,
		      0xffff, "0xffff /* RTM_??? */",
		      NLM_F_REQUEST, "NLM_F_REQUEST",
		      sizeof(family), &family, sizeof(family),
		      printf("{family=AF_UNSPEC}"));

	/* unknown family only */
	family = 0xff;
	TEST_NETLINK_(fd, nlh0,
		      0xffff, "0xffff /* RTM_??? */",
		      NLM_F_REQUEST, "NLM_F_REQUEST",
		      sizeof(family), &family, sizeof(family),
		      printf("{family=0xff /* AF_??? */}"));

	/* short read of family */
	TEST_NETLINK_(fd, nlh0,
		      0xffff, "0xffff /* RTM_??? */",
		      NLM_F_REQUEST, "NLM_F_REQUEST",
		      sizeof(family), &family, sizeof(family) - 1,
		      printf("%p", NLMSG_DATA(TEST_NETLINK_nlh)));

	/* unspecified family and string */
	family = 0;
	memcpy(buf, &family, sizeof(family));
	memcpy(buf + sizeof(family), "1234", 4);
	TEST_NETLINK_(fd, nlh0,
		      0xffff, "0xffff /* RTM_??? */",
		      NLM_F_REQUEST, "NLM_F_REQUEST",
		      sizeof(buf), buf, sizeof(buf),
		      printf("{family=AF_UNSPEC, data=\"\\x31\\x32\\x33\\x34\"}"));

	/* unknown family and string */
	family = 0xfd;
	memcpy(buf, &family, sizeof(family));
	TEST_NETLINK_(fd, nlh0,
		      0xffff, "0xffff /* RTM_??? */",
		      NLM_F_REQUEST, "NLM_F_REQUEST",
		      sizeof(buf), buf, sizeof(buf),
		      printf("{family=%#x /* AF_??? */"
			     ", data=\"\\x31\\x32\\x33\\x34\"}", family));
}

static void
test_rtnl_unsupported_msg(const int fd, uint16_t msg, const char *str)
{
	char buf[64];
	char name[sizeof("0xffff /* RTM_??? */")];
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(buf));

	fill_memory(buf, sizeof(buf));
	buf[0] = AF_INET;

	if (!str)
		snprintf(name, sizeof(name), "%#hx /* RTM_??? */", msg);

	TEST_NETLINK_(fd, nlh0, msg, str ?: name,
		      NLM_F_REQUEST, "NLM_F_REQUEST",
		      1, buf, 1,
		      printf("{family=AF_INET}"));

	TEST_NETLINK_(fd, nlh0, msg, str ?: name,
		      NLM_F_REQUEST, "NLM_F_REQUEST",
		      sizeof(buf), buf, sizeof(buf),
		      printf("{family=AF_INET, data=");
		      print_quoted_hex(buf + 1, DEFAULT_STRLEN);
		      printf("...}"));
}

static void
test_rtnl_unknown_msg(const int fd, uint16_t msg)
{
	test_rtnl_unsupported_msg(fd, msg, NULL);
}

static void
test_rtnl_link(const int fd)
{
	static const struct strval32 types[] = {
		{ ARG_STR(RTM_NEWLINK) },
		{ ARG_STR(RTM_DELLINK) },
		{ ARG_STR(RTM_GETLINK) },
		{ ARG_STR(RTM_SETLINK) },
	};
	const struct ifinfomsg ifinfo = {
		.ifi_family = AF_UNIX,
		.ifi_type = ARPHRD_LOOPBACK,
		.ifi_index = ifindex_lo(),
		.ifi_flags = IFF_UP,
		.ifi_change = 0xfabcdeba
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(ifinfo));

	for (size_t i = 0; i < ARRAY_SIZE(types); i++) {
		TEST_NL_ROUTE_(fd, nlh0, types[i].val, types[i].str, ifinfo,
			       printf("{ifi_family=AF_UNIX"),
			       printf(", ifi_type=ARPHRD_LOOPBACK"
				      ", ifi_index=" IFINDEX_LO_STR
				      ", ifi_flags=IFF_UP");
			       printf(", ");
			       PRINT_FIELD_X(ifinfo, ifi_change);
			       printf("}"));
	}
}

static void
test_rtnl_addr(const int fd)
{
	static const struct strval32 types[] = {
		{ ARG_STR(RTM_NEWADDR) },
		{ ARG_STR(RTM_DELADDR) },
		{ ARG_STR(RTM_GETADDR) },
		{ ARG_STR(RTM_GETMULTICAST) },
		{ ARG_STR(RTM_GETANYCAST) },
	};
	const struct ifaddrmsg msg = {
		.ifa_family = AF_UNIX,
		.ifa_prefixlen = 0xde,
		.ifa_flags = IFA_F_SECONDARY,
		.ifa_scope = RT_SCOPE_UNIVERSE,
		.ifa_index = ifindex_lo()
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(msg));

	for (size_t i = 0; i < ARRAY_SIZE(types); i++) {
		TEST_NL_ROUTE_(fd, nlh0, types[i].val, types[i].str, msg,
			       printf("{ifa_family=AF_UNIX"),
			       printf(", ");
			       PRINT_FIELD_U(msg, ifa_prefixlen);
			       printf(", ifa_flags=IFA_F_SECONDARY"
				      ", ifa_scope=RT_SCOPE_UNIVERSE"
				      ", ifa_index=" IFINDEX_LO_STR);
			       printf("}"));
	}

	test_rtnl_unknown_msg(fd, RTM_NEWADDR + 3);
	test_rtnl_unknown_msg(fd, RTM_GETMULTICAST - 2);
	test_rtnl_unknown_msg(fd, RTM_GETMULTICAST - 1);
	test_rtnl_unknown_msg(fd, RTM_GETMULTICAST + 1);
	test_rtnl_unknown_msg(fd, RTM_GETANYCAST - 2);
	test_rtnl_unknown_msg(fd, RTM_GETANYCAST - 1);
	test_rtnl_unknown_msg(fd, RTM_GETANYCAST + 1);
}

static void
test_rtnl_route(const int fd)
{
	static const struct strval32 types[] = {
		{ ARG_STR(RTM_NEWROUTE) },
		{ ARG_STR(RTM_DELROUTE) },
		{ ARG_STR(RTM_GETROUTE) },
	};
	static const struct rtmsg msg = {
		.rtm_family = AF_UNIX,
		.rtm_dst_len = 0xaf,
		.rtm_src_len = 0xda,
		.rtm_tos = IPTOS_LOWDELAY,
		.rtm_table = RT_TABLE_DEFAULT,
		.rtm_protocol = RTPROT_KERNEL,
		.rtm_scope = RT_SCOPE_UNIVERSE,
		.rtm_type = RTN_LOCAL,
		.rtm_flags = RTM_F_NOTIFY
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(msg));

	for (size_t i = 0; i < ARRAY_SIZE(types); i++) {
		TEST_NL_ROUTE_(fd, nlh0, types[i].val, types[i].str, msg,
			       printf("{rtm_family=AF_UNIX"),
			       printf(", ");
			       PRINT_FIELD_U(msg, rtm_dst_len);
			       printf(", ");
			       PRINT_FIELD_U(msg, rtm_src_len);
			       printf(", rtm_tos=IPTOS_LOWDELAY"
				      ", rtm_table=RT_TABLE_DEFAULT"
				      ", rtm_protocol=RTPROT_KERNEL"
				      ", rtm_scope=RT_SCOPE_UNIVERSE"
				      ", rtm_type=RTN_LOCAL"
				      ", rtm_flags=RTM_F_NOTIFY}"));
	}

	test_rtnl_unknown_msg(fd, RTM_NEWROUTE + 3);
}

static void
test_rtnl_rule(const int fd)
{
	static const struct strval32 types[] = {
		{ ARG_STR(RTM_NEWRULE) },
		{ ARG_STR(RTM_DELRULE) },
		{ ARG_STR(RTM_GETRULE) },
	};
	struct rtmsg msg = {
		.rtm_family = AF_UNIX,
		.rtm_dst_len = 0xaf,
		.rtm_src_len = 0xda,
		.rtm_tos = IPTOS_LOWDELAY,
		.rtm_table = RT_TABLE_UNSPEC,
		.rtm_type = FR_ACT_TO_TBL,
		.rtm_flags = FIB_RULE_INVERT
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(msg));

	for (size_t i = 0; i < ARRAY_SIZE(types); i++) {
		TEST_NL_ROUTE_(fd, nlh0, types[i].val, types[i].str, msg,
			       printf("{family=AF_UNIX"),
			       printf(", dst_len=%u, src_len=%u"
				      ", tos=IPTOS_LOWDELAY"
				      ", table=RT_TABLE_UNSPEC"
				      ", action=FR_ACT_TO_TBL"
				      ", flags=FIB_RULE_INVERT}",
				      msg.rtm_dst_len,
				      msg.rtm_src_len));
	}

	test_rtnl_unknown_msg(fd, RTM_NEWRULE + 3);
}

static void
test_rtnl_neigh(const int fd)
{
	static const struct strval32 types[] = {
		{ ARG_STR(RTM_NEWNEIGH) },
		{ ARG_STR(RTM_DELNEIGH) },
		{ ARG_STR(RTM_GETNEIGH) },
	};
	const struct ndmsg msg = {
		.ndm_family = AF_UNIX,
		.ndm_ifindex = ifindex_lo(),
		.ndm_state = NUD_PERMANENT,
		.ndm_flags = NTF_PROXY,
		.ndm_type = RTN_UNSPEC
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(msg));

	for (size_t i = 0; i < ARRAY_SIZE(types); i++) {
		TEST_NL_ROUTE_(fd, nlh0, types[i].val, types[i].str, msg,
			       printf("{ndm_family=AF_UNIX"),
			       printf(", ndm_ifindex=" IFINDEX_LO_STR
				      ", ndm_state=NUD_PERMANENT"
				      ", ndm_flags=NTF_PROXY"
				      ", ndm_type=RTN_UNSPEC}"));
	}

	test_rtnl_unknown_msg(fd, RTM_NEWNEIGH + 3);
}

static void
test_rtnl_neightbl(const int fd)
{
	static const struct strval32 types[] = {
		{ ARG_STR(RTM_NEWNEIGHTBL) },
		{ ARG_STR(RTM_GETNEIGHTBL) },
		{ ARG_STR(RTM_SETNEIGHTBL) },
	};
	static const struct ndtmsg msg = {
		.ndtm_family = AF_NETLINK
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(msg));

	for (size_t i = 0; i < ARRAY_SIZE(types); i++) {
		TEST_NETLINK_(fd, nlh0, types[i].val, types[i].str,
			      NLM_F_REQUEST, "NLM_F_REQUEST",
			      sizeof(msg), &msg, sizeof(msg),
			      printf("{ndtm_family=AF_NETLINK}"));
	}

	test_rtnl_unknown_msg(fd, RTM_NEWNEIGHTBL + 1);
}

static void
test_rtnl_tc(const int fd)
{
	static const struct strval32 types[] = {
		{ ARG_STR(RTM_NEWQDISC) },
		{ ARG_STR(RTM_DELQDISC) },
		{ ARG_STR(RTM_GETQDISC) },
		{ ARG_STR(RTM_NEWTCLASS) },
		{ ARG_STR(RTM_DELTCLASS) },
		{ ARG_STR(RTM_GETTCLASS) },
		{ ARG_STR(RTM_NEWTFILTER) },
		{ ARG_STR(RTM_DELTFILTER) },
		{ ARG_STR(RTM_GETTFILTER) },
		{ ARG_STR(RTM_NEWCHAIN) },
		{ ARG_STR(RTM_DELCHAIN) },
		{ ARG_STR(RTM_GETCHAIN) },
	};
	const struct tcmsg msg = {
		.tcm_family = AF_UNIX,
		.tcm_ifindex = ifindex_lo(),
		.tcm_handle = 0xfadcdafb,
		.tcm_parent = 0xafbcadab,
		.tcm_info = 0xbcaedafa
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(msg));

	for (size_t i = 0; i < ARRAY_SIZE(types); i++) {
		TEST_NL_ROUTE_(fd, nlh0, types[i].val, types[i].str, msg,
			       printf("{tcm_family=AF_UNIX"),
			       printf(", tcm_ifindex=" IFINDEX_LO_STR);
			       printf(", ");
			       PRINT_FIELD_U(msg, tcm_handle);
			       printf(", ");
			       PRINT_FIELD_U(msg, tcm_parent);
			       printf(", ");
			       PRINT_FIELD_U(msg, tcm_info);
			       printf("}"));
	}

	test_rtnl_unknown_msg(fd, RTM_NEWQDISC + 3);
	test_rtnl_unknown_msg(fd, RTM_NEWTCLASS + 3);
	test_rtnl_unknown_msg(fd, RTM_NEWTFILTER + 3);
	test_rtnl_unknown_msg(fd, RTM_NEWCHAIN + 3);
}

static void
test_rtnl_tca(const int fd)
{
	static const struct strval32 types[] = {
		{ ARG_STR(RTM_NEWACTION) },
		{ ARG_STR(RTM_DELACTION) },
		{ ARG_STR(RTM_GETACTION) },
	};
	struct tcamsg msg = {
		.tca_family = AF_INET
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(msg));

	for (size_t i = 0; i < ARRAY_SIZE(types); i++) {
		TEST_NETLINK_(fd, nlh0, types[i].val, types[i].str,
			      NLM_F_REQUEST, "NLM_F_REQUEST",
			      sizeof(msg), &msg, sizeof(msg),
			      printf("{tca_family=AF_INET}"));
	}

	test_rtnl_unknown_msg(fd, RTM_NEWACTION + 3);
}

static void
test_rtnl_addrlabel(const int fd)
{
	static const struct strval32 types[] = {
		{ ARG_STR(RTM_NEWADDRLABEL) },
		{ ARG_STR(RTM_DELADDRLABEL) },
		{ ARG_STR(RTM_GETADDRLABEL) },
	};
	const struct ifaddrlblmsg msg = {
		.ifal_family = AF_UNIX,
		.ifal_prefixlen = 0xaf,
		.ifal_flags = 0xbd,
		.ifal_index = ifindex_lo(),
		.ifal_seq = 0xfadcdafb
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(msg));

	for (size_t i = 0; i < ARRAY_SIZE(types); i++) {
		TEST_NL_ROUTE_(fd, nlh0, types[i].val, types[i].str, msg,
			       printf("{ifal_family=AF_UNIX"),
			       printf(", ");
			       PRINT_FIELD_U(msg, ifal_prefixlen);
			       printf(", ");
			       PRINT_FIELD_U(msg, ifal_flags);
			       printf(", ifal_index=" IFINDEX_LO_STR);
			       printf(", ");
			       PRINT_FIELD_U(msg, ifal_seq);
			       printf("}"));
	}

	test_rtnl_unknown_msg(fd, RTM_NEWADDRLABEL + 3);
}

static void
test_rtnl_dcb(const int fd)
{
	static const struct strval32 types[] = {
		{ ARG_STR(RTM_GETDCB) },
		{ ARG_STR(RTM_SETDCB) },
	};
	static const struct dcbmsg msg = {
		.dcb_family = AF_UNIX,
		.cmd = DCB_CMD_UNDEFINED
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(msg));

	for (size_t i = 0; i < ARRAY_SIZE(types); i++) {
		TEST_NL_ROUTE_(fd, nlh0, types[i].val, types[i].str, msg,
			       printf("{dcb_family=AF_UNIX"),
			       printf(", cmd=DCB_CMD_UNDEFINED}"));
	}

	test_rtnl_unknown_msg(fd, RTM_GETDCB - 2);
	test_rtnl_unknown_msg(fd, RTM_GETDCB - 1);
}

static void
test_rtnl_netconf(const int fd)
{
	static const struct strval32 types[] = {
		{ ARG_STR(RTM_NEWNETCONF) },
		{ ARG_STR(RTM_DELNETCONF) },
		{ ARG_STR(RTM_GETNETCONF) },
	};
	static const struct netconfmsg msg = {
		.ncm_family = AF_INET
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(msg));

	for (size_t i = 0; i < ARRAY_SIZE(types); i++) {
		TEST_NETLINK_(fd, nlh0, types[i].val, types[i].str,
			      NLM_F_REQUEST, "NLM_F_REQUEST",
			      sizeof(msg), &msg, sizeof(msg),
			      printf("{ncm_family=AF_INET}"));
	}

	test_rtnl_unknown_msg(fd, RTM_NEWNETCONF + 3);
}

static void
test_rtnl_mdb(const int fd)
{
	static const struct strval32 types[] = {
		{ ARG_STR(RTM_NEWMDB) },
		{ ARG_STR(RTM_DELMDB) },
		{ ARG_STR(RTM_GETMDB) },
	};
	const struct br_port_msg msg = {
		.family = AF_UNIX,
		.ifindex = ifindex_lo()
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(msg));

	for (size_t i = 0; i < ARRAY_SIZE(types); i++) {
		TEST_NL_ROUTE_(fd, nlh0, types[i].val, types[i].str, msg,
			      printf("{family=AF_UNIX"),
			      printf(", ifindex=" IFINDEX_LO_STR "}"));
	}

	test_rtnl_unknown_msg(fd, RTM_NEWMDB + 3);
}

static void
test_rtnl_rtgen(const int fd)
{
	static const struct strval32 types[] = {
		{ ARG_STR(RTM_NEWNSID) },
		{ ARG_STR(RTM_DELNSID) },
		{ ARG_STR(RTM_GETNSID) },
		{ ARG_STR(RTM_NEWCACHEREPORT) },
	};
	static const struct rtgenmsg msg = {
		.rtgen_family = AF_UNIX
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(msg));

	for (size_t i = 0; i < ARRAY_SIZE(types); i++) {
		TEST_NETLINK_(fd, nlh0, types[i].val, types[i].str,
			      NLM_F_REQUEST, "NLM_F_REQUEST",
			      sizeof(msg), &msg, sizeof(msg),
			      printf("{rtgen_family=AF_UNIX}"));
	}

	test_rtnl_unknown_msg(fd, RTM_NEWNSID + 3);
	test_rtnl_unknown_msg(fd, RTM_NEWCACHEREPORT + 1);
	test_rtnl_unknown_msg(fd, RTM_NEWCACHEREPORT + 2);
	test_rtnl_unknown_msg(fd, RTM_NEWCACHEREPORT + 3);
}

static void
test_rtnl_nexthop(const int fd)
{
	static const struct strval32 types[] = {
		{ ARG_STR(RTM_NEWNEXTHOP) },
		{ ARG_STR(RTM_DELNEXTHOP) },
		{ ARG_STR(RTM_GETNEXTHOP) },
	};
	static const struct {
		struct nhmsg msg;
		const char *af_str;
		const char *rest_str;
	} msgs[] = {
		{ { .nh_family = AF_UNIX, .nh_scope = RT_SCOPE_UNIVERSE,
		    .nh_protocol = RTPROT_KERNEL, .nh_flags = RTNH_F_DEAD, },
		  "{nh_family=AF_UNIX", ", nh_scope=RT_SCOPE_UNIVERSE"
		  ", nh_protocol=RTPROT_KERNEL, nh_flags=RTNH_F_DEAD}" },
		{ { .nh_family = 45, .nh_scope = 200,
		    .nh_protocol = 5, .resvd=1, .nh_flags = 0x80, },
		  "{nh_family=AF_MCTP", ", nh_scope=RT_SCOPE_SITE"
		  ", nh_protocol=0x5 /* RTPROT_??? */, resvd=0x1"
		  ", nh_flags=0x80 /* RTNH_F_??? */}" },
		{ { .nh_family = 46, .nh_scope = 201,
		    .nh_protocol = 99, .resvd=0xff, .nh_flags = 0xdeadbeef, },
		  "{nh_family=0x2e /* AF_??? */", ", nh_scope=0xc9"
		  ", nh_protocol=RTPROT_OPENR, resvd=0xff, nh_flags=RTNH_F_DEAD"
		  "|RTNH_F_PERVASIVE|RTNH_F_ONLINK|RTNH_F_OFFLOAD"
		  "|RTNH_F_UNRESOLVED|RTNH_F_TRAP|0xdeadbe80}" },
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(msgs[0].msg));

	for (size_t i = 0; i < ARRAY_SIZE(types); i++) {
		for (size_t j = 0; j < ARRAY_SIZE(msgs); j++) {
			TEST_NL_ROUTE_(fd, nlh0, types[i].val, types[i].str,
				       msgs[j].msg,
				       printf("%s", msgs[j].af_str),
				       printf("%s", msgs[j].rest_str));
		}
	}

	test_rtnl_unknown_msg(fd, RTM_NEWNEXTHOP + 3);
}

static void
test_rtnl_ifstats(const int fd)
{
	static const struct strval32 types[] = {
		{ ARG_STR(RTM_NEWSTATS) },
		{ ARG_STR(RTM_GETSTATS) },
	};
	const struct {
		struct if_stats_msg msg;
		const char *af_str;
		const char *rest_str;
	} msgs[] = {
		{ { .family = AF_UNIX, .pad1 = 0, .pad2 = 0,
		    .ifindex = ifindex_lo(), .filter_mask = 0, },
		  "{family=AF_UNIX", ", ifindex=" IFINDEX_LO_STR
		  ", filter_mask=0}" },
		{ { .family = 45, .pad1 = 0, .pad2 = 0xdead,
		    .ifindex = 0xdeadbeef, .filter_mask = 1, },
		  "{family=AF_MCTP", ", pad2=0xdead, ifindex=3735928559"
		  ", filter_mask=1<<IFLA_STATS_UNSPEC}" },
		{ { .family = 46, .pad1 = 0xca, .pad2 = 0,
		    .ifindex = ifindex_lo(), .filter_mask = 0xff, },
		  "{family=0x2e /* AF_??? */", ", pad1=0xca"
		  ", ifindex=" IFINDEX_LO_STR
		  ", filter_mask=1<<IFLA_STATS_UNSPEC|1<<IFLA_STATS_LINK_64"
		  "|1<<IFLA_STATS_LINK_XSTATS|1<<IFLA_STATS_LINK_XSTATS_SLAVE"
		  "|1<<IFLA_STATS_LINK_OFFLOAD_XSTATS|1<<IFLA_STATS_AF_SPEC"
		  "|0xc0}" },
		{ { .family = 255, .pad1 = 0xde, .pad2 = 0xbeef,
		    .ifindex = ifindex_lo(), .filter_mask = 0xdec0dec0, },
		  "{family=0xff /* AF_??? */", ", pad1=0xde"
		  ", pad2=0xbeef, ifindex=" IFINDEX_LO_STR
		  ", filter_mask=0xdec0dec0 /* 1<<IFLA_STATS_??? */}" },
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(msgs[0].msg));

	for (size_t i = 0; i < ARRAY_SIZE(types); i++) {
		for (size_t j = 0; j < ARRAY_SIZE(msgs); j++) {
			TEST_NL_ROUTE_(fd, nlh0, types[i].val, types[i].str,
				       msgs[j].msg,
				       printf("%s", msgs[j].af_str),
				       printf("%s", msgs[j].rest_str));
		}
	}

	test_rtnl_unknown_msg(fd, RTM_NEWSTATS + 1);
	test_rtnl_unknown_msg(fd, RTM_NEWSTATS + 3);
}

int main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	int fd = create_nl_socket(NETLINK_ROUTE);

	test_nlmsg_type(fd);
	test_nlmsg_flags(fd);
	test_nlmsg_done(fd);
	test_rtnl_unspec(fd);

	test_rtnl_link(fd);		/* 16 */
	test_rtnl_addr(fd);		/* 20, 56, 60 */
	test_rtnl_route(fd);		/* 24 */
	test_rtnl_neigh(fd);		/* 28 */
	test_rtnl_rule(fd);		/* 32 */
	test_rtnl_tc(fd);		/* 36, 40, 44, 100 */
	test_rtnl_tca(fd);		/* 48 */

	/* prefix */			/* 52 */
	test_rtnl_unsupported_msg(fd, ARG_STR(RTM_NEWPREFIX));
	test_rtnl_unknown_msg(fd, RTM_NEWPREFIX + 1);
	test_rtnl_unknown_msg(fd, RTM_NEWPREFIX + 2);
	test_rtnl_unknown_msg(fd, RTM_NEWPREFIX + 3);

	test_rtnl_neightbl(fd);		/* 64 */

	/* nduserport */		/* 68 */
	test_rtnl_unsupported_msg(fd, ARG_STR(RTM_NEWNDUSEROPT));
	test_rtnl_unknown_msg(fd, RTM_NEWNDUSEROPT + 1);
	test_rtnl_unknown_msg(fd, RTM_NEWNDUSEROPT + 2);
	test_rtnl_unknown_msg(fd, RTM_NEWNDUSEROPT + 3);

	test_rtnl_addrlabel(fd);	/* 72 */
	test_rtnl_dcb(fd);		/* 76 */
	test_rtnl_netconf(fd);		/* 80 */
	test_rtnl_mdb(fd);		/* 84 */
	test_rtnl_rtgen(fd);		/* 88, 96 */
	test_rtnl_ifstats(fd);		/* 92 */
	test_rtnl_nexthop(fd);		/* 104 */

	/* linkprop */			/* 108 */
	test_rtnl_unsupported_msg(fd, ARG_STR(RTM_NEWLINKPROP));
	test_rtnl_unsupported_msg(fd, ARG_STR(RTM_DELLINKPROP));
	test_rtnl_unsupported_msg(fd, ARG_STR(RTM_GETLINKPROP));
	test_rtnl_unknown_msg(fd, RTM_NEWLINKPROP + 3);

	/* vlan */			/* 112 */
	test_rtnl_unsupported_msg(fd, ARG_STR(RTM_NEWVLAN));
	test_rtnl_unsupported_msg(fd, ARG_STR(RTM_DELVLAN));
	test_rtnl_unsupported_msg(fd, ARG_STR(RTM_GETVLAN));
	test_rtnl_unknown_msg(fd, RTM_NEWVLAN + 3);

	/* nexthopbucket */		/* 116 */
	test_rtnl_unsupported_msg(fd, ARG_STR(RTM_NEWNEXTHOPBUCKET));
	test_rtnl_unsupported_msg(fd, ARG_STR(RTM_DELNEXTHOPBUCKET));
	test_rtnl_unsupported_msg(fd, ARG_STR(RTM_GETNEXTHOPBUCKET));
	test_rtnl_unknown_msg(fd, RTM_NEWNEXTHOPBUCKET + 3);

	for (uint16_t i = 120; i < 124; i++)
		test_rtnl_unknown_msg(fd, i);

	printf("+++ exited with 0 +++\n");

	return 0;
}
