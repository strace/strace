/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "test_nlattr.h"
#include <linux/ip.h>
#include <linux/rtnetlink.h>

#if !defined HAVE_MEMPCPY
# undef mempcpy
# define mempcpy strace_mempcpy
static void *
mempcpy(void *dest, const void *src, size_t n)
{
	memcpy(dest, src, n);

	return dest + n;
}
#endif

static void
print_quoted_hex_ellipsis(const void *const instr, const size_t len)
{
	const unsigned char *str = instr;

	printf("\"");
	for (size_t i = 0; i < MIN(len, DEFAULT_STRLEN); ++i)
		printf("\\x%02x", str[i]);
	printf("\"");
	if (len > DEFAULT_STRLEN)
		printf("...");
}

#define LWTUNNEL_ENCAP_NONE 0

#define DEF_NLATTR_RTMSG_FUNCS(sfx_, af_)				\
	static void							\
	init_##sfx_(struct nlmsghdr *const nlh, const unsigned int msg_len) \
	{								\
		SET_STRUCT(struct nlmsghdr, nlh,			\
			.nlmsg_len = msg_len,				\
			.nlmsg_type = RTM_GETROUTE,			\
			.nlmsg_flags = NLM_F_DUMP			\
		);							\
									\
		struct rtmsg *const msg = NLMSG_DATA(nlh);		\
		SET_STRUCT(struct rtmsg, msg,				\
			.rtm_family = (af_),				\
			.rtm_tos = IPTOS_LOWDELAY,			\
			.rtm_table = RT_TABLE_DEFAULT,			\
			.rtm_protocol = RTPROT_KERNEL,			\
			.rtm_scope = RT_SCOPE_UNIVERSE,			\
			.rtm_type = RTN_LOCAL,				\
			.rtm_flags = RTM_F_NOTIFY			\
		);							\
	}								\
									\
	static void							\
	print_##sfx_(const unsigned int msg_len)			\
	{								\
		printf("{nlmsg_len=%u, nlmsg_type=RTM_GETROUTE"		\
		       ", nlmsg_flags=NLM_F_DUMP"			\
		       ", nlmsg_seq=0, nlmsg_pid=0}, {rtm_family=" #af_	\
		       ", rtm_dst_len=0, rtm_src_len=0"			\
		       ", rtm_tos=IPTOS_LOWDELAY"			\
		       ", rtm_table=RT_TABLE_DEFAULT"			\
		       ", rtm_protocol=RTPROT_KERNEL"			\
		       ", rtm_scope=RT_SCOPE_UNIVERSE"			\
		       ", rtm_type=RTN_LOCAL"				\
		       ", rtm_flags=RTM_F_NOTIFY}",			\
		       msg_len);					\
	}								\
	/* End of DEF_NLATTR_RTMSG_FUNCS */

DEF_NLATTR_RTMSG_FUNCS(rtmsg, AF_UNIX)
DEF_NLATTR_RTMSG_FUNCS(rtmsg_inet, AF_INET)
DEF_NLATTR_RTMSG_FUNCS(rtmsg_inet6, AF_INET6)

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	const int fd = create_nl_socket(NETLINK_ROUTE);
	const unsigned int hdrlen = sizeof(struct rtmsg);
	const unsigned int nla_type = 0xffff & NLA_TYPE_MASK;
	char nla_type_str[256];
	void *nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen),
				   NLA_HDRLEN + sizeof(nla_type_str));

	static char pattern[4096];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);

	sprintf(nla_type_str, "%#x /* RTA_??? */", nla_type);
	TEST_NLATTR_(fd, nlh0, hdrlen,
		     init_rtmsg, print_rtmsg,
		     nla_type, nla_type_str,
		     4, pattern, 4,
		     print_quoted_hex(pattern, 4));

	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_rtmsg, print_rtmsg,
		    RTA_DST, 4, pattern, 4,
		    print_quoted_hex(pattern, 4));

#define MAX_ADDR_SZ 35
	static const struct {
		uint8_t af;
		uint8_t addr[MAX_ADDR_SZ];
		const char *str;
		void (* init_fn)(struct nlmsghdr *, unsigned int);
		void (* print_fn)(unsigned int);
		uint32_t len;
	} addrs[] = {
		{ AF_UNIX,  { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 },
		  "\"\\x00\\x01\\x02\\x03\\x04\\x05\\x06\\x07\\x08\\x09\"",
		  init_rtmsg,       print_rtmsg,       10 },
		{ AF_UNIX,
		  { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, [MAX_ADDR_SZ - 1] = 0xea },
		  "\"\\x00\\x01\\x02\\x03\\x04\\x05\\x06\\x07\\x08\\x09"
		    "\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00"
		    "\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00"
		    "\\x00\\x00"
#if DEFAULT_STRLEN == 32
		    "\"...",
#else
		    "\\x00\\x00\\xea\"",
#endif
		  init_rtmsg,       print_rtmsg,       MAX_ADDR_SZ },
		{ AF_INET,  { 0xde, 0xca, 0xff, 0xed },
		  "inet_addr(\"222.202.255.237\")",
		  init_rtmsg_inet,  print_rtmsg_inet,  4 },
		{ AF_INET6, { 0xfa, 0xce, 0xbe, 0xef, [15] = 0xda },
		  "inet_pton(AF_INET6, \"face:beef::da\")",
		  init_rtmsg_inet6, print_rtmsg_inet6, 16 },
	};
	static const struct strval32 addr_attrs[] = {
		{ ARG_STR(RTA_DST) },
		{ ARG_STR(RTA_SRC) },
		{ ARG_STR(RTA_GATEWAY) },
		{ ARG_STR(RTA_PREFSRC) },
		{ ARG_STR(RTA_NEWDST) },
	};
	for (size_t i = 0; i < ARRAY_SIZE(addrs); i++) {
		for (size_t j = 0; j < ARRAY_SIZE(addr_attrs); j++) {
			TEST_NLATTR_(fd, nlh0, hdrlen,
				     addrs[i].init_fn, addrs[i].print_fn,
				     addr_attrs[j].val, addr_attrs[j].str,
				     addrs[i].len - 1, addrs[i].addr,
				     addrs[i].len - 1,
				     print_quoted_hex_ellipsis(addrs[i].addr,
							       addrs[i].len - 1)
				     );
			TEST_NLATTR_(fd, nlh0, hdrlen,
				     addrs[i].init_fn, addrs[i].print_fn,
				     addr_attrs[j].val, addr_attrs[j].str,
				     addrs[i].len, addrs[i].addr, addrs[i].len,
				     printf("%s", addrs[i].str));
		}
	}

	const uint32_t ifindex = ifindex_lo();
	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_rtmsg, print_rtmsg,
			   RTA_OIF, pattern, ifindex,
			   printf(IFINDEX_LO_STR));

	const uint32_t rt_class_id = RT_TABLE_DEFAULT;
	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_rtmsg, print_rtmsg,
			   RTA_TABLE, pattern, rt_class_id,
			   printf("RT_TABLE_DEFAULT"));

	struct nlattr nla = {
		.nla_type = RTAX_LOCK,
		.nla_len = sizeof(nla)
	};
	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_rtmsg, print_rtmsg,
			   RTA_METRICS, pattern, nla,
			   printf("{nla_len=%u, nla_type=RTAX_LOCK}",
				  nla.nla_len));
	struct rtnexthop nh = {
		.rtnh_len = sizeof(nh) - 1,
		.rtnh_flags = RTNH_F_DEAD,
		.rtnh_hops = 0xab,
		.rtnh_ifindex = ifindex_lo()
	};
	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_rtmsg, print_rtmsg,
			   RTA_MULTIPATH, pattern, nh,
			   printf("[{rtnh_len=%u, rtnh_flags=RTNH_F_DEAD"
				  ", rtnh_hops=%u"
				  ", rtnh_ifindex=" IFINDEX_LO_STR "}]",
				  nh.rtnh_len, nh.rtnh_hops));

	char buf[RTNH_ALIGN(sizeof(nh)) + sizeof(nla)];
	nh.rtnh_len = sizeof(buf);
	nla.nla_type = RTA_DST;
	memcpy(buf, &nh, sizeof(nh));
	memcpy(buf + RTNH_ALIGN(sizeof(nh)), &nla, sizeof(nla));
	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_rtmsg, print_rtmsg,
		    RTA_MULTIPATH, sizeof(buf), buf, sizeof(buf),
		    printf("[{rtnh_len=%u, rtnh_flags=RTNH_F_DEAD"
			   ", rtnh_hops=%u, rtnh_ifindex=" IFINDEX_LO_STR "}"
			   ", {nla_len=%u, nla_type=RTA_DST}]",
			   nh.rtnh_len, nh.rtnh_hops, nla.nla_len));

	static const struct in_addr gw_inet_addr = { .s_addr = BE32(0xdeadbeef) };
	static const uint8_t via_inet6_addr[16] = {
		0xde, 0xad, 0xfa, 0xce, 0xbe, 0xef, 0xca, 0xfe,
		0xfe, 0xed, 0xba, 0x5e, 0x00, 0x00, 0xfa, 0xde };
	static const struct rtvia rtvia = { .rtvia_family = AF_INET6 };
	char buf2[2 * (RTNH_ALIGN(sizeof(nh)) + NLMSG_ALIGN(sizeof(nla))) +
		       + NLMSG_ALIGN(sizeof(gw_inet_addr))
		       + NLMSG_ALIGN(offsetof(struct rtvia, rtvia_addr)
				     + sizeof(via_inet6_addr))];
	char *pos = buf2;

	nh.rtnh_len = RTNH_ALIGN(sizeof(nh)) + NLMSG_ALIGN(sizeof(nla))
		      + NLMSG_ALIGN(sizeof(gw_inet_addr));
	nh.rtnh_flags = 0xc0;
	nla.nla_type = RTA_GATEWAY;
	nla.nla_len = NLMSG_ALIGN(sizeof(nla))
		      + NLMSG_ALIGN(sizeof(gw_inet_addr));
	pos = mempcpy(pos, &nh, sizeof(nh));
	pos = mempcpy(pos, &nla, sizeof(nla));
	pos = mempcpy(pos, &gw_inet_addr, sizeof(gw_inet_addr));

	nh.rtnh_len = RTNH_ALIGN(sizeof(nh)) + NLMSG_ALIGN(sizeof(nla))
		      + NLMSG_ALIGN(offsetof(struct rtvia, rtvia_addr)
				    + sizeof(via_inet6_addr));
	nla.nla_type = RTA_VIA;
	nla.nla_len = NLMSG_ALIGN(sizeof(nla))
		      + NLMSG_ALIGN(offsetof(struct rtvia, rtvia_addr)
				    + sizeof(via_inet6_addr));
	pos = mempcpy(pos, &nh, sizeof(nh));
	pos = mempcpy(pos, &nla, sizeof(nla));
	pos = mempcpy(pos, &rtvia, sizeof(rtvia));
	pos = mempcpy(pos, &via_inet6_addr, sizeof(via_inet6_addr));
	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_rtmsg_inet, print_rtmsg_inet,
		    RTA_MULTIPATH, sizeof(buf2), buf2, sizeof(buf2),
		    printf("[[{rtnh_len=%u, rtnh_flags=RTNH_F_TRAP|0x80"
			   ", rtnh_hops=%u, rtnh_ifindex=" IFINDEX_LO_STR "}"
			   ", [{nla_len=%u, nla_type=RTA_GATEWAY}"
			   ", inet_addr(\"222.173.190.239\")]]"
			   ", [{rtnh_len=%u, rtnh_flags=RTNH_F_TRAP|0x80"
			   ", rtnh_hops=%u, rtnh_ifindex=" IFINDEX_LO_STR "}"
			   ", [{nla_len=%u, nla_type=RTA_VIA}"
			   ", {rtvia_family=AF_INET6"
			   ", inet_pton(AF_INET6"
			   ", \"dead:face:beef:cafe:feed:ba5e:0:fade\""
			   ", &rtvia_addr)}]]]",
			   (uint32_t) (RTNH_ALIGN(sizeof(nh))
			   + NLMSG_ALIGN(sizeof(nla))
			   + NLMSG_ALIGN(sizeof(gw_inet_addr))),
			   nh.rtnh_hops,
			   (uint32_t) (NLMSG_ALIGN(sizeof(nla))
			   + NLMSG_ALIGN(sizeof(gw_inet_addr))),
			   (uint32_t) (RTNH_ALIGN(sizeof(nh))
			   + NLMSG_ALIGN(sizeof(nla))
			   + NLMSG_ALIGN(offsetof(struct rtvia, rtvia_addr)
					 + sizeof(via_inet6_addr))),
			   nh.rtnh_hops,
			   (uint32_t) (NLMSG_ALIGN(sizeof(nla))
			   + NLMSG_ALIGN(offsetof(struct rtvia, rtvia_addr)
					 + sizeof(via_inet6_addr)))
			   ));

	static const struct rta_cacheinfo ci = {
		.rta_clntref = 0xabcdefab,
		.rta_lastuse = 0xbdadaedc,
		.rta_expires = 0xcdadebad,
		.rta_error = 0xdaedadeb,
		.rta_used = 0xedfabdad,
		.rta_id = 0xfeadbcda,
		.rta_ts = 0xacdbaded,
		.rta_tsage = 0xbadeadef
	};
	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_rtmsg, print_rtmsg,
			   RTA_CACHEINFO, pattern, ci,
			   printf("{");
			   PRINT_FIELD_U(ci, rta_clntref);
			   printf(", ");
			   PRINT_FIELD_U(ci, rta_lastuse);
			   printf(", ");
			   PRINT_FIELD_U(ci, rta_expires);
			   printf(", ");
			   PRINT_FIELD_U(ci, rta_error);
			   printf(", ");
			   PRINT_FIELD_U(ci, rta_used);
			   printf(", ");
			   PRINT_FIELD_X(ci, rta_id);
			   printf(", ");
			   PRINT_FIELD_U(ci, rta_ts);
			   printf(", ");
			   PRINT_FIELD_U(ci, rta_tsage);
			   printf("}"));

	static const struct rta_mfc_stats mfcs = {
		.mfcs_packets = 0xadcdedfdadefadcd,
		.mfcs_bytes = 0xbaedadedcdedadbd,
		.mfcs_wrong_if = 0xcddeabeedaedabfa
	};
	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_rtmsg, print_rtmsg,
			   RTA_MFC_STATS, pattern, mfcs,
			   printf("{");
			   PRINT_FIELD_U(mfcs, mfcs_packets);
			   printf(", ");
			   PRINT_FIELD_U(mfcs, mfcs_bytes);
			   printf(", ");
			   PRINT_FIELD_U(mfcs, mfcs_wrong_if);
			   printf("}"));

	static const struct rtvia via = {
		.rtvia_family = AF_INET
	};

	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_rtmsg, print_rtmsg,
			   RTA_VIA, pattern, via,
			   printf("{rtvia_family=AF_INET}"));

	static const char address4[] = "12.34.56.78";
	struct in_addr a4 = {
		.s_addr = inet_addr(address4)
	};
	char rtviabuf[sizeof(via) + sizeof(a4)];
	memcpy(rtviabuf, &via, sizeof(via));
	memcpy(rtviabuf + sizeof(via), &a4, sizeof(a4));

	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_rtmsg, print_rtmsg,
		    RTA_VIA, sizeof(rtviabuf), rtviabuf, sizeof(rtviabuf),
		    printf("{rtvia_family=AF_INET"
			   ", rtvia_addr=inet_addr(\"%s\")}", address4));

	const uint16_t encap_type = LWTUNNEL_ENCAP_NONE;
	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_rtmsg, print_rtmsg,
			   RTA_ENCAP_TYPE, pattern, encap_type,
			   printf("LWTUNNEL_ENCAP_NONE"));

	puts("+++ exited with 0 +++");
	return 0;
}
