/*
 * IFLA_LINKINFO netlink attribute decoding check.
 *
 * Copyright (c) 2018-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <inttypes.h>
#include <stdio.h>
#include <stddef.h>
#include <arpa/inet.h>

#include "test_nlattr.h"

#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/if_link.h>
#include <linux/rtnetlink.h>

#define XLAT_MACROS_ONLY
#include <xlat/rtnl_link_attrs.h>
#include <xlat/rtnl_ifla_info_attrs.h>
#undef XLAT_MACROS_ONLY

#define IFLA_ATTR IFLA_LINKINFO
#include "nlattr_ifla.h"

#define COMMA ,
#define TEST_UNKNOWN_TUNNELS(fd_, nlh0_, objtype_, objtype_str_,	\
			     obj_, objsz_, arrstrs_, ...)		\
	do {								\
		/* 64 is guestimate for maximum unknown type len */	\
		char buf[8 * 2 + 64 + objsz_];				\
		const char **arrstrs[] = arrstrs_;			\
		const char ***arrstrs_pos = arrstrs;			\
		const char **arrstr = *arrstrs_pos;			\
		const char *type = NULL;				\
									\
		for (type = arrstr ? arrstr[0] : NULL; type && arrstr;	\
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
					IFLA_INFO_KIND, "IFLA_INFO_KIND", \
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

#define TEST_LINKINFO_(fd_, nlh0_, nla_type_, nla_type_str_, tuntype_,	\
		       obj_, objsz_, pattern_, fallback_func_, ...)	\
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
					IFLA_INFO_KIND, "IFLA_INFO_KIND", \
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
				IFLA_INFO_KIND, "IFLA_INFO_KIND",	\
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
				IFLA_INFO_KIND, "IFLA_INFO_KIND",	\
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

#define TEST_LINKINFO(fd_, nlh0_, nla_type_, tuntype_,	\
		      obj_, pattern_, fallback_func_, ...)	\
	TEST_LINKINFO_((fd_), (nlh0_), nla_type_, #nla_type_, (tuntype_), \
		       (obj_), sizeof(obj_), pattern_, fallback_func_,	\
		       __VA_ARGS__)

#define TEST_NESTED_LINKINFO(fd_, nlh0_,				\
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
				IFLA_INFO_KIND, "IFLA_INFO_KIND",	\
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
	static const char *unsupported_tunnel_types[] = {
		"batadv", "bond",
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
		NULL
	};
	static const char *unsupported_xstats_types[] = {
		"bridge",
		"tun",
		NULL
	};
	static const char *unsupported_data_types[] = {
		"can",
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
	TEST_UNKNOWN_TUNNELS(fd, nlh0, IFLA_INFO_UNSPEC, "IFLA_INFO_UNSPEC",
			     unknown_msg, sizeof(unknown_msg),
			     {unsupported_tunnel_types COMMA
			      unsupported_xstats_types COMMA
			      unsupported_data_types COMMA
			      NULL},
			     printf("\"\\xab\\xac\\xdb\\xcd\""));


	/* IFLA_INFO_KIND + IFLA_INFO_KIND */
	TEST_UNKNOWN_TUNNELS(fd, nlh0, IFLA_INFO_KIND, "IFLA_INFO_KIND",
			     unknown_msg, sizeof(unknown_msg),
			     {unsupported_tunnel_types COMMA
			      unsupported_xstats_types COMMA
			      unsupported_data_types COMMA
			      NULL},
			     printf("\"\\253\\254\\333\\315\"..."));


	/* IFLA_INFO_KIND + IFLA_INFO_DATA */
	TEST_UNKNOWN_TUNNELS(fd, nlh0, IFLA_INFO_DATA, "IFLA_INFO_DATA",
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
		{ 47, "0x2f /* IFLA_BR_??? */" },
	};

	for (size_t k = 0; k < ARRAY_SIZE(und_br_attrs); k++) {
		TEST_NESTED_LINKINFO(fd, nlh0, 2, "IFLA_INFO_DATA", "bridge",
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
		TEST_NESTED_LINKINFO(fd, nlh0, 2, "IFLA_INFO_DATA", "bridge",
				     hwa_br_attrs[k].val, hwa_br_attrs[k].name,
				     unknown_msg, pattern,
				     { 2, "ab:ac" },
				     { 4, "ab:ac:db:cd" },
				     { 6, "ab:ac:db:cd:61:62" },
				     { 8, "ab:ac:db:cd:61:62:63:64" },
				     { 10, "ab:ac:db:cd:61:62:63:64:65:66" });
	}

	static const struct val_name u64_br_attrs[] = {
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

	for (size_t k = 0; k < ARRAY_SIZE(u64_br_attrs); k++) {
		TEST_NESTED_LINKINFO(fd, nlh0, 2, "IFLA_INFO_DATA", "bridge",
				     u64_br_attrs[k].val, u64_br_attrs[k].name,
				     u64_val, pattern,
				     { 7, "\"" BE_LE(
					"\\xde\\xad\\xc0\\xde\\xfa\\xce\\xfe",
					"\\xed\\xfe\\xce\\xfa\\xde\\xc0\\xad")
					"\"" },
				     { 8, "16045693111314087661" },
				     { 9, "16045693111314087661" });
	}

	static const struct val_name u32_br_attrs[] = {
		{  1, "IFLA_BR_FORWARD_DELAY" },
		{  2, "IFLA_BR_HELLO_TIME" },
		{  3, "IFLA_BR_MAX_AGE" },
		{  4, "IFLA_BR_AGEING_TIME" },
		{  5, "IFLA_BR_STP_STATE" },
		{ 13, "IFLA_BR_ROOT_PATH_COST" },
		{ 26, "IFLA_BR_MCAST_HASH_ELASTICITY" },
		{ 27, "IFLA_BR_MCAST_HASH_MAX" },
		{ 28, "IFLA_BR_MCAST_LAST_MEMBER_CNT" },
		{ 29, "IFLA_BR_MCAST_STARTUP_QUERY_CNT" },
	};

	for (size_t k = 0; k < ARRAY_SIZE(u32_br_attrs); k++) {
		TEST_NESTED_LINKINFO(fd, nlh0, 2, "IFLA_INFO_DATA", "bridge",
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
		TEST_NESTED_LINKINFO(fd, nlh0, 2, "IFLA_INFO_DATA", "bridge",
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
		TEST_NESTED_LINKINFO(fd, nlh0, 2, "IFLA_INFO_DATA", "bridge",
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
		TEST_NESTED_LINKINFO(fd, nlh0, 2, "IFLA_INFO_DATA", "bridge",
				     u8_br_attrs[k].val, u8_br_attrs[k].name,
				     u8_val, pattern,
				     { 0, NULL },
				     { 1, "161" },
				     { 2, "161" });
	}

	unsigned short eth_p = htons(0x88C7);
	TEST_NESTED_LINKINFO(fd, nlh0, 2, "IFLA_INFO_DATA", "bridge",
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
		TEST_NESTED_LINKINFO(fd, nlh0, 2, "IFLA_INFO_DATA", "bridge",
				     br_id_attrs[k].val, br_id_attrs[k].name,
				     bridge_id, pattern,
				     { 7, "\"\\xbe\\xef\\xfa\\xce"
					  "\\xde\\xc0\\xde\"" },
				     { 8, "{prio=[190, 239]"
					  ", addr=fa:ce:de:c0:de:ad}" },
				     { 9, "{prio=[190, 239]"
					  ", addr=fa:ce:de:c0:de:ad}" });
	}

	/* tun attrs */
	static const struct val_name u8_tun_attrs[] = {
		{ 4, "IFLA_TUN_PI" },
		{ 5, "IFLA_TUN_VNET_HDR" },
		{ 6, "IFLA_TUN_PERSIST" },
		{ 7, "IFLA_TUN_MULTI_QUEUE" },
	};

	for (size_t k = 0; k < ARRAY_SIZE(u8_tun_attrs); k++) {
		TEST_NESTED_LINKINFO(fd, nlh0, 2, "IFLA_INFO_DATA", "tun",
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
		TEST_NESTED_LINKINFO(fd, nlh0, 2, "IFLA_INFO_DATA", "tun",
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
		TEST_NESTED_LINKINFO(fd, nlh0, 2, "IFLA_INFO_DATA", "tun",
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
		TEST_NESTED_LINKINFO(fd, nlh0, 2, "IFLA_INFO_DATA", "tun",
				     uid_tun_attrs[k].val,
				     uid_tun_attrs[k].name,
				     u32_val, pattern,
				     { 3, BE_LE("\"\\xba\\xdc\\x0d\"",
						"\"\\xed\\x0d\\xdc\"") },
				     { 4, "3134983661" },
				     { 5, "3134983661" });

		TEST_NESTED_LINKINFO(fd, nlh0, 2, "IFLA_INFO_DATA", "tun",
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
		TEST_NESTED_LINKINFO(fd, nlh0, 2, "IFLA_INFO_DATA", "tun",
				     3, "IFLA_TUN_TYPE",
				     tun_types[k].val, pattern,
				     { 0, NULL },
				     { 1, tun_types[k].str },
				     { 2, tun_types[k].str });
	}


	/* IFLA_INFO_KIND + IFLA_INFO_XSTATS */
	TEST_UNKNOWN_TUNNELS(fd, nlh0, IFLA_INFO_XSTATS, "IFLA_INFO_XSTATS",
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

	TEST_LINKINFO(fd, nlh0, IFLA_INFO_XSTATS, "can",
		      can_stats_data, pattern, print_quoted_hex,
		      printf("{bus_error=3134983648"
			     ", error_warning=3134983649"
			     ", error_passive=3134983650"
			     ", bus_off=3134983651"
			     ", arbitration_lost=3134983652"
			     ", restarts=3134983653}"));


	/* IFLA_INFO_KIND + IFLA_INFO_SLVAE_KIND */
	TEST_UNKNOWN_TUNNELS(fd, nlh0,
			     IFLA_INFO_SLAVE_KIND, "IFLA_INFO_SLAVE_KIND",
			     unknown_msg, sizeof(unknown_msg),
			     {unsupported_tunnel_types COMMA
			      unsupported_xstats_types COMMA
			      unsupported_data_types COMMA
			      NULL},
			     printf("\"\\253\\254\\333\\315\"..."));


	/* IFLA_INFO_KIND + IFLA_INFO_SLAVE_DATA */
	TEST_UNKNOWN_TUNNELS(fd, nlh0,
			     IFLA_INFO_SLAVE_DATA, "IFLA_INFO_SLAVE_DATA",
			     unknown_msg, sizeof(unknown_msg),
			     {unsupported_tunnel_types COMMA
			      unsupported_xstats_types COMMA
			      unsupported_data_types COMMA
			      NULL},
			     printf("\"\\xab\\xac\\xdb\\xcd\""));


	/* IFLA_INFO_KIND + unknown type */
	TEST_UNKNOWN_TUNNELS(fd, nlh0, 6, "0x6 /* IFLA_INFO_??? */",
			     unknown_msg, sizeof(unknown_msg),
			     {unsupported_tunnel_types COMMA
			      unsupported_xstats_types COMMA
			      unsupported_data_types COMMA
			      NULL},
			     printf("\"\\xab\\xac\\xdb\\xcd\""));


	puts("+++ exited with 0 +++");
	return 0;
}
