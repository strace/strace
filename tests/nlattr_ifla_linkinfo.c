/*
 * IFLA_LINKINFO netlink attribute decoding check.
 *
 * Copyright (c) 2018-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "test_nlattr.h"
#include "xmalloc.h"

#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/if_bridge.h>
#include <linux/if_link.h>
#include <linux/rtnetlink.h>

#define XLAT_MACROS_ONLY
#include <xlat/rtnl_link_attrs.h>
#include <xlat/rtnl_ifla_info_attrs.h>
#undef XLAT_MACROS_ONLY

#define IFLA_ATTR IFLA_LINKINFO
#include "nlattr_ifla.h"

#define COMMA ,
#define TEST_UNKNOWN_TUNNELS(fd_, nlh0_, kindtype_, objtype_, objtype_str_, \
			     obj_, objsz_, arrstrs_, ...)		\
	do {								\
		/* 64 is guestimate for maximum unknown type len */	\
		char buf[8 * 2 + 64 + objsz_];				\
		const char *const *arrstrs[] = arrstrs_;		\
		const char *const **arrstrs_pos = arrstrs;		\
		const char *const *arrstr = *arrstrs_pos;		\
									\
		for (const char *type = arrstr ? arrstr[0] : NULL;	\
		     type && arrstr;					\
		     type = (++arrstr)[0] ? arrstr[0]			\
					  : (++arrstrs_pos)[0]		\
				             ? (arrstr = arrstrs_pos[0])[0] \
					     : NULL)			\
		{							\
			size_t type_len = strlen(type) + 1;		\
									\
			if (type_len > 64)				\
				error_msg_and_fail("Unexpectedly long "	\
						   "unknown type: \"%s\" " \
						   "(length is %zu)",	\
						   type, type_len);	\
									\
			struct nlattr obj_nla = {			\
				.nla_len = NLA_HDRLEN + (objsz_),	\
				.nla_type = (objtype_),			\
			};						\
									\
			char *pos = buf;				\
			memcpy(pos, type, type_len);			\
			pos += NLA_ALIGN(type_len);			\
			memcpy(pos, &obj_nla, sizeof(obj_nla));		\
			pos += sizeof(obj_nla);				\
			memcpy(pos, (obj_), (objsz_));			\
									\
			TEST_NLATTR_EX_((fd_),				\
					(nlh0_) - hdrlen - (pos - buf),	\
					hdrlen + NLA_HDRLEN,		\
					init_ifinfomsg, print_ifinfomsg, \
					(kindtype_), #kindtype_,	\
					type_len, objsz_ + (pos - buf),	\
					buf, objsz_ + (pos - buf),	\
					printf("\"%s\"]", type);	\
					printf(", [{nla_len=%zu"	\
				               ", nla_type=%s}, ",	\
					       (objsz_) + NLA_HDRLEN,	\
					       (objtype_str_));		\
									\
					{ __VA_ARGS__; }		\
									\
					printf("]"));			\
		}							\
	} while (0)

#define TEST_LINKINFO_(fd_, nlh0_, kindtype_, nla_type_, nla_type_str_,	\
		       tuntype_, obj_, objsz_, pattern_, fallback_func_, ...) \
	do {								\
		size_t tuntype_len = strlen(tuntype_) + 1;		\
		char *buf = tail_alloc(NLA_ALIGN(tuntype_len)		\
				       + NLA_HDRLEN + (objsz_));	\
		char *pos = buf;					\
									\
		struct nlattr obj_nla = {				\
			.nla_len = NLA_HDRLEN + (objsz_),		\
			.nla_type = (nla_type_),			\
		};							\
									\
		memcpy(pos, (tuntype_), tuntype_len);			\
		pos += NLA_ALIGN(tuntype_len);				\
		memcpy(pos, &obj_nla, sizeof(obj_nla));			\
		pos += sizeof(obj_nla);					\
		memcpy(pos, &(obj_), (objsz_));				\
									\
		if (fallback_func_ == print_quoted_hex) {		\
			TEST_NLATTR_EX_((fd_),				\
					(nlh0_) - NLA_HDRLEN,		\
					hdrlen + NLA_HDRLEN,		\
					init_ifinfomsg, print_ifinfomsg, \
					(kindtype_), #kindtype_,	\
					tuntype_len,			\
					objsz_ + (pos - buf) - 1,	\
					buf, objsz_ + (pos - buf) - 1,	\
					printf("\"%s\"]", (tuntype_));	\
					printf(", [{nla_len=%zu"	\
					       ", nla_type=%s}, ",	\
					       (objsz_) + NLA_HDRLEN,	\
					       (nla_type_str_));	\
					(fallback_func_)((obj_),	\
							 (objsz_) - 1);	\
					printf("]"));			\
		}							\
									\
		TEST_NLATTR_EX_((fd_), (nlh0_) - NLA_HDRLEN,		\
				hdrlen + NLA_HDRLEN,			\
				init_ifinfomsg, print_ifinfomsg,	\
				(kindtype_), #kindtype_,		\
				tuntype_len, objsz_ + (pos - buf),	\
				buf, objsz_ + (pos - buf) - 1,		\
				printf("\"%s\"]", (tuntype_));		\
				printf(", [{nla_len=%zu, nla_type=%s}, ", \
				       (objsz_) + NLA_HDRLEN,		\
				       (nla_type_str_));		\
				printf("%p]",				\
				       RTA_DATA(NLMSG_ATTR(nlh,		\
				       (hdrlen + NLA_HDRLEN + (pos - buf)))) \
				       )				\
				);					\
									\
		TEST_NLATTR_EX_((fd_), (nlh0_) - NLA_HDRLEN,		\
				hdrlen + NLA_HDRLEN,			\
				init_ifinfomsg, print_ifinfomsg,	\
				(kindtype_), #kindtype_,		\
				tuntype_len, objsz_ + (pos - buf),	\
				buf, objsz_ + (pos - buf),		\
				printf("\"%s\"]", (tuntype_));		\
				printf(", [{nla_len=%zu, nla_type=%s}, ", \
				       (objsz_) + NLA_HDRLEN,		\
				       (nla_type_str_));		\
									\
				{ __VA_ARGS__; }			\
									\
				printf("]"));				\
	} while (0)

#define TEST_LINKINFO(fd_, nlh0_, kindtype_, nla_type_, tuntype_,	\
		      obj_, pattern_, fallback_func_, ...)		\
	TEST_LINKINFO_((fd_), (nlh0_), kindtype_, nla_type_, #nla_type_, \
		       (tuntype_), (obj_), sizeof(obj_), pattern_,	\
		       fallback_func_, __VA_ARGS__)

#define TEST_NESTED_LINKINFO(fd_, nlh0_, kindtype_,			\
			     nla_type_, nla_type_str_, tuntype_,	\
			     subnla_type_, subnla_type_str_,		\
			     obj_, pattern_, ...)			\
	do {								\
		size_t tuntype_len = strlen(tuntype_) + 1;		\
		struct {						\
			size_t sz;					\
			const char *str;				\
		} attrs[] = { __VA_ARGS__ };				\
		size_t tunhdrlen;					\
		size_t buflen = NLA_ALIGN(tuntype_len) + NLA_HDRLEN;	\
		size_t attrsz = 0;					\
									\
		for (size_t i = 0; i < ARRAY_SIZE(attrs); i++)		\
			attrsz += NLA_HDRLEN + NLA_ALIGN(attrs[i].sz);	\
									\
		buflen += attrsz;					\
									\
		char *buf = tail_alloc(buflen);				\
		char *pos = buf;					\
									\
		struct nlattr nla = {					\
			.nla_len = NLA_HDRLEN + attrsz,			\
			.nla_type = (nla_type_),			\
		};							\
									\
		memcpy(pos, (tuntype_), tuntype_len);			\
		pos += NLA_ALIGN(tuntype_len);				\
		memcpy(pos, &nla, sizeof(nla));				\
		pos += sizeof(nla);					\
									\
		tunhdrlen = pos - buf;					\
									\
		nla.nla_type = subnla_type_;				\
									\
		for (size_t i = 0; i < ARRAY_SIZE(attrs); i++) {	\
			nla.nla_len = NLA_HDRLEN + attrs[i].sz;		\
			memcpy(pos, &nla, sizeof(nla));			\
			pos += sizeof(nla);				\
									\
			memcpy(pos, &(obj_), MIN(sizeof(obj_), attrs[i].sz)); \
									\
			if (attrs[i].sz > sizeof(obj_))			\
				memcpy(pos + sizeof(obj_),		\
				       &(pattern_),			\
				       attrs[i].sz - sizeof(obj_));	\
									\
			pos += NLA_ALIGN(attrs[i].sz);			\
		}							\
									\
		TEST_NLATTR_EX_((fd_), (nlh0_) - hdrlen - tunhdrlen,	\
				hdrlen + NLA_HDRLEN,			\
				init_ifinfomsg, print_ifinfomsg,	\
				(kindtype_), #kindtype_,		\
				tuntype_len, buflen,			\
				buf, buflen,				\
				printf("\"%s\"]", (tuntype_));		\
				printf(", [{nla_len=%zu, nla_type=%s}, [", \
				       attrsz + NLA_HDRLEN,		\
				       (nla_type_str_));		\
									\
				for (size_t i = 0; i < ARRAY_SIZE(attrs); i++) \
					printf("%s%s{nla_len=%zu"	\
					       ", nla_type=%s}%s%s%s",	\
					       i ? ", " : "",		\
					       attrs[i].str ? "[": "",	\
					       attrs[i].sz + NLA_HDRLEN, \
					       subnla_type_str_,	\
					       attrs[i].str ? ", ": "", \
					       attrs[i].str ?: "",	\
					       attrs[i].str ? "]" : ""); \
									\
				printf("]]"));				\
	} while (0)

int
main(void)
{
	static const uint8_t unknown_msg[] = { 0xab, 0xac, 0xdb, 0xcd };
	static const char *const unsupported_tunnel_types[] = {
		"batadv", "bareudp", "bond",
		"caif", "cfhsi",
		"dummy",
		"erspan",
		"geneve", "gre", "gretap", "gtp",
		"hsr",
		"ifb", "ip6erspan", "ip6gre", "ip6gretap", "ip6tnl",
		"ipip", "ipoib", "ipvlan", "ipvtap",
		"lowpan",
		"macsec", "macvlan", "macvtap",
		"netdevsim", "nlmon",
		"openvswitch",
		"ppp",
		"rmnet",
		"sit",
		"team",
		"vcan", "veth", "vlan", "vrf", "vsockmon",
		"vti", "vti6", "vxcan", "vxlan",
		"wireguard", "wwan",
		"xfrm",
		NULL
	};
	static const char *const unsupported_xstats_types[] = {
		"bridge",
		"tun",
		NULL
	};
	static const char *const unsupported_data_types[] = {
		"can",
		NULL
	};
	static const char *const unsupported_slave_data_types[] = {
		"can",
		"tun",
		NULL
	};
	/* supported by at least one attribute */
	static const char *const supported_tunnel_types[] = {
		"bridge",
		"can",
		"tun",
		NULL
	};

	skip_if_unavailable("/proc/self/fd/");

	const int fd = create_nl_socket(NETLINK_ROUTE);

	const unsigned int hdrlen = sizeof(struct ifinfomsg);
	void *nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen), 2 * NLA_HDRLEN + 256);

	static char pattern[4096];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);


	/* unknown AF_INFO_* type */
	TEST_NESTED_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_ifinfomsg, print_ifinfomsg,
			   IFLA_INFO_UNSPEC, pattern, unknown_msg,
			   printf("\"\\xab\\xac\\xdb\\xcd\""));

	TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
			       init_ifinfomsg, print_ifinfomsg,
			       6, "0x6 /* IFLA_INFO_??? */", pattern,
			       unknown_msg, print_quoted_hex, 1,
			       printf("\"\\xab\\xac\\xdb\\xcd\""));


	/* IFLA_INFO_KIND */
	TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
				      init_ifinfomsg, print_ifinfomsg,
				      IFLA_INFO_KIND, "IFLA_INFO_KIND", pattern,
				      unknown_msg, print_quoted_stringn, 1,
				      printf("\"\\253\\254\\333\\315\"..."));


	/* IFLA_INFO_KIND + IFLA_INFO_UNSPEC */
	TEST_UNKNOWN_TUNNELS(fd, nlh0, IFLA_INFO_KIND,
			     IFLA_INFO_UNSPEC, "IFLA_INFO_UNSPEC",
			     unknown_msg, sizeof(unknown_msg),
			     {unsupported_tunnel_types COMMA
			      supported_tunnel_types COMMA
			      NULL},
			     printf("\"\\xab\\xac\\xdb\\xcd\""));


	/* IFLA_INFO_KIND + IFLA_INFO_KIND */
	TEST_UNKNOWN_TUNNELS(fd, nlh0, IFLA_INFO_KIND,
			     IFLA_INFO_KIND, "IFLA_INFO_KIND",
			     unknown_msg, sizeof(unknown_msg),
			     {unsupported_tunnel_types COMMA
			      supported_tunnel_types COMMA
			      NULL},
			     printf("\"\\253\\254\\333\\315\"..."));


	/* IFLA_INFO_KIND + IFLA_INFO_DATA */
	TEST_UNKNOWN_TUNNELS(fd, nlh0, IFLA_INFO_KIND,
			     IFLA_INFO_DATA, "IFLA_INFO_DATA",
			     unknown_msg, sizeof(unknown_msg),
			     {unsupported_tunnel_types COMMA
			      unsupported_data_types COMMA
			      NULL},
			     printf("\"\\xab\\xac\\xdb\\xcd\""));

	struct val_name {
		unsigned int val;
		const char *name;
	};

	static const uint64_t u64_val = 0xdeadc0defacefeedULL;
	static const uint32_t u32_val = 0xbadc0dedU;
	static const uint16_t u16_val = 0xdeed;
	static const uint8_t  u8_val  = 0xa1;

	/* bridge attrs */
	static const struct val_name und_br_attrs[] = {
		{ 0, "IFLA_BR_UNSPEC" },
		{ 21, "IFLA_BR_FDB_FLUSH" },
		{ 40, "IFLA_BR_PAD" },
		{ 48, "0x30 /* IFLA_BR_??? */" },
	};

	for (size_t k = 0; k < ARRAY_SIZE(und_br_attrs); k++) {
		TEST_NESTED_LINKINFO(fd, nlh0, IFLA_INFO_KIND,
				     2, "IFLA_INFO_DATA", "bridge",
				     und_br_attrs[k].val, und_br_attrs[k].name,
				     unknown_msg, pattern,
				     { 2, "\"\\xab\\xac\"" },
				     { 4, "\"\\xab\\xac\\xdb\\xcd\"" },
				     { 6,
					"\"\\xab\\xac\\xdb\\xcd\\x61\\x62\"" },
				     { 8, "\"\\xab\\xac\\xdb\\xcd\\x61\\x62"
					"\\x63\\x64\"" },
				     { 10, "\"\\xab\\xac\\xdb\\xcd\\x61\\x62"
					"\\x63\\x64\\x65\\x66\"" });
	}

	static const struct val_name hwa_br_attrs[] = {
		{ 20, "IFLA_BR_GROUP_ADDR" },
	};

	for (size_t k = 0; k < ARRAY_SIZE(hwa_br_attrs); k++) {
		TEST_NESTED_LINKINFO(fd, nlh0, IFLA_INFO_KIND,
				     2, "IFLA_INFO_DATA", "bridge",
				     hwa_br_attrs[k].val, hwa_br_attrs[k].name,
				     unknown_msg, pattern,
				     { 2, "ab:ac" },
				     { 4, "ab:ac:db:cd" },
				     { 6, "ab:ac:db:cd:61:62" },
				     { 8, "ab:ac:db:cd:61:62:63:64" },
				     { 10, "ab:ac:db:cd:61:62:63:64:65:66" });
	}

	static const struct val_name c_t_br_attrs[] = {
		{  1, "IFLA_BR_FORWARD_DELAY" },
		{  2, "IFLA_BR_HELLO_TIME" },
		{  3, "IFLA_BR_MAX_AGE" },
		{  4, "IFLA_BR_AGEING_TIME" },
		{ 16, "IFLA_BR_HELLO_TIMER" },
		{ 17, "IFLA_BR_TCN_TIMER" },
		{ 18, "IFLA_BR_TOPOLOGY_CHANGE_TIMER" },
		{ 19, "IFLA_BR_GC_TIMER" },
		{ 30, "IFLA_BR_MCAST_LAST_MEMBER_INTVL" },
		{ 31, "IFLA_BR_MCAST_MEMBERSHIP_INTVL" },
		{ 32, "IFLA_BR_MCAST_QUERIER_INTVL" },
		{ 33, "IFLA_BR_MCAST_QUERY_INTVL" },
		{ 34, "IFLA_BR_MCAST_QUERY_RESPONSE_INTVL" },
		{ 35, "IFLA_BR_MCAST_STARTUP_QUERY_INTVL" },
	};
	char sz7_str[64];
	char sz8_str[64];

	clock_t_str(BE_LE(0xdeadc0defacefe, 0xadc0defacefeed),
		    ARRSZ_PAIR(sz7_str));
	clock_t_str(0xdeadc0defacefeed, ARRSZ_PAIR(sz8_str));

	for (size_t k = 0; k < ARRAY_SIZE(c_t_br_attrs); k++) {
		TEST_NESTED_LINKINFO(fd, nlh0, IFLA_INFO_KIND,
				     2, "IFLA_INFO_DATA", "bridge",
				     c_t_br_attrs[k].val, c_t_br_attrs[k].name,
				     u64_val, pattern,
				     { 7, sz7_str },
				     { 8, sz8_str },
				     { 9, "\"" BE_LE("\\xde\\xad\\xc0\\xde"
						     "\\xfa\\xce\\xfe\\xed",
						     "\\xed\\xfe\\xce\\xfa"
						     "\\xde\\xc0\\xad\\xde")
					  "\\x61\"" });
	}

	static const struct val_name u32_br_attrs[] = {
		{  5, "IFLA_BR_STP_STATE" },
		{ 13, "IFLA_BR_ROOT_PATH_COST" },
		{ 26, "IFLA_BR_MCAST_HASH_ELASTICITY" },
		{ 27, "IFLA_BR_MCAST_HASH_MAX" },
		{ 28, "IFLA_BR_MCAST_LAST_MEMBER_CNT" },
		{ 29, "IFLA_BR_MCAST_STARTUP_QUERY_CNT" },
	};

	for (size_t k = 0; k < ARRAY_SIZE(u32_br_attrs); k++) {
		TEST_NESTED_LINKINFO(fd, nlh0, IFLA_INFO_KIND,
				     2, "IFLA_INFO_DATA", "bridge",
				     u32_br_attrs[k].val, u32_br_attrs[k].name,
				     u32_val, pattern,
				     { 3, BE_LE("\"\\xba\\xdc\\x0d\"",
						"\"\\xed\\x0d\\xdc\"") },
				     { 4, "3134983661" },
				     { 5, "3134983661" });
	}

	static const struct val_name u16_br_attrs[] = {
		{  6, "IFLA_BR_PRIORITY" },
		{ 12, "IFLA_BR_ROOT_PORT" },
		{ 39, "IFLA_BR_VLAN_DEFAULT_PVID" },
	};

	for (size_t k = 0; k < ARRAY_SIZE(u16_br_attrs); k++) {
		TEST_NESTED_LINKINFO(fd, nlh0, IFLA_INFO_KIND,
				     2, "IFLA_INFO_DATA", "bridge",
				     u16_br_attrs[k].val, u16_br_attrs[k].name,
				     u16_val, pattern,
				     { 1, "\"" BE_LE("\\xde", "\\xed") "\"" },
				     { 2, "57069" },
				     { 3, "57069" });
	}

	static const struct val_name x16_br_attrs[] = {
		{  9, "IFLA_BR_GROUP_FWD_MASK" },
	};

	for (size_t k = 0; k < ARRAY_SIZE(x16_br_attrs); k++) {
		TEST_NESTED_LINKINFO(fd, nlh0, IFLA_INFO_KIND,
				     2, "IFLA_INFO_DATA", "bridge",
				     x16_br_attrs[k].val, x16_br_attrs[k].name,
				     u16_val, pattern,
				     { 1, "\"" BE_LE("\\xde", "\\xed") "\"" },
				     { 2, "0xdeed" },
				     { 3, "0xdeed" });
	}

	static const struct val_name u8_br_attrs[] = {
		{  7, "IFLA_BR_VLAN_FILTERING" },
		{ 14, "IFLA_BR_TOPOLOGY_CHANGE" },
		{ 15, "IFLA_BR_TOPOLOGY_CHANGE_DETECTED" },
		{ 22, "IFLA_BR_MCAST_ROUTER" },
		{ 23, "IFLA_BR_MCAST_SNOOPING" },
		{ 24, "IFLA_BR_MCAST_QUERY_USE_IFADDR" },
		{ 25, "IFLA_BR_MCAST_QUERIER" },
		{ 36, "IFLA_BR_NF_CALL_IPTABLES" },
		{ 37, "IFLA_BR_NF_CALL_IP6TABLES" },
		{ 38, "IFLA_BR_NF_CALL_ARPTABLES" },
		{ 41, "IFLA_BR_VLAN_STATS_ENABLED" },
		{ 42, "IFLA_BR_MCAST_STATS_ENABLED" },
		{ 43, "IFLA_BR_MCAST_IGMP_VERSION" },
		{ 44, "IFLA_BR_MCAST_MLD_VERSION" },
		{ 45, "IFLA_BR_VLAN_STATS_PER_PORT" },
	};

	for (size_t k = 0; k < ARRAY_SIZE(u8_br_attrs); k++) {
		TEST_NESTED_LINKINFO(fd, nlh0, IFLA_INFO_KIND,
				     2, "IFLA_INFO_DATA", "bridge",
				     u8_br_attrs[k].val, u8_br_attrs[k].name,
				     u8_val, pattern,
				     { 0, NULL },
				     { 1, "161" },
				     { 2, "161" });
	}

	unsigned short eth_p = htons(0x88C7);
	TEST_NESTED_LINKINFO(fd, nlh0, IFLA_INFO_KIND,
			     2, "IFLA_INFO_DATA", "bridge",
			     8, "IFLA_BR_VLAN_PROTOCOL",
			     eth_p, pattern,
			     { 1, "\"\\x88\"" },
			     { 2, "htons(ETH_P_PREAUTH)" },
			     { 2, "htons(ETH_P_PREAUTH)" });

	static const uint8_t bridge_id[]
		= { 0xbe, 0xef, 0xfa, 0xce, 0xde, 0xc0, 0xde, 0xad };
	static const struct val_name br_id_attrs[] = {
		{ 10, "IFLA_BR_ROOT_ID" },
		{ 11, "IFLA_BR_BRIDGE_ID" },
	};

	for (size_t k = 0; k < ARRAY_SIZE(br_id_attrs); k++) {
		TEST_NESTED_LINKINFO(fd, nlh0, IFLA_INFO_KIND,
				     2, "IFLA_INFO_DATA", "bridge",
				     br_id_attrs[k].val, br_id_attrs[k].name,
				     bridge_id, pattern,
				     { 7, "\"\\xbe\\xef\\xfa\\xce"
					  "\\xde\\xc0\\xde\"" },
				     { 8, "{prio=[190, 239]"
					  ", addr=fa:ce:de:c0:de:ad}" },
				     { 9, "{prio=[190, 239]"
					  ", addr=fa:ce:de:c0:de:ad}" });
	}

	static const struct {
		struct br_boolopt_multi val;
		const char *crop_str;
		const char *str;
	} boolopts[] = {
		{ { .optval = 0, .optmask = 0 },
		  "\"\\x00\\x00\\x00\\x00\\x00\\x00\\x00\"",
		  "{optval=0, optmask=0}" },
		{ { .optval = 1, .optmask = 2 },
		  BE_LE("\"\\x00\\x00\\x00\\x01\\x00\\x00\\x00\"",
			"\"\\x01\\x00\\x00\\x00\\x02\\x00\\x00\""),
		  "{optval=1<<BR_BOOLOPT_NO_LL_LEARN"
		  ", optmask=1<<BR_BOOLOPT_MCAST_VLAN_SNOOPING}" },
		{ { .optval = 0xdeadfae8, .optmask = 0xbadc0de8 },
		  BE_LE("\"\\xde\\xad\\xfa\\xe8\\xba\\xdc\\x0d\"",
			"\"\\xe8\\xfa\\xad\\xde\\xe8\\x0d\\xdc\""),
		  "{optval=0xdeadfae8 /* 1<<BR_BOOLOPT_??? */"
		  ", optmask=0xbadc0de8 /* 1<<BR_BOOLOPT_??? */}" },
		{ { .optval = 0xfacebeef, .optmask = 0xfeedcafe },
		  BE_LE("\"\\xfa\\xce\\xbe\\xef\\xfe\\xed\\xca\"",
			"\"\\xef\\xbe\\xce\\xfa\\xfe\\xca\\xed\""),
		  "{optval=1<<BR_BOOLOPT_NO_LL_LEARN"
			 "|1<<BR_BOOLOPT_MCAST_VLAN_SNOOPING"
			 "|1<<BR_BOOLOPT_MST_ENABLE|0xfacebee8"
		  ", optmask=1<<BR_BOOLOPT_MCAST_VLAN_SNOOPING"
		           "|1<<BR_BOOLOPT_MST_ENABLE|0xfeedcaf8}" },
	};
	for (size_t k = 0; k < ARRAY_SIZE(boolopts); k++) {
		TEST_NESTED_LINKINFO(fd, nlh0, IFLA_INFO_KIND,
				     2, "IFLA_INFO_DATA", "bridge",
				     IFLA_BR_MULTI_BOOLOPT,
				     "IFLA_BR_MULTI_BOOLOPT",
				     boolopts[k].val, pattern,
				     { 7, boolopts[k].crop_str },
				     { 9, boolopts[k].str },
				     { 9, boolopts[k].str });
	}

#define QSTATE_NLA(type_, type_str_, field_, crop_str_, str_, ...)	\
	{ { .nla = { .nla_len = NLA_HDRLEN				\
				+ sizeof(qstate_attrs[0].val.payload.field_), \
		     .nla_type = type_ },				\
	    .payload = { .field_ = __VA_ARGS__ } },			\
	  .sz = NLA_HDRLEN + sizeof(qstate_attrs[0].val.payload.field_), \
	  .type_str = type_str_, .crop_str = crop_str_, .str = str_ }

	char ip_timer_crop[64];
	char ip_timer[64];
	char ipv6_timer_crop[64];
	char ipv6_timer[64];
	struct {
		struct {
			struct nlattr nla;
			union {
				uint8_t  chr;
				uint8_t  ipv4[4];
				uint8_t  ipv6[16];
				uint32_t ifindex;
				uint32_t clk;
			} payload;
		} val;
		size_t sz;
		const char *type_str;
		const char *crop_str;
		const char *str;
	} qstate_attrs[] = {
		QSTATE_NLA(BRIDGE_QUERIER_UNSPEC, "BRIDGE_QUERIER_UNSPEC", ipv4,
			   "\"\\xde\\xad\\xfa\"",
			   "\"\\xde\\xad\\xfa\\xce\"",
			   { 0xde, 0xad, 0xfa, 0xce } ),
		QSTATE_NLA(BRIDGE_QUERIER_IP_ADDRESS,
			   "BRIDGE_QUERIER_IP_ADDRESS", ipv4,
			   "\"\\x5d\\xb8\\xd8\"",
			   "inet_addr(\"93.184.216.34\")",
			   { 0x5d, 0xb8, 0xd8, 0x22 } ),
		QSTATE_NLA(BRIDGE_QUERIER_IP_PORT,
			   "BRIDGE_QUERIER_IP_PORT", ifindex,
			   xasprintf("\"\\x%02x\\x%02x\\x%02x\"",
				     (ifindex_lo() >> BE_LE(24, 0)) & 0xff,
				     (ifindex_lo() >> BE_LE(16, 8)) & 0xff,
				     (ifindex_lo() >> BE_LE(8, 16)) & 0xff),
			   IFINDEX_LO_STR,
			   ifindex_lo() ),
		QSTATE_NLA(BRIDGE_QUERIER_IP_OTHER_TIMER,
			   "BRIDGE_QUERIER_IP_OTHER_TIMER", clk,
			   clock_t_str(BE_LE(0xcafefe, 0xfefeed),
				       ARRSZ_PAIR(ip_timer_crop)),
			   clock_t_str(0xcafefeed, ARRSZ_PAIR(ip_timer)),
			   0xcafefeed ),
		QSTATE_NLA(BRIDGE_QUERIER_PAD, "BRIDGE_QUERIER_PAD", ipv4,
			   "\"\\xfa\\xce\\xfe\"",
			   "\"\\xfa\\xce\\xfe\\xed\"",
			   { 0xfa, 0xce, 0xfe, 0xed } ),
		QSTATE_NLA(BRIDGE_QUERIER_IPV6_ADDRESS,
			   "BRIDGE_QUERIER_IPV6_ADDRESS", ipv6,
			   "\"\\xde\\xad\\xfa\\xce\\x80\\x00\\x00\\x00"
			     "\\x00\\x00\\x00\\xad\\x00\\x00\\x00\"",
			   "inet_pton(AF_INET6, \"dead:face:8000::ad:0:ec\")",
			   { 0xde, 0xad, 0xfa, 0xce, 0x80, 0x00, 0x00, 0x00,
			     0x00, 0x00, 0x00, 0xad, 0x00, 0x00, 0x00, 0xec } ),
		QSTATE_NLA(BRIDGE_QUERIER_IPV6_PORT,
			   "BRIDGE_QUERIER_IPV6_PORT", ifindex,
			   BE_LE("\"\\xbb\\x40\\xe6\"", "\"\\x4d\\xe6\\x40\""),
			   "3141592653",
			   3141592653 ),
		QSTATE_NLA(BRIDGE_QUERIER_IPV6_OTHER_TIMER,
			   "BRIDGE_QUERIER_IPV6_OTHER_TIMER", clk,
			   clock_t_str(BE_LE(0xfacebe, 0xcebeef),
				       ARRSZ_PAIR(ipv6_timer_crop)),
			   clock_t_str(0xfacebeef, ARRSZ_PAIR(ipv6_timer)),
			   0xfacebeef ),
		QSTATE_NLA(8, "0x8 /* BRIDGE_QUERIER_??? */", chr,
			   "", "\"\\x69\"", 0x69 ),
	};
	for (size_t k = 0; k < ARRAY_SIZE(qstate_attrs); k++) {
		char crop_str[256];
		char str[256];

		snprintf(crop_str, sizeof(crop_str),
			 "%s{nla_len=%zu, nla_type=%s}%s%s%s",
			 qstate_attrs[k].crop_str[0] ? "[" : "",
			 qstate_attrs[k].sz, qstate_attrs[k].type_str,
			 qstate_attrs[k].crop_str[0] ? ", " : "",
			 qstate_attrs[k].crop_str,
			 qstate_attrs[k].crop_str[0] ? "]" : "");
		snprintf(str, sizeof(str), "[{nla_len=%zu, nla_type=%s}, %s]",
			 qstate_attrs[k].sz, qstate_attrs[k].type_str,
			 qstate_attrs[k].str);
		TEST_NESTED_LINKINFO(fd, nlh0, IFLA_INFO_KIND,
				     2, "IFLA_INFO_DATA", "bridge",
				     IFLA_BR_MCAST_QUERIER_STATE,
				     "IFLA_BR_MCAST_QUERIER_STATE",
				     qstate_attrs[k].val, pattern,
				     { qstate_attrs[k].sz - 1, crop_str },
				     { qstate_attrs[k].sz,     str });
	}

	/* tun attrs */
	static const struct val_name u8_tun_attrs[] = {
		{ 4, "IFLA_TUN_PI" },
		{ 5, "IFLA_TUN_VNET_HDR" },
		{ 6, "IFLA_TUN_PERSIST" },
		{ 7, "IFLA_TUN_MULTI_QUEUE" },
	};

	for (size_t k = 0; k < ARRAY_SIZE(u8_tun_attrs); k++) {
		TEST_NESTED_LINKINFO(fd, nlh0, IFLA_INFO_KIND,
				     2, "IFLA_INFO_DATA", "tun",
				     u8_tun_attrs[k].val, u8_tun_attrs[k].name,
				     u8_val, pattern,
				     { 0, NULL },
				     { 1, "161" },
				     { 2, "161" });
	}

	static const struct val_name u32_tun_attrs[] = {
		{ 8, "IFLA_TUN_NUM_QUEUES" },
		{ 9, "IFLA_TUN_NUM_DISABLED_QUEUES" },
	};

	for (size_t k = 0; k < ARRAY_SIZE(u32_tun_attrs); k++) {
		TEST_NESTED_LINKINFO(fd, nlh0, IFLA_INFO_KIND,
				     2, "IFLA_INFO_DATA", "tun",
				     u32_tun_attrs[k].val,
				     u32_tun_attrs[k].name,
				     u32_val, pattern,
				     { 3, BE_LE("\"\\xba\\xdc\\x0d\"",
						"\"\\xed\\x0d\\xdc\"") },
				     { 4, "3134983661" },
				     { 5, "3134983661" });
	}

	static const struct val_name und_tun_attrs[] = {
		{ 0,  "IFLA_TUN_UNSPEC" },
		{ 10, "0xa /* IFLA_TUN_??? */" },
	};

	for (size_t k = 0; k < ARRAY_SIZE(und_tun_attrs); k++) {
		TEST_NESTED_LINKINFO(fd, nlh0, IFLA_INFO_KIND,
				     2, "IFLA_INFO_DATA", "tun",
				     und_tun_attrs[k].val,
				     und_tun_attrs[k].name,
				     unknown_msg, pattern,
				     { 2, "\"\\xab\\xac\"" },
				     { 4, "\"\\xab\\xac\\xdb\\xcd\"" },
				     { 6,
					"\"\\xab\\xac\\xdb\\xcd\\x61\\x62\"" },
				     { 8, "\"\\xab\\xac\\xdb\\xcd\\x61\\x62"
					"\\x63\\x64\"" },
				     { 10, "\"\\xab\\xac\\xdb\\xcd\\x61\\x62"
					"\\x63\\x64\\x65\\x66\"" });
	}

	static const uint32_t minus_one = 0xffffffffU;
	static const struct val_name uid_tun_attrs[] = {
		{ 1, "IFLA_TUN_OWNER" },
		{ 2, "IFLA_TUN_GROUP" },
	};

	for (size_t k = 0; k < ARRAY_SIZE(uid_tun_attrs); k++) {
		TEST_NESTED_LINKINFO(fd, nlh0, IFLA_INFO_KIND,
				     2, "IFLA_INFO_DATA", "tun",
				     uid_tun_attrs[k].val,
				     uid_tun_attrs[k].name,
				     u32_val, pattern,
				     { 3, BE_LE("\"\\xba\\xdc\\x0d\"",
						"\"\\xed\\x0d\\xdc\"") },
				     { 4, "3134983661" },
				     { 5, "3134983661" });

		TEST_NESTED_LINKINFO(fd, nlh0, IFLA_INFO_KIND,
				     2, "IFLA_INFO_DATA", "tun",
				     uid_tun_attrs[k].val,
				     uid_tun_attrs[k].name,
				     minus_one, pattern,
				     { 3, "\"\\xff\\xff\\xff\"" },
				     { 4, "-1" },
				     { 5, "-1" });
	}

	static const struct {
		uint8_t val;
		const char *str;
	} tun_types[] = {
		{ 0, "0 /* IFF_??? */"},
		{ 1, "IFF_TUN"},
		{ 2, "IFF_TAP"},
		{ 3, "0x3 /* IFF_??? */"},
		{ 0xda, "0xda /* IFF_??? */"},
	};

	for (size_t k = 0; k < ARRAY_SIZE(tun_types); k++) {
		TEST_NESTED_LINKINFO(fd, nlh0, IFLA_INFO_KIND,
				     2, "IFLA_INFO_DATA", "tun",
				     3, "IFLA_TUN_TYPE",
				     tun_types[k].val, pattern,
				     { 0, NULL },
				     { 1, tun_types[k].str },
				     { 2, tun_types[k].str });
	}


	/* IFLA_INFO_KIND + IFLA_INFO_XSTATS */
	TEST_UNKNOWN_TUNNELS(fd, nlh0, IFLA_INFO_KIND,
			     IFLA_INFO_XSTATS, "IFLA_INFO_XSTATS",
			     unknown_msg, sizeof(unknown_msg),
			     {unsupported_tunnel_types COMMA
			     /*
			      * can decoder decodes its data only if it's big
			      * enough.
			      */
			      unsupported_xstats_types COMMA
			      unsupported_data_types COMMA
			      NULL},
			     printf("\"\\xab\\xac\\xdb\\xcd\""));

	uint32_t can_stats_data[] = {
		0xbadc0de0, 0xbadc0de1, 0xbadc0de2, 0xbadc0de3,
		0xbadc0de4, 0xbadc0de5,
	};

	TEST_LINKINFO(fd, nlh0, IFLA_INFO_KIND, IFLA_INFO_XSTATS, "can",
		      can_stats_data, pattern, print_quoted_hex,
		      printf("{bus_error=3134983648"
			     ", error_warning=3134983649"
			     ", error_passive=3134983650"
			     ", bus_off=3134983651"
			     ", arbitration_lost=3134983652"
			     ", restarts=3134983653}"));


	/* IFLA_INFO_KIND + IFLA_INFO_SLAVE_KIND */
	TEST_UNKNOWN_TUNNELS(fd, nlh0, IFLA_INFO_KIND,
			     IFLA_INFO_SLAVE_KIND, "IFLA_INFO_SLAVE_KIND",
			     unknown_msg, sizeof(unknown_msg),
			     {unsupported_tunnel_types COMMA
			      supported_tunnel_types COMMA
			      NULL},
			     printf("\"\\253\\254\\333\\315\"..."));


	/* IFLA_INFO_KIND + IFLA_INFO_SLAVE_DATA */
	TEST_UNKNOWN_TUNNELS(fd, nlh0, IFLA_INFO_KIND,
			     IFLA_INFO_SLAVE_DATA, "IFLA_INFO_SLAVE_DATA",
			     unknown_msg, sizeof(unknown_msg),
			     {unsupported_tunnel_types COMMA
			      supported_tunnel_types COMMA
			      NULL},
			     printf("\"\\xab\\xac\\xdb\\xcd\""));


	/* IFLA_INFO_KIND + unknown type */
	TEST_UNKNOWN_TUNNELS(fd, nlh0, IFLA_INFO_KIND,
			     6, "0x6 /* IFLA_INFO_??? */",
			     unknown_msg, sizeof(unknown_msg),
			     {unsupported_tunnel_types COMMA
			      supported_tunnel_types COMMA
			      NULL},
			     printf("\"\\xab\\xac\\xdb\\xcd\""));


	/* IFLA_INFO_SLAVE_KIND */
	TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
				      init_ifinfomsg, print_ifinfomsg,
				      IFLA_INFO_SLAVE_KIND,
				      "IFLA_INFO_SLAVE_KIND", pattern,
				      unknown_msg, print_quoted_stringn, 1,
				      printf("\"\\253\\254\\333\\315\"..."));


	/* IFLA_INFO_SLAVE_KIND + IFLA_INFO_UNSPEC */
	TEST_UNKNOWN_TUNNELS(fd, nlh0, IFLA_INFO_SLAVE_KIND,
			     IFLA_INFO_UNSPEC, "IFLA_INFO_UNSPEC",
			     unknown_msg, sizeof(unknown_msg),
			     {unsupported_tunnel_types COMMA
			      supported_tunnel_types COMMA
			      NULL},
			     printf("\"\\xab\\xac\\xdb\\xcd\""));


	/* IFLA_INFO_SLAVE_KIND + IFLA_INFO_KIND */
	TEST_UNKNOWN_TUNNELS(fd, nlh0, IFLA_INFO_SLAVE_KIND,
			     IFLA_INFO_KIND, "IFLA_INFO_KIND",
			     unknown_msg, sizeof(unknown_msg),
			     {unsupported_tunnel_types COMMA
			      supported_tunnel_types COMMA
			      NULL},
			     printf("\"\\253\\254\\333\\315\"..."));


	/* IFLA_INFO_SLAVE_KIND + IFLA_INFO_DATA */
	TEST_UNKNOWN_TUNNELS(fd, nlh0, IFLA_INFO_SLAVE_KIND,
			     IFLA_INFO_DATA, "IFLA_INFO_DATA",
			     unknown_msg, sizeof(unknown_msg),
			     {unsupported_tunnel_types COMMA
			      unsupported_data_types COMMA
			      NULL},
			     printf("\"\\xab\\xac\\xdb\\xcd\""));

	/* IFLA_INFO_SLAVE_KIND + IFLA_INFO_SLAVE_DATA */
	TEST_UNKNOWN_TUNNELS(fd, nlh0, IFLA_INFO_SLAVE_KIND,
			     IFLA_INFO_SLAVE_DATA, "IFLA_INFO_SLAVE_DATA",
			     unknown_msg, sizeof(unknown_msg),
			     {unsupported_tunnel_types COMMA
			      unsupported_slave_data_types COMMA
			      NULL},
			     printf("\"\\xab\\xac\\xdb\\xcd\""));

	/* bridge attrs */
	static const struct val_name und_brport_attrs[] = {
		{ 0, "IFLA_BRPORT_UNSPEC" },
		{ 24, "IFLA_BRPORT_FLUSH" },
		{ 26, "IFLA_BRPORT_PAD" },
		{ 45, "0x2d /* IFLA_BRPORT_??? */" },
		{ 2989, "0xbad /* IFLA_BRPORT_??? */" },
	};

	for (size_t k = 0; k < ARRAY_SIZE(und_brport_attrs); k++) {
		TEST_NESTED_LINKINFO(fd, nlh0, IFLA_INFO_SLAVE_KIND,
				     5, "IFLA_INFO_SLAVE_DATA", "bridge",
				     und_brport_attrs[k].val,
				     und_brport_attrs[k].name,
				     unknown_msg, pattern,
				     { 2, "\"\\xab\\xac\"" },
				     { 4, "\"\\xab\\xac\\xdb\\xcd\"" },
				     { 6,
					"\"\\xab\\xac\\xdb\\xcd\\x61\\x62\"" },
				     { 8, "\"\\xab\\xac\\xdb\\xcd\\x61\\x62"
					"\\x63\\x64\"" },
				     { 10, "\"\\xab\\xac\\xdb\\xcd\\x61\\x62"
					"\\x63\\x64\\x65\\x66\"" });
	}

	static const struct val_name u8_brport_attrs[] = {
		{ ARG_STR(IFLA_BRPORT_STATE) },
		{ ARG_STR(IFLA_BRPORT_MODE) },
		{ ARG_STR(IFLA_BRPORT_GUARD) },
		{ ARG_STR(IFLA_BRPORT_PROTECT) },
		{ ARG_STR(IFLA_BRPORT_FAST_LEAVE) },
		{ ARG_STR(IFLA_BRPORT_LEARNING) },
		{ ARG_STR(IFLA_BRPORT_UNICAST_FLOOD) },
		{ ARG_STR(IFLA_BRPORT_PROXYARP) },
		{ ARG_STR(IFLA_BRPORT_LEARNING_SYNC) },
		{ ARG_STR(IFLA_BRPORT_PROXYARP_WIFI) },
		{ ARG_STR(IFLA_BRPORT_TOPOLOGY_CHANGE_ACK) },
		{ ARG_STR(IFLA_BRPORT_CONFIG_PENDING) },
		{ ARG_STR(IFLA_BRPORT_MULTICAST_ROUTER) },
		{ ARG_STR(IFLA_BRPORT_MCAST_FLOOD) },
		{ ARG_STR(IFLA_BRPORT_MCAST_TO_UCAST) },
		{ ARG_STR(IFLA_BRPORT_VLAN_TUNNEL) },
		{ ARG_STR(IFLA_BRPORT_BCAST_FLOOD) },
		{ ARG_STR(IFLA_BRPORT_NEIGH_SUPPRESS) },
		{ ARG_STR(IFLA_BRPORT_ISOLATED) },
		{ ARG_STR(IFLA_BRPORT_MRP_RING_OPEN) },
		{ ARG_STR(IFLA_BRPORT_MRP_IN_OPEN) },
		{ ARG_STR(IFLA_BRPORT_LOCKED) },
		{ ARG_STR(IFLA_BRPORT_MAB) },
		{ ARG_STR(IFLA_BRPORT_NEIGH_VLAN_SUPPRESS) },
	};

	for (size_t k = 0; k < ARRAY_SIZE(u8_brport_attrs); k++) {
		TEST_NESTED_LINKINFO(fd, nlh0, IFLA_INFO_SLAVE_KIND,
				     5, "IFLA_INFO_SLAVE_DATA", "bridge",
				     u8_brport_attrs[k].val,
				     u8_brport_attrs[k].name,
				     u8_val, pattern,
				     { 0, NULL },
				     { 1, "161" },
				     { 2, "161" });
	}

	static const struct val_name u16_brport_attrs[] = {
		{ ARG_STR(IFLA_BRPORT_PRIORITY) },
		{ ARG_STR(IFLA_BRPORT_DESIGNATED_PORT) },
		{ ARG_STR(IFLA_BRPORT_DESIGNATED_COST) },
		{ ARG_STR(IFLA_BRPORT_ID) },
		{ ARG_STR(IFLA_BRPORT_NO) },
	};

	for (size_t k = 0; k < ARRAY_SIZE(u16_brport_attrs); k++) {
		TEST_NESTED_LINKINFO(fd, nlh0, IFLA_INFO_SLAVE_KIND,
				     5, "IFLA_INFO_SLAVE_DATA", "bridge",
				     u16_brport_attrs[k].val,
				     u16_brport_attrs[k].name,
				     u16_val, pattern,
				     { 1, "\"" BE_LE("\\xde", "\\xed") "\"" },
				     { 2, "57069" },
				     { 3, "57069" });
	}

	static const struct val_name x16_brport_attrs[] = {
		{ ARG_STR(IFLA_BRPORT_GROUP_FWD_MASK) },
	};

	for (size_t k = 0; k < ARRAY_SIZE(x16_brport_attrs); k++) {
		TEST_NESTED_LINKINFO(fd, nlh0, IFLA_INFO_SLAVE_KIND,
				     5, "IFLA_INFO_SLAVE_DATA", "bridge",
				     x16_brport_attrs[k].val,
				     x16_brport_attrs[k].name,
				     u16_val, pattern,
				     { 1, "\"" BE_LE("\\xde", "\\xed") "\"" },
				     { 2, "0xdeed" },
				     { 3, "0xdeed" });
	}

	static const struct val_name u32_brport_attrs[] = {
		{  3, "IFLA_BRPORT_COST" },
		{ 37, "IFLA_BRPORT_MCAST_EHT_HOSTS_LIMIT" },
		{ 38, "IFLA_BRPORT_MCAST_EHT_HOSTS_CNT" },
		{ ARG_STR(IFLA_BRPORT_MCAST_N_GROUPS) },
		{ ARG_STR(IFLA_BRPORT_MCAST_MAX_GROUPS) },
		{ ARG_STR(IFLA_BRPORT_BACKUP_NHID) },
	};

	for (size_t k = 0; k < ARRAY_SIZE(u32_brport_attrs); k++) {
		TEST_NESTED_LINKINFO(fd, nlh0, IFLA_INFO_SLAVE_KIND,
				     5, "IFLA_INFO_SLAVE_DATA", "bridge",
				     u32_brport_attrs[k].val,
				     u32_brport_attrs[k].name,
				     u32_val, pattern,
				     { 3, BE_LE("\"\\xba\\xdc\\x0d\"",
						"\"\\xed\\x0d\\xdc\"") },
				     { 4, "3134983661" },
				     { 5, "3134983661" });
	}

	static const struct val_name brport_id_attrs[] = {
		{ 13, "IFLA_BRPORT_ROOT_ID" },
		{ 14, "IFLA_BRPORT_BRIDGE_ID" },
	};

	for (size_t k = 0; k < ARRAY_SIZE(brport_id_attrs); k++) {
		TEST_NESTED_LINKINFO(fd, nlh0, IFLA_INFO_SLAVE_KIND,
				     5, "IFLA_INFO_SLAVE_DATA", "bridge",
				     brport_id_attrs[k].val,
				     brport_id_attrs[k].name,
				     bridge_id, pattern,
				     { 7, "\"\\xbe\\xef\\xfa\\xce"
					  "\\xde\\xc0\\xde\"" },
				     { 8, "{prio=[190, 239]"
					  ", addr=fa:ce:de:c0:de:ad}" },
				     { 9, "{prio=[190, 239]"
					  ", addr=fa:ce:de:c0:de:ad}" });
	}

	static const struct val_name c_t_brport_attrs[] = {
		{ 21, "IFLA_BRPORT_MESSAGE_AGE_TIMER" },
		{ 22, "IFLA_BRPORT_FORWARD_DELAY_TIMER" },
		{ 23, "IFLA_BRPORT_HOLD_TIMER" },
	};

	for (size_t k = 0; k < ARRAY_SIZE(c_t_brport_attrs); k++) {
		TEST_NESTED_LINKINFO(fd, nlh0, IFLA_INFO_SLAVE_KIND,
				     5, "IFLA_INFO_SLAVE_DATA", "bridge",
				     c_t_brport_attrs[k].val,
				     c_t_brport_attrs[k].name,
				     u64_val, pattern,
				     { 7, sz7_str },
				     { 8, sz8_str },
				     { 9, "\"" BE_LE("\\xde\\xad\\xc0\\xde"
						     "\\xfa\\xce\\xfe\\xed",
						     "\\xed\\xfe\\xce\\xfa"
						     "\\xde\\xc0\\xad\\xde")
					  "\\x61\"" });
	}

	static const struct val_name ifidx_brport_attrs[] = {
		{ 34, "IFLA_BRPORT_BACKUP_PORT" },
	};
	const uint32_t ifidx_lo = ifindex_lo();

	for (size_t k = 0; k < ARRAY_SIZE(ifidx_brport_attrs); k++) {
		TEST_NESTED_LINKINFO(fd, nlh0, IFLA_INFO_SLAVE_KIND,
				     5, "IFLA_INFO_SLAVE_DATA", "bridge",
				     ifidx_brport_attrs[k].val,
				     ifidx_brport_attrs[k].name,
				     u32_val, pattern,
				     { 3, BE_LE("\"\\xba\\xdc\\x0d\"",
						"\"\\xed\\x0d\\xdc\"") },
				     { 4, "3134983661" },
				     { 5, "3134983661" });

		TEST_NESTED_LINKINFO(fd, nlh0, IFLA_INFO_SLAVE_KIND,
				     5, "IFLA_INFO_SLAVE_DATA", "bridge",
				     ifidx_brport_attrs[k].val,
				     ifidx_brport_attrs[k].name,
				     ifidx_lo, pattern,
				     { 3, BE_LE("\"\\x00\\x00\\x00\"",
						"\"\\x01\\x00\\x00\"") },
				     { 4, IFINDEX_LO_STR },
				     { 5, IFINDEX_LO_STR });
	}

	/* IFLA_INFO_SLAVE_KIND + unknown type */
	TEST_UNKNOWN_TUNNELS(fd, nlh0, IFLA_INFO_SLAVE_KIND,
			     6, "0x6 /* IFLA_INFO_??? */",
			     unknown_msg, sizeof(unknown_msg),
			     {unsupported_tunnel_types COMMA
			      supported_tunnel_types COMMA
			      NULL},
			     printf("\"\\xab\\xac\\xdb\\xcd\""));


	puts("+++ exited with 0 +++");
	return 0;
}
