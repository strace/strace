/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2021 The strace developers.
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
#include <linux/ip.h>
#include <linux/neighbour.h>
#include <linux/netconf.h>
#include <linux/rtnetlink.h>

#define TEST_NL_ROUTE(fd_, nlh0_, type_, obj_, print_family_, ...)	\
	do {								\
		/* family and string */					\
		TEST_NETLINK((fd_), (nlh0_),				\
			     type_, NLM_F_REQUEST,			\
			     sizeof(obj_) - 1,				\
			     &(obj_), sizeof(obj_) - 1,			\
			     (print_family_);				\
			     printf(", ...}"));				\
									\
		/* sizeof(obj_) */					\
		TEST_NETLINK((fd_), (nlh0_),				\
			     type_, NLM_F_REQUEST,			\
			     sizeof(obj_), &(obj_), sizeof(obj_),	\
			     (print_family_);				\
			      __VA_ARGS__);				\
									\
		/* short read of sizeof(obj_) */			\
		TEST_NETLINK((fd_), (nlh0_),				\
			     type_, NLM_F_REQUEST,			\
			     sizeof(obj_), &(obj_), sizeof(obj_) - 1,	\
			     (print_family_);				\
			     printf(", %p}",				\
				    NLMSG_DATA(TEST_NETLINK_nlh) + 1));	\
	} while (0)

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
	nlh.nlmsg_flags = NLM_F_NONREC;
	rc = sendto(fd, &nlh, sizeof(nlh), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {nlmsg_len=%u, nlmsg_type=RTM_DELLINK"
	       ", nlmsg_flags=NLM_F_NONREC, nlmsg_seq=0, nlmsg_pid=0}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
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
		      printf("{family=AF_UNSPEC, \"\\x31\\x32\\x33\\x34\"}"));

	/* unknown family and string */
	family = 0xfd;
	memcpy(buf, &family, sizeof(family));
	TEST_NETLINK_(fd, nlh0,
		      0xffff, "0xffff /* RTM_??? */",
		      NLM_F_REQUEST, "NLM_F_REQUEST",
		      sizeof(buf), buf, sizeof(buf),
		      printf("{family=%#x /* AF_??? */"
			     ", \"\\x31\\x32\\x33\\x34\"}", family));
}

static void
test_rtnl_link(const int fd)
{
	const struct ifinfomsg ifinfo = {
		.ifi_family = AF_UNIX,
		.ifi_type = ARPHRD_LOOPBACK,
		.ifi_index = ifindex_lo(),
		.ifi_flags = IFF_UP,
		.ifi_change = 0xfabcdeba
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(ifinfo));

	TEST_NL_ROUTE(fd, nlh0, RTM_GETLINK, ifinfo,
		      printf("{ifi_family=AF_UNIX"),
		      printf(", ifi_type=ARPHRD_LOOPBACK"
			     ", ifi_index=" IFINDEX_LO_STR
			     ", ifi_flags=IFF_UP");
		      printf(", ");
		      PRINT_FIELD_X(ifinfo, ifi_change);
		      printf("}"));
}

static void
test_rtnl_addr(const int fd)
{
	const struct ifaddrmsg msg = {
		.ifa_family = AF_UNIX,
		.ifa_prefixlen = 0xde,
		.ifa_flags = IFA_F_SECONDARY,
		.ifa_scope = RT_SCOPE_UNIVERSE,
		.ifa_index = ifindex_lo()
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(msg));

	TEST_NL_ROUTE(fd, nlh0, RTM_GETADDR, msg,
		      printf("{ifa_family=AF_UNIX"),
		      printf(", ");
		      PRINT_FIELD_U(msg, ifa_prefixlen);
		      printf(", ifa_flags=IFA_F_SECONDARY"
			     ", ifa_scope=RT_SCOPE_UNIVERSE"
			     ", ifa_index=" IFINDEX_LO_STR);
		      printf("}"));
}

static void
test_rtnl_route(const int fd)
{
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

	TEST_NL_ROUTE(fd, nlh0, RTM_GETROUTE, msg,
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

static void
test_rtnl_rule(const int fd)
{
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

	TEST_NL_ROUTE(fd, nlh0, RTM_GETRULE, msg,
		      printf("{family=AF_UNIX"),
		      printf(", dst_len=%u, src_len=%u"
			     ", tos=IPTOS_LOWDELAY"
			     ", table=RT_TABLE_UNSPEC"
			     ", action=FR_ACT_TO_TBL"
			     ", flags=FIB_RULE_INVERT}",
			     msg.rtm_dst_len,
			     msg.rtm_src_len));
}

static void
test_rtnl_neigh(const int fd)
{
	const struct ndmsg msg = {
		.ndm_family = AF_UNIX,
		.ndm_ifindex = ifindex_lo(),
		.ndm_state = NUD_PERMANENT,
		.ndm_flags = NTF_PROXY,
		.ndm_type = RTN_UNSPEC
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(msg));

	TEST_NL_ROUTE(fd, nlh0, RTM_GETNEIGH, msg,
		      printf("{ndm_family=AF_UNIX"),
		      printf(", ndm_ifindex=" IFINDEX_LO_STR
			     ", ndm_state=NUD_PERMANENT"
			     ", ndm_flags=NTF_PROXY"
			     ", ndm_type=RTN_UNSPEC}"));
}

static void
test_rtnl_neightbl(const int fd)
{
	static const struct ndtmsg msg = {
		.ndtm_family = AF_NETLINK
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(msg));

	TEST_NETLINK(fd, nlh0,
		     RTM_GETNEIGHTBL, NLM_F_REQUEST,
		     sizeof(msg), &msg, sizeof(msg),
		     printf("{ndtm_family=AF_NETLINK}"));
}

static void
test_rtnl_tc(const int fd)
{
	const struct tcmsg msg = {
		.tcm_family = AF_UNIX,
		.tcm_ifindex = ifindex_lo(),
		.tcm_handle = 0xfadcdafb,
		.tcm_parent = 0xafbcadab,
		.tcm_info = 0xbcaedafa
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(msg));

	TEST_NL_ROUTE(fd, nlh0, RTM_GETQDISC, msg,
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

static void
test_rtnl_tca(const int fd)
{
	struct tcamsg msg = {
		.tca_family = AF_INET
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(msg));

	TEST_NETLINK(fd, nlh0,
		     RTM_GETACTION, NLM_F_REQUEST,
		     sizeof(msg), &msg, sizeof(msg),
		     printf("{tca_family=AF_INET}"));
}

static void
test_rtnl_addrlabel(const int fd)
{
	const struct ifaddrlblmsg msg = {
		.ifal_family = AF_UNIX,
		.ifal_prefixlen = 0xaf,
		.ifal_flags = 0xbd,
		.ifal_index = ifindex_lo(),
		.ifal_seq = 0xfadcdafb
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(msg));

	TEST_NL_ROUTE(fd, nlh0, RTM_GETADDRLABEL, msg,
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

static void
test_rtnl_dcb(const int fd)
{
	static const struct dcbmsg msg = {
		.dcb_family = AF_UNIX,
		.cmd = DCB_CMD_UNDEFINED
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(msg));

	TEST_NL_ROUTE(fd, nlh0, RTM_GETDCB, msg,
		      printf("{dcb_family=AF_UNIX"),
		      printf(", cmd=DCB_CMD_UNDEFINED}"));
}

static void
test_rtnl_netconf(const int fd)
{
	static const struct netconfmsg msg = {
		.ncm_family = AF_INET
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(msg));

	TEST_NETLINK(fd, nlh0,
		     RTM_GETNETCONF, NLM_F_REQUEST,
		     sizeof(msg), &msg, sizeof(msg),
		     printf("{ncm_family=AF_INET}"));
}

static void
test_rtnl_mdb(const int fd)
{
	const struct br_port_msg msg = {
		.family = AF_UNIX,
		.ifindex = ifindex_lo()
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(msg));

	TEST_NL_ROUTE(fd, nlh0, RTM_GETMDB, msg,
		      printf("{family=AF_UNIX"),
		      printf(", ifindex=" IFINDEX_LO_STR "}"));
}

static void
test_rtnl_nsid(const int fd)
{
	static const struct rtgenmsg msg = {
		.rtgen_family = AF_UNIX
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(msg));

	TEST_NETLINK(fd, nlh0,
		     RTM_GETNSID, NLM_F_REQUEST,
		     sizeof(msg), &msg, sizeof(msg),
		     printf("{rtgen_family=AF_UNIX}"));
}

int main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	int fd = create_nl_socket(NETLINK_ROUTE);

	test_nlmsg_type(fd);
	test_nlmsg_flags(fd);
	test_nlmsg_done(fd);
	test_rtnl_unspec(fd);
	test_rtnl_link(fd);
	test_rtnl_addr(fd);
	test_rtnl_route(fd);
	test_rtnl_rule(fd);
	test_rtnl_neigh(fd);
	test_rtnl_neightbl(fd);
	test_rtnl_tc(fd);
	test_rtnl_tca(fd);
	test_rtnl_addrlabel(fd);
	test_rtnl_dcb(fd);
	test_rtnl_netconf(fd);
	test_rtnl_mdb(fd);
	test_rtnl_nsid(fd);

	printf("+++ exited with 0 +++\n");

	return 0;
}
