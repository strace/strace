/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/tcp.h>
#include "test_nlattr.h"
#include <linux/inet_diag.h>
#include <linux/sock_diag.h>

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
	printf("{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {idiag_family=AF_INET"
	       ", idiag_state=TCP_LISTEN, idiag_timer=0, idiag_retrans=0"
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
print_uint(const unsigned int *p, size_t i)
{
	if (i >= ARRAY_SIZE(sk_meminfo_strs))
		printf("[%zu /* SK_MEMINFO_??? */", i);
	else
		printf("[%s", sk_meminfo_strs[i]);

	printf("] = %u", *p);
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
	static const uint8_t shutdown = 0xcd;

	const int fd = create_nl_socket(NETLINK_SOCK_DIAG);
	const unsigned int hdrlen = sizeof(struct inet_diag_msg);
	void *const nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen),
					 NLA_HDRLEN +
					 MAX(sizeof(bigmem), DEFAULT_STRLEN));

	static char pattern[4096];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);

	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_inet_diag_msg, print_inet_diag_msg,
			   INET_DIAG_MEMINFO, pattern, minfo,
			   PRINT_FIELD_U("{", minfo, idiag_rmem);
			   PRINT_FIELD_U(", ", minfo, idiag_wmem);
			   PRINT_FIELD_U(", ", minfo, idiag_fmem);
			   PRINT_FIELD_U(", ", minfo, idiag_tmem);
			   printf("}"));

	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_inet_diag_msg, print_inet_diag_msg,
			   INET_DIAG_VEGASINFO, pattern, vegas,
			   PRINT_FIELD_U("{", vegas, tcpv_enabled);
			   PRINT_FIELD_U(", ", vegas, tcpv_rttcnt);
			   PRINT_FIELD_U(", ", vegas, tcpv_rtt);
			   PRINT_FIELD_U(", ", vegas, tcpv_minrtt);
			   printf("}"));


	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_inet_diag_msg, print_inet_diag_msg,
			   INET_DIAG_DCTCPINFO, pattern, dctcp,
			   PRINT_FIELD_U("{", dctcp, dctcp_enabled);
			   PRINT_FIELD_U(", ", dctcp, dctcp_ce_state);
			   PRINT_FIELD_U(", ", dctcp, dctcp_alpha);
			   PRINT_FIELD_U(", ", dctcp, dctcp_ab_ecn);
			   PRINT_FIELD_U(", ", dctcp, dctcp_ab_tot);
			   printf("}"));

	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_inet_diag_msg, print_inet_diag_msg,
			   INET_DIAG_BBRINFO, pattern, bbr,
			   PRINT_FIELD_X("{", bbr, bbr_bw_lo);
			   PRINT_FIELD_X(", ", bbr, bbr_bw_hi);
			   PRINT_FIELD_U(", ", bbr, bbr_min_rtt);
			   PRINT_FIELD_U(", ", bbr, bbr_pacing_gain);
			   PRINT_FIELD_U(", ", bbr, bbr_cwnd_gain);
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

	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_inet_diag_msg, print_inet_diag_msg, INET_DIAG_SHUTDOWN,
		    sizeof(shutdown), &shutdown, sizeof(shutdown),
		    printf("%u", shutdown));

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

	puts("+++ exited with 0 +++");
	return 0;
}
