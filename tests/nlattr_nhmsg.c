/*
 * Copyright (c) 2021 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2021-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <arpa/inet.h>
#include <inttypes.h>
#include <linux/ip.h>
#include <linux/rtnetlink.h>
#include <linux/nexthop.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "test_nlattr.h"

#include "xlat.h"
#define XLAT_MACROS_ONLY
# include "xlat/rtnl_nexthop_attrs.h"
# include "xlat/rtnl_nha_res_group_attrs.h"
# include "xlat/rtnl_nha_res_bucket_attrs.h"
#undef XLAT_MACROS_ONLY

#define DEF_NLATTR_NHMSG_FUNCS(sfx_, af_)				\
	static void							\
	init_##sfx_(struct nlmsghdr *const nlh, const unsigned int msg_len) \
	{								\
		SET_STRUCT(struct nlmsghdr, nlh,			\
			.nlmsg_len = msg_len,				\
			.nlmsg_type = RTM_GETNEXTHOP,			\
			.nlmsg_flags = NLM_F_DUMP,			\
		);							\
									\
		struct nhmsg *const msg = NLMSG_DATA(nlh);		\
		SET_STRUCT(struct nhmsg, msg,				\
			.nh_family = (af_),				\
			.nh_scope = RT_SCOPE_NOWHERE,			\
			.nh_protocol = RTPROT_UNSPEC,			\
			.nh_flags = 0x22,				\
		);							\
	}								\
									\
	static void							\
	print_##sfx_(const unsigned int msg_len)			\
	{								\
		printf("{nlmsg_len=%u, nlmsg_type=" XLAT_FMT		\
		       ", nlmsg_flags=" XLAT_FMT ", nlmsg_seq=0"	\
		       ", nlmsg_pid=0}, {nh_family=" XLAT_FMT		\
		       ", nh_scope=" XLAT_FMT ", nh_protocol=" XLAT_FMT	\
		       ", nh_flags=" XLAT_FMT "}",			\
		       msg_len, XLAT_ARGS(RTM_GETNEXTHOP),		\
		       XLAT_ARGS(NLM_F_DUMP), XLAT_SEL(af_, #af_),	\
		       XLAT_ARGS(RT_SCOPE_NOWHERE),			\
		       XLAT_ARGS(RTPROT_UNSPEC),			\
		       XLAT_ARGS(RTNH_F_PERVASIVE|RTNH_F_UNRESOLVED));	\
	}								\
	/* End of DEF_NLATTR_NHMSG_FUNCS */

#define DEF_NLATTR_NHMSG_NESTED_FUNCS(sfx_, attr_)			\
	static void							\
	init_nhmsg_##sfx_(struct nlmsghdr *const nlh,			\
			  const unsigned int msg_len)			\
	{								\
		init_nhmsg(nlh, msg_len);				\
		struct nlattr *nla = NLMSG_ATTR(nlh, sizeof(struct nhmsg));	\
		SET_STRUCT(struct nlattr, nla,				\
			.nla_len = msg_len				\
				   - NLMSG_SPACE(sizeof(struct nhmsg)),	\
			.nla_type = attr_,				\
		);							\
	}								\
									\
	static void							\
	print_nhmsg_##sfx_(const unsigned int msg_len)			\
	{								\
		print_nhmsg(msg_len);					\
		printf(", [{nla_len=%u, nla_type=" XLAT_FMT "}",	\
		       (unsigned int) (msg_len - NLMSG_HDRLEN		\
				       - NLMSG_ALIGN(sizeof(struct nhmsg))), \
		       XLAT_SEL(attr_, #attr_));			\
	}								\
	/* End of DEF_NLATTR_NHMSG_NESTED_FUNCS */

DEF_NLATTR_NHMSG_FUNCS(nhmsg, AF_UNIX)
DEF_NLATTR_NHMSG_FUNCS(nhmsg_inet, AF_INET)
DEF_NLATTR_NHMSG_FUNCS(nhmsg_inet6, AF_INET6)

DEF_NLATTR_NHMSG_NESTED_FUNCS(res_grp, NHA_RES_GROUP)
DEF_NLATTR_NHMSG_NESTED_FUNCS(res_bkt, NHA_RES_BUCKET)

static void
print_nh_grp(const struct nexthop_grp *const elem, size_t idx)
{
	switch (idx) {
	case 0: printf("{id=3735928559, weight=0, weight_high=0}"); break;
	case 1:	printf("{id=0, weight=218, weight_high=0, resvd2=0xdead}"); break;
	case 2: printf("{id=4207869677, weight=190, weight_high=236}"); break;
	case 3: printf("{id=0, weight=0, weight_high=202, resvd2=0xbeef}"); break;
	default: error_msg_and_fail("Unexpected grp index: %zu", idx);
	}
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	const int fd = create_nl_socket(NETLINK_ROUTE);
	const unsigned int hdrlen = sizeof(struct nhmsg);
	char nla_type_str[256];
	void *nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen),
				   NLA_HDRLEN + 42);

	static char pattern[4096];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);


	/* Unknown attrs */
	static const uint16_t unk_types[] = { 14, 0xffff & NLA_TYPE_MASK };
	for (size_t i = 0; i < ARRAY_SIZE(unk_types); i++) {
		sprintf(nla_type_str, "%#x" NRAW(" /* NHA_??? */"), unk_types[i]);
		TEST_NLATTR_(fd, nlh0, hdrlen,
			     init_nhmsg, print_nhmsg,
			     unk_types[i], nla_type_str,
			     4, pattern, 4,
			     print_quoted_hex(pattern, 4));
	}


	/* unimplemented, no semantics: NHA_UNSPEC, NHA_ENCAP */
	static const struct strval32 unimp_types[] = {
		{ ARG_XLAT_KNOWN(0, "NHA_UNSPEC") },
		{ ARG_XLAT_KNOWN(0x8, "NHA_ENCAP") } };
	for (size_t i = 0; i < ARRAY_SIZE(unimp_types); i++) {
		TEST_NLATTR_(fd, nlh0, hdrlen,
			     init_nhmsg, print_nhmsg,
			     unimp_types[i].val, unimp_types[i].str,
			     42, pattern, 42,
			     print_quoted_hex(pattern, 32);
			     printf("..."));
	}


	/* u32 attrs: NHA_ID, NHA_BLACKHOLE, NHA_GROUPS, NHA_FDB */
	static const struct strval32 u32_attrs[] = {
		{ ARG_XLAT_KNOWN(0x1, "NHA_ID") },
		{ ARG_XLAT_KNOWN(0x4, "NHA_BLACKHOLE") },
		{ ARG_XLAT_KNOWN(0x9, "NHA_GROUPS") },
		{ ARG_XLAT_KNOWN(0xb, "NHA_FDB") },
	};
	for (size_t i = 0; i < ARRAY_SIZE(u32_attrs); i++) {
		check_u32_nlattr(fd, nlh0, hdrlen, init_nhmsg, print_nhmsg,
				 u32_attrs[i].val, u32_attrs[i].str, pattern,
				 0);
	}


	/* NHA_GROUP */
	static const struct nexthop_grp grps[] = {
		{ .id = 0xdeadbeef, .weight = 0, .weight_high = 0, .resvd2 = 0 },
		{ .id = 0, .weight = 218, .weight_high = 0, .resvd2 = 0xdead },
		{ .id = 0xfacefeed, .weight = 190, .weight_high = 236, .resvd2 = 0 },
		{ .id = 0, .weight = 0, .weight_high = 202, .resvd2 = 0xbeef },
	};
	TEST_NLATTR_ARRAY_(fd, nlh0, hdrlen, init_nhmsg, print_nhmsg,
			   NHA_GROUP, XLAT_KNOWN(0x2, "NHA_GROUP"),
			   pattern, grps, print_nh_grp);


	/* NHA_GROUP_TYPE */
	static const struct strval16 grp_types[] = {
		{ ARG_XLAT_KNOWN(0, "NEXTHOP_GRP_TYPE_MPATH") },
		{ ARG_XLAT_KNOWN(0x1, "NEXTHOP_GRP_TYPE_RES") },
		{ ARG_XLAT_UNKNOWN(0x2, "NEXTHOP_GRP_TYPE_???") },
		{ ARG_XLAT_UNKNOWN(0xbeef, "NEXTHOP_GRP_TYPE_???") },
	};
	for (size_t i = 0; i < ARRAY_SIZE(grp_types); i++) {
		TEST_NLATTR_OBJECT_(fd, nlh0, hdrlen, init_nhmsg, print_nhmsg,
				   NHA_GROUP_TYPE,
				   XLAT_KNOWN(0x3, "NHA_GROUP_TYPE"),
				   pattern, grp_types[i].val,
				   printf("%s", grp_types[i].str));
		TEST_NLATTR_(fd, nlh0, hdrlen, init_nhmsg, print_nhmsg,
			     NHA_GROUP_TYPE, XLAT_KNOWN(0x3, "NHA_GROUP_TYPE"),
			     sizeof(grp_types[i].val) + 4,
			     &grp_types[i].val, sizeof(grp_types[i].val),
			     printf("%s", grp_types[i].str));
	}


	/* ifindex: NHA_OIF, NHA_MASTER */
	static const struct strval32 if_attrs[] = {
		{ ARG_XLAT_KNOWN(0x5, "NHA_OIF") },
		{ ARG_XLAT_KNOWN(0xa, "NHA_MASTER") },
	};
	const uint32_t ifindex = ifindex_lo();
	for (size_t i = 0; i < ARRAY_SIZE(if_attrs); i++) {
		static const uint32_t bogus = 0xdeadc0de;
		TEST_NLATTR_OBJECT_(fd, nlh0, hdrlen, init_nhmsg, print_nhmsg,
				   if_attrs[i].val, if_attrs[i].str,
				   pattern, bogus,
				   printf("3735929054"));
		TEST_NLATTR_(fd, nlh0, hdrlen, init_nhmsg, print_nhmsg,
			     if_attrs[i].val, if_attrs[i].str,
			     sizeof(bogus) + 4, &bogus, sizeof(bogus),
			     printf("3735929054"));

		TEST_NLATTR_OBJECT_(fd, nlh0, hdrlen, init_nhmsg, print_nhmsg,
				   if_attrs[i].val, if_attrs[i].str,
				   pattern, ifindex,
				   printf(XLAT_FMT_U,
					  XLAT_SEL(ifindex, IFINDEX_LO_STR)));
		TEST_NLATTR_(fd, nlh0, hdrlen, init_nhmsg, print_nhmsg,
			     if_attrs[i].val, if_attrs[i].str,
			     sizeof(ifindex) + 4, &ifindex, sizeof(ifindex),
			     printf(XLAT_FMT_U,
				    XLAT_SEL(ifindex, IFINDEX_LO_STR)));
	}


	/* NHA_GATEWAY */
	static const struct {
		uint8_t af;
		uint8_t addr[16];
		const char *str;
		void (* init_fn)(struct nlmsghdr *, unsigned int);
		void (* print_fn)(unsigned int);
		uint32_t len;
	} addrs[] = {
		{ AF_UNIX,  { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 },
		  "\"\\x00\\x01\\x02\\x03\\x04\\x05\\x06\\x07\\x08\\x09\"",
		  init_nhmsg,       print_nhmsg,       10 },
		{ AF_INET,  { 0xde, 0xca, 0xff, 0xed },
		  "inet_addr(\"222.202.255.237\")",
		  init_nhmsg_inet,  print_nhmsg_inet,  4 },
		{ AF_INET6, { 0xfa, 0xce, 0xbe, 0xef, [15] = 0xda },
		  "inet_pton(AF_INET6, \"face:beef::da\")",
		  init_nhmsg_inet6, print_nhmsg_inet6, 16 },
	};
	static const struct strval32 addr_attrs[] = {
		{ ARG_XLAT_KNOWN(0x6, "NHA_GATEWAY") },
	};
	for (size_t i = 0; i < ARRAY_SIZE(addrs); i++) {
		for (size_t j = 0; j < ARRAY_SIZE(addr_attrs); j++) {
			TEST_NLATTR_(fd, nlh0, hdrlen,
				     addrs[i].init_fn, addrs[i].print_fn,
				     addr_attrs[j].val, addr_attrs[j].str,
				     addrs[i].len - 1, addrs[i].addr,
				     addrs[i].len - 1,
				     print_quoted_hex(addrs[i].addr,
						      addrs[i].len - 1)
				     );
			TEST_NLATTR_(fd, nlh0, hdrlen,
				     addrs[i].init_fn, addrs[i].print_fn,
				     addr_attrs[j].val, addr_attrs[j].str,
				     addrs[i].len, addrs[i].addr, addrs[i].len,
#if XLAT_RAW || XLAT_VERBOSE
				     print_quoted_hex(addrs[i].addr,
						      addrs[i].len);
#endif
#if !XLAT_RAW
				     if (!(XLAT_VERBOSE
					   && addrs[i].af == AF_UNIX))
					     printf(VERB(" /* ") "%s"
						    VERB(" */"), addrs[i].str);
#endif
				     );
		}
	}


	/* NHA_ENCAP_TYPE */
	static const struct strval16 enc_types[] = {
		{ ARG_XLAT_KNOWN(0, "LWTUNNEL_ENCAP_NONE") },
		{ ARG_XLAT_KNOWN(0x8, "LWTUNNEL_ENCAP_RPL") },
		{ ARG_XLAT_UNKNOWN(0x9, "LWTUNNEL_ENCAP_???") },
		{ ARG_XLAT_UNKNOWN(0xbeef, "LWTUNNEL_ENCAP_???") },
	};
	for (size_t i = 0; i < ARRAY_SIZE(enc_types); i++) {
		TEST_NLATTR_OBJECT_(fd, nlh0, hdrlen, init_nhmsg, print_nhmsg,
				   NHA_ENCAP_TYPE,
				   XLAT_KNOWN(0x7, "NHA_ENCAP_TYPE"),
				   pattern, enc_types[i].val,
				   printf("%s", enc_types[i].str));
		TEST_NLATTR_(fd, nlh0, hdrlen, init_nhmsg, print_nhmsg,
			     NHA_ENCAP_TYPE, XLAT_KNOWN(0x7, "NHA_ENCAP_TYPE"),
			     sizeof(enc_types[i].val) + 4,
			     &enc_types[i].val, sizeof(enc_types[i].val),
			     printf("%s", enc_types[i].str));
	}


	/* NHA_RES_GROUP */
	static const unsigned int res_grp_hdrlen =
		sizeof(struct nhmsg) + sizeof(struct nlattr);
	void *nlh1 = midtail_alloc(NLMSG_SPACE(res_grp_hdrlen),
				   NLA_HDRLEN + 16);

	TEST_NLATTR_(fd, nlh0, hdrlen, init_nhmsg, print_nhmsg,
		     NHA_RES_GROUP, XLAT_KNOWN(0xc, "NHA_RES_GROUP"),
		     3, pattern, 3,
		     print_quoted_hex(pattern, 3));

	/* unknown NHA_RES_GROUP_* attr */
	static const uint16_t unk_res_grp_types[] = {
		5, 0xffff & NLA_TYPE_MASK,
	};
	for (size_t i = 0; i < ARRAY_SIZE(unk_res_grp_types); i++) {
		sprintf(nla_type_str, "%#x" NRAW(" /* NHA_RES_GROUP_??? */"),
			unk_res_grp_types[i]);
		TEST_NLATTR_(fd, nlh1, res_grp_hdrlen,
			     init_nhmsg_res_grp, print_nhmsg_res_grp,
			     unk_res_grp_types[i], nla_type_str,
			     16, pattern, 16,
			     print_quoted_hex(pattern, 16);
			     printf("]"));
	}

	/* not decoded: NHA_RES_GROUP_UNSPEC/NHA_RES_GROUP_PAD */
	TEST_NLATTR_(fd, nlh1, res_grp_hdrlen,
		     init_nhmsg_res_grp, print_nhmsg_res_grp,
		     NHA_RES_GROUP_PAD, XLAT_KNOWN(0, "NHA_RES_GROUP_PAD"),
		     8, pattern, 8,
		     print_quoted_hex(pattern, 8);
		     printf("]"));

	/* u16: NHA_RES_GROUP_BUCKETS */
	check_u16_nlattr(fd, nlh0, hdrlen,
			 init_nhmsg_res_grp, print_nhmsg_res_grp,
			 ARG_XLAT_KNOWN(0x1, "NHA_RES_GROUP_BUCKETS"),
			 pattern, 1);

	/* clock_t: NHA_RES_GROUP_IDLE_TIMER, NHA_RES_GROUP_UNBALANCED_TIMER,
	 *          NHA_RES_GROUP_UNBALANCED_TIME */
	static const struct strval32 res_grp_clk_attrs[] = {
		{ ARG_XLAT_KNOWN(0x2, "NHA_RES_GROUP_IDLE_TIMER") },
		{ ARG_XLAT_KNOWN(0x3, "NHA_RES_GROUP_UNBALANCED_TIMER") },
		{ ARG_XLAT_KNOWN(0x4, "NHA_RES_GROUP_UNBALANCED_TIME") },
	};
	for (size_t i = 0; i < ARRAY_SIZE(res_grp_clk_attrs); i++) {
		check_clock_t_nlattr(fd, nlh0, hdrlen,
				     init_nhmsg_res_grp, print_nhmsg_res_grp,
				     res_grp_clk_attrs[i].val,
				     res_grp_clk_attrs[i].str, 1);
	}


	/* NHA_RES_BUCKET */
	static const unsigned int res_bkt_hdrlen =
		sizeof(struct nhmsg) + sizeof(struct nlattr);

	TEST_NLATTR_(fd, nlh0, hdrlen, init_nhmsg, print_nhmsg,
		     NHA_RES_BUCKET, XLAT_KNOWN(0xd, "NHA_RES_BUCKET"),
		     3, pattern, 3,
		     print_quoted_hex(pattern, 3));

	/* unknown NHA_RES_GROUP_* attr */
	static const uint16_t unk_res_bkt_types[] = {
		4, 0xffff & NLA_TYPE_MASK,
	};
	for (size_t i = 0; i < ARRAY_SIZE(unk_res_bkt_types); i++) {
		sprintf(nla_type_str, "%#x" NRAW(" /* NHA_RES_BUCKET_??? */"),
			unk_res_bkt_types[i]);
		TEST_NLATTR_(fd, nlh1, res_bkt_hdrlen,
			     init_nhmsg_res_bkt, print_nhmsg_res_bkt,
			     unk_res_bkt_types[i], nla_type_str,
			     16, pattern, 16,
			     print_quoted_hex(pattern, 16);
			     printf("]"));
	}

	/* not decoded: NHA_RES_BUCKET_UNSPEC/NHA_RES_BUCKET_PAD */
	TEST_NLATTR_(fd, nlh1, res_bkt_hdrlen,
		     init_nhmsg_res_bkt, print_nhmsg_res_bkt,
		     NHA_RES_BUCKET_PAD, XLAT_KNOWN(0, "NHA_RES_BUCKET_PAD"),
		     8, pattern, 8,
		     print_quoted_hex(pattern, 8);
		     printf("]"));

	/* u16: NHA_RES_BUCKET_INDEX */
	check_u16_nlattr(fd, nlh0, hdrlen,
			 init_nhmsg_res_bkt, print_nhmsg_res_bkt,
			 ARG_XLAT_KNOWN(0x1, "NHA_RES_BUCKET_INDEX"),
			 pattern, 1);

	/* clock_t: NHA_RES_BUCKET_IDLE_TIME */
	static const struct strval32 res_bkt_clk_attrs[] = {
		{ ARG_XLAT_KNOWN(0x2, "NHA_RES_BUCKET_IDLE_TIME") },
	};
	for (size_t i = 0; i < ARRAY_SIZE(res_bkt_clk_attrs); i++) {
		check_clock_t_nlattr(fd, nlh0, hdrlen,
				     init_nhmsg_res_bkt, print_nhmsg_res_bkt,
				     res_bkt_clk_attrs[i].val,
				     res_bkt_clk_attrs[i].str, 1);
	}

	/* u32: NHA_RES_BUCKET_NH_ID */
	check_u32_nlattr(fd, nlh0, hdrlen,
			 init_nhmsg_res_bkt, print_nhmsg_res_bkt,
			 ARG_XLAT_KNOWN(0x3, "NHA_RES_BUCKET_NH_ID"),
			 pattern, 1);


	puts("+++ exited with 0 +++");
	return 0;
}
