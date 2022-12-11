/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2022 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <linux/atalk.h>
#include <linux/mptcp.h>
#include <linux/tls.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include "test_nlattr.h"
#include <linux/inet_diag.h>
#include <linux/sock_diag.h>


#ifndef HAVE_STRUCT_TCP_DIAG_MD5SIG
struct tcp_diag_md5sig {
	__u8	tcpm_family;
	__u8	tcpm_prefixlen;
	__u16	tcpm_keylen;
	__be32	tcpm_addr[4];
	__u8	tcpm_key[80 /* TCP_MD5SIG_MAXKEYLEN */];
};
#endif

static const char * const sk_meminfo_strs[] = {
	"SK_MEMINFO_RMEM_ALLOC",
	"SK_MEMINFO_RCVBUF",
	"SK_MEMINFO_WMEM_ALLOC",
	"SK_MEMINFO_SNDBUF",
	"SK_MEMINFO_FWD_ALLOC",
	"SK_MEMINFO_WMEM_QUEUED",
	"SK_MEMINFO_OPTMEM",
	"SK_MEMINFO_BACKLOG",
	"SK_MEMINFO_DROPS",
};

static const char address[] = "10.11.12.13";
static const unsigned int hdrlen = sizeof(struct inet_diag_msg);
static uint16_t  attr1;
static const char *attr1_str = NULL;
static uint16_t  attr2;
static const char *attr2_str = NULL;


static void
init_inet_diag_msg(struct nlmsghdr *const nlh, const unsigned int msg_len)
{
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = msg_len,
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_DUMP
	);

	struct inet_diag_msg *const msg = NLMSG_DATA(nlh);
	SET_STRUCT(struct inet_diag_msg, msg,
		.idiag_family = AF_INET,
		.idiag_state = TCP_LISTEN,
		.id.idiag_if = ifindex_lo()
	);

	if (!inet_pton(AF_INET, address, msg->id.idiag_src) ||
	    !inet_pton(AF_INET, address, msg->id.idiag_dst))
		perror_msg_and_skip("inet_pton");
}

static void
print_inet_diag_msg(const unsigned int msg_len)
{
	printf("{nlmsg_len=%u, nlmsg_type=SOCK_DIAG_BY_FAMILY"
	       ", nlmsg_flags=NLM_F_DUMP, nlmsg_seq=0, nlmsg_pid=0}"
	       ", {idiag_family=AF_INET, idiag_state=TCP_LISTEN"
	       ", idiag_timer=0, idiag_retrans=0"
	       ", id={idiag_sport=htons(0), idiag_dport=htons(0)"
	       ", idiag_src=inet_addr(\"%s\")"
	       ", idiag_dst=inet_addr(\"%s\")"
	       ", idiag_if=" IFINDEX_LO_STR
	       ", idiag_cookie=[0, 0]}"
	       ", idiag_expires=0, idiag_rqueue=0, idiag_wqueue=0"
	       ", idiag_uid=0, idiag_inode=0}",
	       msg_len, address, address);
}

static void
init_inet_diag_nest_1(struct nlmsghdr *const nlh, const unsigned int msg_len)
{
	init_inet_diag_msg(nlh, msg_len);

	struct nlattr *nla = NLMSG_ATTR(nlh, hdrlen);
	SET_STRUCT(struct nlattr, nla,
		.nla_len = msg_len - NLMSG_SPACE(hdrlen),
		.nla_type = attr1,
	);
}

static void
print_inet_diag_nest_1(const unsigned int msg_len)
{
	print_inet_diag_msg(msg_len);
	printf(", [{nla_len=%u, nla_type=%s}",
	       msg_len - NLMSG_SPACE(hdrlen), attr1_str);
}

static void
init_inet_diag_nest_2(struct nlmsghdr *const nlh, const unsigned int msg_len)
{
	init_inet_diag_nest_1(nlh, msg_len);

	struct nlattr *nla = NLMSG_ATTR(nlh, hdrlen);
	nla += 1;
	SET_STRUCT(struct nlattr, nla,
		.nla_len = msg_len - NLMSG_SPACE(hdrlen) - NLA_HDRLEN,
		.nla_type = attr2,
	);
}

static void
print_inet_diag_nest_2(const unsigned int msg_len)
{
	print_inet_diag_nest_1(msg_len);
	printf(", [{nla_len=%u, nla_type=%s}",
	       msg_len - NLMSG_SPACE(hdrlen) - NLA_HDRLEN, attr2_str);
}

static void
print_uint(const unsigned int *p, size_t i)
{
	if (i >= ARRAY_SIZE(sk_meminfo_strs))
		printf("[%zu /* SK_MEMINFO_??? */", i);
	else
		printf("[%s", sk_meminfo_strs[i]);

	printf("]=%u", *p);
}

static const struct {
	struct tcp_diag_md5sig val;
	const char *str;
} md5sig_vecs[] = {
	{ { 0 },
	  "{tcpm_family=AF_UNSPEC, tcpm_prefixlen=0, tcpm_keylen=0"
	  ", tcpm_addr=\"\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00"
	  "\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\", tcpm_key=\"\"}" },
	{ { AF_INET, 0x42, 1, { BE_LE(0xdeadface, 0xcefaadde) } },
	  "{tcpm_family=AF_INET, tcpm_prefixlen=66, tcpm_keylen=1"
	  ", tcpm_addr=inet_addr(\"222.173.250.206\")"
	  ", tcpm_key=\"\\x00\"}" },
	{ { AF_INET6, 0xbe, 42,
	    { BE_LE(0xdeadface, 0xcefaadde), BE_LE(0xcafe0000, 0xfeca),
	      BE_LE(0xface, 0xcefa0000), BE_LE(0xbadc0ded, 0xed0ddcba) },
	    "OH HAI THAR\0\1\2\3\4\5\6\7\3779876543210abcdefghijklmnopqrstuv" },
	  "{tcpm_family=AF_INET6, tcpm_prefixlen=190, tcpm_keylen=42"
	  ", inet_pton(AF_INET6, \"dead:face:cafe::face:badc:ded\", &tcpm_addr)"
	  ", tcpm_key=\"\\x4f\\x48\\x20\\x48\\x41\\x49\\x20\\x54\\x48\\x41"
	  "\\x52\\x00\\x01\\x02\\x03\\x04\\x05\\x06\\x07\\xff\\x39\\x38\\x37"
	  "\\x36\\x35\\x34\\x33\\x32\\x31\\x30\\x61\\x62\\x63\\x64\\x65\\x66"
	  "\\x67\\x68\\x69\\x6a\\x6b\\x6c\"}" },
	{ { 46, 0, 45067,
	    { BE_LE(0xdeadface, 0xcefaadde), BE_LE(0xcafe0000, 0xfeca),
	      BE_LE(0xface, 0xcefa0000), BE_LE(0xbadc0ded, 0xed0ddcba) },
	    "OH HAI THAR\0\1\2\3\4\5\6\7\3779876543210abcdefghijklmnopqrstuv"
	    "xyz0123456789ABCDEFGHIJKLMNO" },
	  "{tcpm_family=0x2e /* AF_??? */, tcpm_prefixlen=0, tcpm_keylen=45067"
	  ", tcpm_addr=\"\\xde\\xad\\xfa\\xce\\xca\\xfe\\x00\\x00"
	  "\\x00\\x00\\xfa\\xce\\xba\\xdc\\x0d\\xed\""
	  ", tcpm_key=\"\\x4f\\x48\\x20\\x48\\x41\\x49\\x20\\x54\\x48\\x41"
	  "\\x52\\x00\\x01\\x02\\x03\\x04\\x05\\x06\\x07\\xff\\x39\\x38\\x37"
	  "\\x36\\x35\\x34\\x33\\x32\\x31\\x30\\x61\\x62\\x63\\x64\\x65\\x66"
	  "\\x67\\x68\\x69\\x6a\\x6b\\x6c\\x6d\\x6e\\x6f\\x70\\x71\\x72\\x73"
	  "\\x74\\x75\\x76\\x78\\x79\\x7a\\x30\\x31\\x32\\x33\\x34\\x35\\x36"
	  "\\x37\\x38\\x39\\x41\\x42\\x43\\x44\\x45\\x46\\x47\\x48\\x49\\x4a"
	  "\\x4b\\x4c\\x4d\\x4e\\x4f\"}" },
};

static void
print_md5sig(const struct tcp_diag_md5sig *p, size_t i)
{
	printf("%s", md5sig_vecs[i].str);
}

static void
print_sa(const struct sockaddr_storage *p, size_t i)
{
	static const char *strs[] = {
		"{sa_family=AF_INET, sin_port=htons(42069)"
		", sin_addr=inet_addr(\"18.52.86.120\")}",
		"{sa_family=AF_INET6, sin6_port=htons(23456)"
		", sin6_flowinfo=htonl(324508639)"
		", inet_pton(AF_INET6, \"1234:5678::9abc:def0\", &sin6_addr)"
		", sin6_scope_id=610839776}",
		"{sa_family=AF_APPLETALK"
		", sa_data=\"i\\0" BE_LE("\\207e", "e\\207") "B\\0\\0\\0\\0\\0"
		"\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0"
		"\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0"
		"\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0"
		"\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0"
		"\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0"
		"\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\"}"
	};

	printf("%s", strs[i]);
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	static const struct inet_diag_meminfo minfo = {
		.idiag_rmem = 0xfadcacdb,
		.idiag_wmem = 0xbdabcada,
		.idiag_fmem = 0xbadbfafb,
		.idiag_tmem = 0xfdacdadf
	};
	static const struct tcpvegas_info vegas = {
		.tcpv_enabled = 0xfadcacdb,
		.tcpv_rttcnt = 0xbdabcada,
		.tcpv_rtt = 0xbadbfafb,
		.tcpv_minrtt = 0xfdacdadf
	};
	static const struct tcp_dctcp_info dctcp = {
		.dctcp_enabled = 0xfdac,
		.dctcp_ce_state = 0xfadc,
		.dctcp_alpha = 0xbdabcada,
		.dctcp_ab_ecn = 0xbadbfafb,
		.dctcp_ab_tot = 0xfdacdadf
	};
	static const struct tcp_bbr_info bbr = {
		.bbr_bw_lo = 0xfdacdadf,
		.bbr_bw_hi = 0xfadcacdb,
		.bbr_min_rtt = 0xbdabcada,
		.bbr_pacing_gain = 0xbadbfafb,
		.bbr_cwnd_gain = 0xfdacdadf
	};
	static const uint32_t mem[] = { 0xaffacbad, 0xffadbcab };
	static uint32_t bigmem[SK_MEMINFO_VARS + 1];
	static const uint32_t mark = 0xabdfadca;

	const int fd = create_nl_socket(NETLINK_SOCK_DIAG);
	void *const nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen),
					 NLA_HDRLEN +
					 MAX(sizeof(bigmem), DEFAULT_STRLEN));

	static char pattern[4096];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);

	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_inet_diag_msg, print_inet_diag_msg,
			   INET_DIAG_MEMINFO, pattern, minfo,
			   printf("{");
			   PRINT_FIELD_U(minfo, idiag_rmem);
			   printf(", ");
			   PRINT_FIELD_U(minfo, idiag_wmem);
			   printf(", ");
			   PRINT_FIELD_U(minfo, idiag_fmem);
			   printf(", ");
			   PRINT_FIELD_U(minfo, idiag_tmem);
			   printf("}"));

	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_inet_diag_msg, print_inet_diag_msg,
			   INET_DIAG_VEGASINFO, pattern, vegas,
			   printf("{");
			   PRINT_FIELD_U(vegas, tcpv_enabled);
			   printf(", ");
			   PRINT_FIELD_U(vegas, tcpv_rttcnt);
			   printf(", ");
			   PRINT_FIELD_U(vegas, tcpv_rtt);
			   printf(", ");
			   PRINT_FIELD_U(vegas, tcpv_minrtt);
			   printf("}"));


	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_inet_diag_msg, print_inet_diag_msg,
			   INET_DIAG_DCTCPINFO, pattern, dctcp,
			   printf("{");
			   PRINT_FIELD_U(dctcp, dctcp_enabled);
			   printf(", ");
			   PRINT_FIELD_U(dctcp, dctcp_ce_state);
			   printf(", ");
			   PRINT_FIELD_U(dctcp, dctcp_alpha);
			   printf(", ");
			   PRINT_FIELD_U(dctcp, dctcp_ab_ecn);
			   printf(", ");
			   PRINT_FIELD_U(dctcp, dctcp_ab_tot);
			   printf("}"));

	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_inet_diag_msg, print_inet_diag_msg,
			   INET_DIAG_BBRINFO, pattern, bbr,
			   printf("{");
			   PRINT_FIELD_X(bbr, bbr_bw_lo);
			   printf(", ");
			   PRINT_FIELD_X(bbr, bbr_bw_hi);
			   printf(", ");
			   PRINT_FIELD_U(bbr, bbr_min_rtt);
			   printf(", ");
			   PRINT_FIELD_U(bbr, bbr_pacing_gain);
			   printf(", ");
			   PRINT_FIELD_U(bbr, bbr_cwnd_gain);
			   printf("}"));

	TEST_NLATTR_ARRAY(fd, nlh0, hdrlen,
			  init_inet_diag_msg, print_inet_diag_msg,
			  INET_DIAG_SKMEMINFO, pattern, mem, print_uint);

	memcpy(bigmem, pattern, sizeof(bigmem));

	TEST_NLATTR_ARRAY(fd, nlh0, hdrlen,
			  init_inet_diag_msg, print_inet_diag_msg,
			  INET_DIAG_SKMEMINFO, pattern, bigmem, print_uint);

	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_inet_diag_msg, print_inet_diag_msg,
			   INET_DIAG_MARK, pattern, mark,
			   printf("%u", mark));

	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_inet_diag_msg, print_inet_diag_msg,
			   INET_DIAG_CLASS_ID, pattern, mark,
			   printf("%u", mark));

	static const struct strval8 shutdown_vecs[] = {
		{ ARG_STR(0) },
		{ 1, "0x1 /* RCV_SHUTDOWN */" },
		{ 2, "0x2 /* SEND_SHUTDOWN */" },
		{ 3, "0x3 /* RCV_SHUTDOWN|SEND_SHUTDOWN */" },
		{ 4, "0x4 /* ???_SHUTDOWN */" },
		{ 23, "0x17 /* RCV_SHUTDOWN|SEND_SHUTDOWN|0x14 */" },
		{ 252, "0xfc /* ???_SHUTDOWN */" },
	};
	TAIL_ALLOC_OBJECT_CONST_PTR(uint8_t, shutdown);
	for (size_t i = 0; i < ARRAY_SIZE(shutdown_vecs); i++) {
		*shutdown = shutdown_vecs[i].val;
		TEST_NLATTR(fd, nlh0, hdrlen,
			    init_inet_diag_msg, print_inet_diag_msg,
			    INET_DIAG_SHUTDOWN,
			    sizeof(*shutdown), shutdown, sizeof(*shutdown),
			    printf("%s", shutdown_vecs[i].str));
	}

	char *const str = tail_alloc(DEFAULT_STRLEN);
	fill_memory_ex(str, DEFAULT_STRLEN, '0', 10);
	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_inet_diag_msg, print_inet_diag_msg, INET_DIAG_CONG,
		    DEFAULT_STRLEN, str, DEFAULT_STRLEN,
		    printf("\"%.*s\"...", DEFAULT_STRLEN, str));
	str[DEFAULT_STRLEN - 1] = '\0';
	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_inet_diag_msg, print_inet_diag_msg, INET_DIAG_CONG,
		    DEFAULT_STRLEN, str, DEFAULT_STRLEN,
		    printf("\"%s\"", str));

	/* u8 INET_DIAG_* attrs */
	static const struct strval16 u8_attrs[] = {
		{ ENUM_KNOWN(0x5, INET_DIAG_TOS) },
		{ ENUM_KNOWN(0x6, INET_DIAG_TCLASS) },
		{ ENUM_KNOWN(0xb, INET_DIAG_SKV6ONLY) },
	};
	void *nlh_u8 = midtail_alloc(NLMSG_SPACE(hdrlen), sizeof(uint8_t));

	for (size_t i = 0; i < ARRAY_SIZE(u8_attrs); i++) {
		check_u8_nlattr(fd, nlh_u8, hdrlen,
				init_inet_diag_msg, print_inet_diag_msg,
				u8_attrs[i].val, u8_attrs[i].str, pattern, 0);
	}

	/* u32 INET_DIAG_* attrs */
	static const struct strval16 u32_attrs[] = {
		{ ENUM_KNOWN(0xf, INET_DIAG_MARK) },
		{ ENUM_KNOWN(0x11, INET_DIAG_CLASS_ID) },
	};
	void *nlh_u32 = midtail_alloc(NLMSG_SPACE(hdrlen), sizeof(uint32_t));

	for (size_t i = 0; i < ARRAY_SIZE(u32_attrs); i++) {
		check_u32_nlattr(fd, nlh_u32, hdrlen,
				 init_inet_diag_msg, print_inet_diag_msg,
				 u32_attrs[i].val, u32_attrs[i].str,
				 pattern, 0);
	}

	/* u64 INET_DIAG_* attrs */
	static const struct strval16 u64_attrs[] = {
		{ ENUM_KNOWN(0x15, INET_DIAG_CGROUP_ID) },
	};
	void *nlh_u64 = midtail_alloc(NLMSG_SPACE(hdrlen), sizeof(uint64_t));

	for (size_t i = 0; i < ARRAY_SIZE(u64_attrs); i++) {
		check_u64_nlattr(fd, nlh_u64, hdrlen,
				 init_inet_diag_msg, print_inet_diag_msg,
				 u64_attrs[i].val, u64_attrs[i].str,
				 pattern, 0);
	}

	/* INET_DIAG_PROTOCOL */
	static const struct strval8 protos[] = {
		{ 0, "IPPROTO_IP" },
		{ 2, "IPPROTO_IGMP" },
		{ 5, "0x5 /* IPPROTO_??? */" },
		{ 6, "IPPROTO_TCP" },
		{ 190, "0xbe /* IPPROTO_??? */" },
		{ 255, "IPPROTO_RAW" },
	};

	for (size_t i = 0; i < ARRAY_SIZE(protos); i++) {
		TEST_NLATTR(fd, nlh0, hdrlen,
			    init_inet_diag_msg, print_inet_diag_msg,
			    INET_DIAG_PROTOCOL,
			    sizeof(uint8_t), &protos[i].val, sizeof(uint8_t),
			    printf("%s", protos[i].str));
	}

	/* INET_DIAG_MD5SIG */
	struct tcp_diag_md5sig md5s_arr[ARRAY_SIZE(md5sig_vecs)];

	for (size_t i = 0; i < ARRAY_SIZE(md5sig_vecs); i++) {
		memcpy(md5s_arr + i, &md5sig_vecs[i].val, sizeof(md5s_arr[0]));

		TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
				   init_inet_diag_msg, print_inet_diag_msg,
				   INET_DIAG_MD5SIG, pattern,
				   md5sig_vecs[i].val,
				   printf("[%s]", md5sig_vecs[i].str));
	}

	TEST_NLATTR_ARRAY(fd, nlh0, hdrlen,
			  init_inet_diag_msg, print_inet_diag_msg,
			  INET_DIAG_MD5SIG, pattern, md5s_arr, print_md5sig);

	/* INET_DIAG_ULP_INFO */
	attr1 = INET_DIAG_ULP_INFO;
	attr1_str = "INET_DIAG_ULP_INFO";

	/* INET_DIAG_ULP_INFO: unknown, undecoded */
	static const struct strval16 ulp_unk_attrs[] = {
		{ ENUM_KNOWN(0, INET_ULP_INFO_UNSPEC) },
		{ ARG_XLAT_UNKNOWN(0x4, "INET_ULP_INFO_???") },
		{ ARG_XLAT_UNKNOWN(0x1ace, "INET_ULP_INFO_???") },
	};
	static const uint32_t dummy = BE_LE(0xdeadc0de, 0xdec0adde);

	for (size_t i = 0; i < ARRAY_SIZE(ulp_unk_attrs); i++) {
		TEST_NESTED_NLATTR_(fd, nlh0, hdrlen,
				    init_inet_diag_nest_1,
				    print_inet_diag_nest_1,
				    ulp_unk_attrs[i].val,
				    ulp_unk_attrs[i].str,
				    sizeof(dummy), &dummy, sizeof(dummy), 1,
				    printf("\"\\xde\\xad\\xc0\\xde\""));
	}

	/* INET_DIAG_ULP_INFO: INET_ULP_INFO_NAME */
	static const struct {
		const char *val;
		const char *str;
		ssize_t sz;
	} ulp_names[] = {
		{ "OH HAI", "\"OH HAI\"", 7 },
		{ "\0\0\0", "\"\\0\\0\\0\"", 4 },
		{ "\1\2\3\4\5\6\7\10\11\12\13\14",
		  "\"\\1\\2\\3\\4\\5\\6\\7\\10\\t\\n\\v\\f\"...", 12 },
	};

	for (size_t i = 0; i < ARRAY_SIZE(ulp_names); i++) {
		TEST_NESTED_NLATTR_(fd, nlh0, hdrlen,
				    init_inet_diag_nest_1,
				    print_inet_diag_nest_1,
				    INET_ULP_INFO_NAME, "INET_ULP_INFO_NAME",
				    ulp_names[i].sz, ulp_names[i].val,
				    ulp_names[i].sz, 1,
				    printf("%s", ulp_names[i].str));
	}

	/* INET_DIAG_ULP_INFO: INET_ULP_INFO_TLS */
	attr2 = INET_ULP_INFO_TLS;
	attr2_str = "INET_ULP_INFO_TLS";

	/* INET_DIAG_ULP_INFO: INET_ULP_INFO_TLS: unknown, undecoded */
	static const struct strval16 tls_unk_attrs[] = {
		{ ENUM_KNOWN(0, TLS_INFO_UNSPEC) },
		{ ENUM_KNOWN(0x6, TLS_INFO_RX_NO_PAD) },
		{ ARG_XLAT_UNKNOWN(0x7, "TLS_INFO_???") },
		{ ARG_XLAT_UNKNOWN(0x1ace, "TLS_INFO_???") },
	};

	for (size_t i = 0; i < ARRAY_SIZE(tls_unk_attrs); i++) {
		TEST_NESTED_NLATTR_(fd, nlh0, hdrlen,
				    init_inet_diag_nest_2,
				    print_inet_diag_nest_2,
				    tls_unk_attrs[i].val,
				    tls_unk_attrs[i].str,
				    sizeof(dummy), &dummy, sizeof(dummy), 2,
				    printf("\"\\xde\\xad\\xc0\\xde\""));
	}

	/* INET_DIAG_ULP_INFO: INET_ULP_INFO_TLS: TLS_INFO_VERSION */
	static const struct strval16 tls_vers[] = {
		{ ARG_XLAT_UNKNOWN(0, "TLS_???_VERSION") },
		{ ARG_XLAT_UNKNOWN(0x200, "TLS_???_VERSION") },
		{ ARG_XLAT_UNKNOWN(0x300, "TLS_???_VERSION") },
		{ ARG_XLAT_UNKNOWN(0x301, "TLS_???_VERSION") },
		{ ARG_XLAT_UNKNOWN(0x302, "TLS_???_VERSION") },
		{ ENUM_KNOWN(0x303, TLS_1_2_VERSION) },
		{ ENUM_KNOWN(0x304, TLS_1_3_VERSION) },
		{ ARG_XLAT_UNKNOWN(0x305, "TLS_???_VERSION") },
		{ ARG_XLAT_UNKNOWN(0xdead, "TLS_???_VERSION") },
	};

	for (size_t i = 0; i < ARRAY_SIZE(tls_vers); i++) {
		TEST_NESTED_NLATTR_(fd, nlh0, hdrlen,
				    init_inet_diag_nest_2,
				    print_inet_diag_nest_2,
				    TLS_INFO_VERSION, "TLS_INFO_VERSION",
				    2, &tls_vers[i].val, 2, 2,
				    printf("%s", tls_vers[i].str));
	}

	/* INET_DIAG_ULP_INFO: INET_ULP_INFO_TLS: TLS_INFO_CIPHER */
	static const struct strval16 tls_ciphers[] = {
		{ ARG_XLAT_UNKNOWN(0, "TLS_CIPHER_???") },
		{ ARG_XLAT_UNKNOWN(0x32, "TLS_CIPHER_???") },
		{ ENUM_KNOWN(0x33, TLS_CIPHER_AES_GCM_128) },
		{ ENUM_KNOWN(0x34, TLS_CIPHER_AES_GCM_256) },
		{ ENUM_KNOWN(0x35, TLS_CIPHER_AES_CCM_128) },
		{ ENUM_KNOWN(0x36, TLS_CIPHER_CHACHA20_POLY1305) },
		{ ENUM_KNOWN(0x37, TLS_CIPHER_SM4_GCM) },
		{ ENUM_KNOWN(0x38, TLS_CIPHER_SM4_CCM) },
		{ ENUM_KNOWN(0x39, TLS_CIPHER_ARIA_GCM_128) },
		{ ENUM_KNOWN(0x3a, TLS_CIPHER_ARIA_GCM_256) },
		{ ARG_XLAT_UNKNOWN(0x3b, "TLS_CIPHER_???") },
		{ ARG_XLAT_UNKNOWN(0xcafe, "TLS_CIPHER_???") },
	};

	for (size_t i = 0; i < ARRAY_SIZE(tls_ciphers); i++) {
		TEST_NESTED_NLATTR_(fd, nlh0, hdrlen,
				    init_inet_diag_nest_2,
				    print_inet_diag_nest_2,
				    TLS_INFO_CIPHER, "TLS_INFO_CIPHER",
				    2, &tls_ciphers[i].val, 2, 2,
				    printf("%s", tls_ciphers[i].str));
	}

	/* INET_DIAG_ULP_INFO: INET_ULP_INFO_TLS: TLS_INFO_[RT]XCONF */
	static const struct strval16 tls_cfg_attrs[] = {
		{ ENUM_KNOWN(0, TLS_INFO_TXCONF) },
		{ ENUM_KNOWN(0, TLS_INFO_RXCONF) },
	};
	static const struct strval16 tls_cfgs[] = {
		{ ARG_XLAT_UNKNOWN(0, "TLS_CONF_???") },
		{ ENUM_KNOWN(0x1, TLS_CONF_BASE) },
		{ ENUM_KNOWN(0x2, TLS_CONF_SW) },
		{ ENUM_KNOWN(0x3, TLS_CONF_HW) },
		{ ENUM_KNOWN(0x4, TLS_CONF_HW_RECORD) },
		{ ARG_XLAT_UNKNOWN(0x5, "TLS_CONF_???") },
		{ ARG_XLAT_UNKNOWN(0xface, "TLS_CONF_???") },
	};

	for (size_t i = 0; i < ARRAY_SIZE(tls_cfg_attrs); i++) {
		for (size_t j = 0; j < ARRAY_SIZE(tls_cfgs); j++) {
			TEST_NESTED_NLATTR_(fd, nlh0, hdrlen,
					    init_inet_diag_nest_2,
					    print_inet_diag_nest_2,
					    tls_cfg_attrs[i].val,
					    tls_cfg_attrs[i].str,
					    2, &tls_cfgs[i].val, 2, 2,
					    printf("%s", tls_cfgs[i].str));
		}
	}

	/* INET_DIAG_ULP_INFO: INET_ULP_INFO_MPTCP */
	attr2 = INET_ULP_INFO_MPTCP;
	attr2_str = "INET_ULP_INFO_MPTCP";

	/* INET_DIAG_ULP_INFO: INET_ULP_INFO_MPTCP: unknown, undecoded */
	static const struct strval16 mptcp_unk_attrs[] = {
		{ ENUM_KNOWN(0, MPTCP_SUBFLOW_ATTR_UNSPEC) },
		{ ENUM_KNOWN(0xb, MPTCP_SUBFLOW_ATTR_PAD) },
		{ ARG_XLAT_UNKNOWN(0xc, "MPTCP_SUBFLOW_ATTR_???") },
		{ ARG_XLAT_UNKNOWN(0x1ace, "MPTCP_SUBFLOW_ATTR_???") },
	};

	for (size_t i = 0; i < ARRAY_SIZE(mptcp_unk_attrs); i++) {
		TEST_NESTED_NLATTR_(fd, nlh0, hdrlen,
				    init_inet_diag_nest_2,
				    print_inet_diag_nest_2,
				    mptcp_unk_attrs[i].val,
				    mptcp_unk_attrs[i].str,
				    sizeof(dummy), &dummy, sizeof(dummy), 2,
				    printf("\"\\xde\\xad\\xc0\\xde\""));
	}

	/* INET_DIAG_ULP_INFO: INET_ULP_INFO_MPTCP: u8 */
	static const struct strval16 mptcp_u8_attrs[] = {
		{ ENUM_KNOWN(0x9, MPTCP_SUBFLOW_ATTR_ID_REM) },
		{ ENUM_KNOWN(0xa, MPTCP_SUBFLOW_ATTR_ID_LOC) },
	};
	void *nlh_n2_u8 = midtail_alloc(NLMSG_SPACE(hdrlen),
					NLA_HDRLEN * 2 + sizeof(uint8_t));

	for (size_t i = 0; i < ARRAY_SIZE(mptcp_u8_attrs); i++) {
		check_u8_nlattr(fd, nlh_n2_u8, hdrlen,
				init_inet_diag_nest_2,
				print_inet_diag_nest_2,
				mptcp_u8_attrs[i].val, mptcp_u8_attrs[i].str,
				pattern, 2);
	}

	/* INET_DIAG_ULP_INFO: INET_ULP_INFO_MPTCP: u16 */
	static const struct strval16 mptcp_u16_attrs[] = {
		{ ENUM_KNOWN(0x7, MPTCP_SUBFLOW_ATTR_MAP_DATALEN) },
	};
	void *nlh_n2_u16 = midtail_alloc(NLMSG_SPACE(hdrlen),
					 NLA_HDRLEN * 2 + sizeof(uint16_t));

	for (size_t i = 0; i < ARRAY_SIZE(mptcp_u16_attrs); i++) {
		check_u16_nlattr(fd, nlh_n2_u16, hdrlen,
				init_inet_diag_nest_2,
				print_inet_diag_nest_2,
				mptcp_u16_attrs[i].val, mptcp_u16_attrs[i].str,
				pattern, 2);
	}

	/* INET_DIAG_ULP_INFO: INET_ULP_INFO_MPTCP: u32 */
	static const struct strval16 mptcp_u32_attrs[] = {
		{ ENUM_KNOWN(0x3, MPTCP_SUBFLOW_ATTR_RELWRITE_SEQ) },
		{ ENUM_KNOWN(0x5, MPTCP_SUBFLOW_ATTR_MAP_SFSEQ) },
		{ ENUM_KNOWN(0x6, MPTCP_SUBFLOW_ATTR_SSN_OFFSET) },
	};
	void *nlh_n2_u32 = midtail_alloc(NLMSG_SPACE(hdrlen),
					 NLA_HDRLEN * 2 + sizeof(uint32_t));

	for (size_t i = 0; i < ARRAY_SIZE(mptcp_u32_attrs); i++) {
		check_u32_nlattr(fd, nlh_n2_u32, hdrlen,
				init_inet_diag_nest_2,
				print_inet_diag_nest_2,
				mptcp_u32_attrs[i].val, mptcp_u32_attrs[i].str,
				pattern, 2);
	}

	/* INET_DIAG_ULP_INFO: INET_ULP_INFO_MPTCP: u64 */
	static const struct strval16 mptcp_u64_attrs[] = {
		{ ENUM_KNOWN(0x4, MPTCP_SUBFLOW_ATTR_MAP_SEQ) },
	};
	void *nlh_n2_u64 = midtail_alloc(NLMSG_SPACE(hdrlen),
					 NLA_HDRLEN * 2 + sizeof(uint64_t));

	for (size_t i = 0; i < ARRAY_SIZE(mptcp_u64_attrs); i++) {
		check_u64_nlattr(fd, nlh_n2_u64, hdrlen,
				init_inet_diag_nest_2,
				print_inet_diag_nest_2,
				mptcp_u64_attrs[i].val, mptcp_u64_attrs[i].str,
				pattern, 2);
	}

	/* INET_DIAG_ULP_INFO: INET_ULP_INFO_MPTCP: x32 */
	static const struct strval16 mptcp_x32_attrs[] = {
		{ ENUM_KNOWN(0x1, MPTCP_SUBFLOW_ATTR_TOKEN_REM) },
		{ ENUM_KNOWN(0x2, MPTCP_SUBFLOW_ATTR_TOKEN_LOC) },
	};

	for (size_t i = 0; i < ARRAY_SIZE(mptcp_x32_attrs); i++) {
		check_x32_nlattr(fd, nlh_u32, hdrlen,
				init_inet_diag_nest_2,
				print_inet_diag_nest_2,
				mptcp_x32_attrs[i].val, mptcp_x32_attrs[i].str,
				pattern, 2);
	}

	/* INET_DIAG_ULP_INFO: INET_ULP_INFO_MPTCP: MPTCP_SUBFLOW_ATTR_FLAGS */
	static const struct strval32 mptcp_flags[] = {
		{ ARG_STR(0) },
		{ ARG_XLAT_KNOWN(0x1, "MPTCP_SUBFLOW_FLAG_MCAP_REM") },
		{ ARG_XLAT_KNOWN(0xdecaffed,
				 "MPTCP_SUBFLOW_FLAG_MCAP_REM"
				 "|MPTCP_SUBFLOW_FLAG_JOIN_REM"
				 "|MPTCP_SUBFLOW_FLAG_JOIN_LOC"
				 "|MPTCP_SUBFLOW_FLAG_BKUP_LOC"
				 "|MPTCP_SUBFLOW_FLAG_FULLY_ESTABLISHED"
				 "|MPTCP_SUBFLOW_FLAG_CONNECTED"
				 "|MPTCP_SUBFLOW_FLAG_MAPVALID|0xdecafe00") },
		{ ARG_XLAT_UNKNOWN(0xfffffe00, "MPTCP_SUBFLOW_FLAG_???") },
	};

	for (size_t i = 0; i < ARRAY_SIZE(mptcp_flags); i++) {
		TEST_NESTED_NLATTR_(fd, nlh0, hdrlen,
				    init_inet_diag_nest_2,
				    print_inet_diag_nest_2,
				    MPTCP_SUBFLOW_ATTR_FLAGS,
				    "MPTCP_SUBFLOW_ATTR_FLAGS",
				    4, &mptcp_flags[i].val, 4, 2,
				    printf("%s", mptcp_flags[i].str));
	}

	/* INET_DIAG_SK_BPF_STORAGES */
	attr1 = INET_DIAG_SK_BPF_STORAGES;
	attr1_str = "INET_DIAG_SK_BPF_STORAGES";

	/* INET_DIAG_SK_BPF_STORAGES: unknown, undecoded */
	static const struct strval16 bpfsts_unk_attrs[] = {
		{ ENUM_KNOWN(0, SK_DIAG_BPF_STORAGE_REP_NONE) },
		{ ARG_XLAT_UNKNOWN(0x2, "SK_DIAG_BPF_STORAGE_???") },
		{ ARG_XLAT_UNKNOWN(0x1ace, "SK_DIAG_BPF_STORAGE_???") },
	};

	for (size_t i = 0; i < ARRAY_SIZE(bpfsts_unk_attrs); i++) {
		TEST_NESTED_NLATTR_(fd, nlh0, hdrlen,
				    init_inet_diag_nest_1,
				    print_inet_diag_nest_1,
				    bpfsts_unk_attrs[i].val,
				    bpfsts_unk_attrs[i].str,
				    sizeof(dummy), &dummy, sizeof(dummy), 1,
				    printf("\"\\xde\\xad\\xc0\\xde\""));
	}

	/* INET_DIAG_SK_BPF_STORAGES: SK_DIAG_BPF_STORAGE */
	attr2 = SK_DIAG_BPF_STORAGE;
	attr2_str = "SK_DIAG_BPF_STORAGE";

	/* INET_DIAG_SK_BPF_STORAGES: SK_DIAG_BPF_STORAGE: unknown, undecoded */
	static const struct strval16 bpfst_unk_attrs[] = {
		{ ENUM_KNOWN(0, SK_DIAG_BPF_STORAGE_NONE) },
		{ ENUM_KNOWN(0x1, SK_DIAG_BPF_STORAGE_NONE) },
		{ ARG_XLAT_UNKNOWN(0x4, "SK_DIAG_BPF_STORAGE_???") },
		{ ARG_XLAT_UNKNOWN(0x1ace, "SK_DIAG_BPF_STORAGE_???") },
	};

	for (size_t i = 0; i < ARRAY_SIZE(bpfst_unk_attrs); i++) {
		TEST_NESTED_NLATTR_(fd, nlh0, hdrlen,
				    init_inet_diag_nest_2,
				    print_inet_diag_nest_2,
				    bpfst_unk_attrs[i].val,
				    bpfst_unk_attrs[i].str,
				    sizeof(dummy), &dummy, sizeof(dummy), 2,
				    printf("\"\\xde\\xad\\xc0\\xde\""));
	}

	/* INET_DIAG_SK_BPF_STORAGES: SK_DIAG_BPF_STORAGE: u32 */
	static const struct strval16 bpfst_u32_attrs[] = {
		{ ENUM_KNOWN(0x2, SK_DIAG_BPF_STORAGE_MAP_ID) },
	};

	for (size_t i = 0; i < ARRAY_SIZE(bpfst_u32_attrs); i++) {
		check_u32_nlattr(fd, nlh_u32, hdrlen,
				init_inet_diag_nest_2,
				print_inet_diag_nest_2,
				bpfst_u32_attrs[i].val, bpfst_u32_attrs[i].str,
				pattern, 2);
	}

	/*
	 * INET_DIAG_SK_BPF_STORAGES: SK_DIAG_BPF_STORAGE:
	 *                            SK_DIAG_BPF_STORAGE_MAP_VALUE
	 */
	static const struct {
		ssize_t sz;
		const char *val;
		const char *str;
	} bpfst_vals[] = {
		{ 1, "\xbe", "0xbe" },
		{ 2, BE_LE("\xde\xad", "\xad\xde"), "0xdead" },
		{ 3, "\xca\xff\xee", "\"\\xca\\xff\\xee\"" },
		{ 4, BE_LE("\xba\xdc\x0d\xed", "\xed\x0d\xdc\xba"),
		     "0xbadc0ded" },
		{ 5, "\x00\x09\x0a\x0b\x0c",
		     "\"\\x00\\x09\\x0a\\x0b\\x0c\"" },
		{ 6, "012345",
		     "\"\\x30\\x31\\x32\\x33\\x34\\x35\"" },
		{ 7, "abcdefg",
		     "\"\\x61\\x62\\x63\\x64\\x65\\x66\\x67\"" },
		{ 8, BE_LE("\xbe\xef\xfa\xce\xde\xad\xc0\xde",
			   "\xde\xc0\xad\xde\xce\xfa\xef\xbe"),
		     "0xbeeffacedeadc0de" },
		{ 9, "ABCDEFGHI",
		     "\"\\x41\\x42\\x43\\x44\\x45\\x46\\x47\\x48\\x49\"" },
		{ 10, "1234567890",
		      "\"\\x31\\x32\\x33\\x34\\x35"
		      "\\x36\\x37\\x38\\x39\\x30\"" },
		{ 12, "a1b2c3d4e5f6",
		      "\"\\x61\\x31\\x62\\x32\\x63\\x33"
		      "\\x64\\x34\\x65\\x35\\x66\\x36\"" },
		{ 16, "A1B2C3D4E5F6G7H8",
		      "\"\\x41\\x31\\x42\\x32\\x43\\x33\\x44\\x34"
		      "\\x45\\x35\\x46\\x36\\x47\\x37\\x48\\x38\"" },
		{ 36, "abcdefghijklmnopqrstuvwxyz0123456789",
		      "\"\\x61\\x62\\x63\\x64\\x65\\x66\\x67\\x68\\x69\\x6a"
		      "\\x6b\\x6c\\x6d\\x6e\\x6f\\x70\\x71\\x72\\x73\\x74\\x75"
		      "\\x76\\x77\\x78\\x79\\x7a\\x30\\x31\\x32\\x33\\x34\\x35"
		      "\"..." },
	};

	for (size_t i = 0; i < ARRAY_SIZE(bpfst_vals); i++) {
		TEST_NESTED_NLATTR_(fd, nlh0, hdrlen,
				    init_inet_diag_nest_2,
				    print_inet_diag_nest_2,
				    SK_DIAG_BPF_STORAGE_MAP_VALUE,
				    "SK_DIAG_BPF_STORAGE_MAP_VALUE",
				    bpfst_vals[i].sz, bpfst_vals[i].val,
				    bpfst_vals[i].sz, 2,
				    printf("%s", bpfst_vals[i].str));
	}

	/* INET_DIAG_SOCKOPT */
	static const struct {
		ssize_t sz;
		const char *val;
		const char *str;
	} sockopts[] = {
		{ 1, "\xbe", "\"\\xbe\"" },
		{ 2, "\x00\x00", "{}" },
		{ 2, BE_LE("\xca\xa0", "\x53\x05"),
		  "{recverr=1, is_icsk=1, mc_loop=1, mc_all=1"
		  ", bind_address_no_port=1, defer_connect=1}" },
		{ 3, BE_LE("\x1e\xad", "\x78\xb5"),
		  "{hdrincl=1, mc_loop=1, transparent=1, mc_all=1"
		  ", bind_address_no_port=1, defer_connect=1"
		  ", unused=" BE_LE("0xd", "0x16") " /* bits 3..8 */}" },
		{ 4, "\xff\xff\x00\xff",
		  "{recverr=1, is_icsk=1, freebind=1, hdrincl=1, mc_loop=1"
		  ", transparent=1, mc_all=1, nodefrag=1"
		  ", bind_address_no_port=1, recverr_rfc4884=1, defer_connect=1"
		  ", unused=0x1f /* bits 3..8 */}"
		  ", /* bytes 2..3 */ \"\\x00\\xff\"" },
	};

	for (size_t i = 0; i < ARRAY_SIZE(sockopts); i++) {
		TEST_NLATTR(fd, nlh0, hdrlen,
			    init_inet_diag_msg, print_inet_diag_msg,
			    INET_DIAG_SOCKOPT,
			    sockopts[i].sz, sockopts[i].val, sockopts[i].sz,
			    printf("%s", sockopts[i].str));
	}

	/* INET_DIAG_LOCALS, INET_DIAG_PEERS */
	static const struct strval16 sa_attrs[] = {
		{ ENUM_KNOWN(0xc, INET_DIAG_LOCALS) },
		{ ENUM_KNOWN(0xd, INET_DIAG_PEERS) },
	};
	enum {
		SA_CNT = 3,
		SA_SZ  = sizeof(struct sockaddr_storage) * SA_CNT,
	};
	void *nlh_sa = midtail_alloc(NLMSG_SPACE(hdrlen), SA_SZ);
	struct sockaddr_storage buf[SA_CNT] = { { 0 } };

	struct sockaddr_in *sa_in = (struct sockaddr_in *) (buf);
	sa_in->sin_family = AF_INET;
	sa_in->sin_port = htons(42069);
	sa_in->sin_addr.s_addr = htonl(0x12345678);

	struct sockaddr_in6 *sa_in6 = (struct sockaddr_in6 *) (buf + 1);
	sa_in6->sin6_family = AF_INET6;
	sa_in6->sin6_port = htons(23456);
	sa_in6->sin6_flowinfo = htonl(0x13579bdf);
	sa_in6->sin6_scope_id = 0x2468ace0;
	memcpy(sa_in6->sin6_addr.s6_addr,
	       "\x12\x34\x56\x78\0\0\0\0\0\0\0\0\x9a\xbc\xde\xf0",
	       sizeof(sa_in6->sin6_addr.s6_addr));

	struct sockaddr_at *sa_at = (struct sockaddr_at *) (buf + 2);
	sa_at->sat_family = AF_APPLETALK;
	sa_at->sat_port = 0x69;
	sa_at->sat_addr.s_net = 0x8765;
	sa_at->sat_addr.s_node = 0x42;

	for (size_t i = 0; i < ARRAY_SIZE(sa_attrs); i++) {
		TEST_NLATTR_ARRAY_(fd, nlh_sa, hdrlen,
				   init_inet_diag_msg, print_inet_diag_msg,
				   sa_attrs[i].val, sa_attrs[i].str,
				   pattern, buf, print_sa);
	}

	puts("+++ exited with 0 +++");
	return 0;
}
