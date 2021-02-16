/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <sys/socket.h>

#ifndef AF_SMC
# define AF_SMC 43
#endif

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include "test_nlattr.h"
#include <linux/rtnetlink.h>
#include <linux/smc_diag.h>
#include <linux/sock_diag.h>

#ifndef SMC_CLNT
# define SMC_CLNT 0
#endif
#ifndef SMC_ACTIVE
# define SMC_ACTIVE 1
#endif

static const char address[] = "12.34.56.78";

static void
init_smc_diag_msg(struct nlmsghdr *const nlh, const unsigned int msg_len)
{
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = msg_len,
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_DUMP
	);

	struct smc_diag_msg *const msg = NLMSG_DATA(nlh);
	SET_STRUCT(struct smc_diag_msg, msg,
		.diag_family = AF_SMC,
		.diag_state = SMC_ACTIVE
	);

	if (!inet_pton(AF_INET, address, msg->id.idiag_src) ||
	    !inet_pton(AF_INET, address, msg->id.idiag_dst))
		perror_msg_and_skip("inet_pton");
}

static void
print_smc_diag_msg(const unsigned int msg_len)
{
	printf("{nlmsg_len=%u, nlmsg_type=SOCK_DIAG_BY_FAMILY"
	       ", nlmsg_flags=NLM_F_DUMP, nlmsg_seq=0, nlmsg_pid=0}"
	       ", {diag_family=AF_SMC, diag_state=SMC_ACTIVE"
	       ", diag_fallback=SMC_DIAG_MODE_SMCR, diag_shutdown=0"
	       ", id={idiag_sport=htons(0), idiag_dport=htons(0)"
	       ", idiag_src=inet_addr(\"%s\")"
	       ", idiag_dst=inet_addr(\"%s\")"
	       ", idiag_if=0, idiag_cookie=[0, 0]}"
	       ", diag_uid=0, diag_inode=0}",
	       msg_len, address, address);
}

#define PRINT_FIELD_SMC_DIAG_CURSOR(where_, field_)		\
	do {							\
		printf("%s=", #field_);				\
		printf("{");					\
		PRINT_FIELD_U((where_).field_, reserved);	\
		printf(", ");					\
		PRINT_FIELD_U((where_).field_, wrap);		\
		printf(", ");					\
		PRINT_FIELD_U((where_).field_, count);		\
		printf("}");					\
	} while (0)

int main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	static const struct smc_diag_conninfo cinfo = {
		.token = 0xabcdefac,
		.sndbuf_size = 0xbcdaefad,
		.rmbe_size = 0xcdbaefab,
		.peer_rmbe_size = 0xdbcdedaf,
		.rx_prod = {
			.reserved = 0xabc1,
			.wrap = 0xbca1,
			.count = 0xcdedbad1
		},
		.rx_cons = {
			.reserved = 0xabc2,
			.wrap = 0xbca2,
			.count = 0xcdedbad2
		},
		.tx_prod = {
			.reserved = 0xabc3,
			.wrap = 0xbca3,
			.count = 0xcdedbad3
		},
		.tx_cons = {
			.reserved = 0xabc4,
			.wrap = 0xbca4,
			.count = 0xcdedbad4
		},
		.rx_prod_flags = 0xff,
		.rx_conn_state_flags = 0xff,
		.tx_prod_flags = 0xff,
		.tx_conn_state_flags = 0xff,
		.tx_prep = {
			.reserved = 0xabc5,
			.wrap = 0xbca5,
			.count = 0xcdedbad5
		},
		.tx_sent = {
			.reserved = 0xabc6,
			.wrap = 0xbca6,
			.count = 0xcdedbad6
		},
		.tx_fin = {
			.reserved = 0xabc7,
			.wrap = 0xbca7,
			.count = 0xcdedbad7
		}
	};
	static const struct smc_diag_lgrinfo linfo = {
		.lnk[0] = {
			.link_id = 0xaf,
			.ibport = 0xfa,
			.ibname = "123",
			.gid = "456",
			.peer_gid = "789"
		},
		.role = SMC_CLNT
	};
	static const struct smcd_diag_dmbinfo dinfo = {
		.linkid     = 0xdeadc0de,
		.peer_gid   = 0xbefeededbadc0dedULL,
		.my_gid     = 0xdeec0dedfacebeefULL,
		.token      = 0xcafedecaffeedeedULL,
		.peer_token = 0xfeedfacebeeff00dULL,
	};
	static const struct smc_diag_fallback fb1 = {
		.reason         = 0,
		.peer_diagnosis = 0x03020000,
	};
	static const struct smc_diag_fallback fb2 = {
		.reason         = 0x03060000,
		.peer_diagnosis = 0x99999999,
	};
	static uint8_t sd1 = 0x23;
	static uint8_t sd2 = 0x40;

	int fd = create_nl_socket(NETLINK_SOCK_DIAG);
	const unsigned int hdrlen = sizeof(struct smc_diag_msg);
	void *const nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen),
					 NLA_HDRLEN +
					 MAX(sizeof(cinfo), sizeof(linfo)));

	static char pattern[4096];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);

	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_smc_diag_msg, print_smc_diag_msg,
			   SMC_DIAG_SHUTDOWN, pattern, sd1,
			   printf("RCV_SHUTDOWN|SEND_SHUTDOWN|0x20"));

	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_smc_diag_msg, print_smc_diag_msg,
			   SMC_DIAG_SHUTDOWN, pattern, sd2,
			   printf("0x40 /* ???_SHUTDOWN */"));

	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_smc_diag_msg, print_smc_diag_msg,
			   SMC_DIAG_CONNINFO, pattern, cinfo,
			   printf("{");
			   PRINT_FIELD_U(cinfo, token);
			   printf(", ");
			   PRINT_FIELD_U(cinfo, sndbuf_size);
			   printf(", ");
			   PRINT_FIELD_U(cinfo, rmbe_size);
			   printf(", ");
			   PRINT_FIELD_U(cinfo, peer_rmbe_size);
			   printf(", ");
			   PRINT_FIELD_SMC_DIAG_CURSOR(cinfo, rx_prod);
			   printf(", ");
			   PRINT_FIELD_SMC_DIAG_CURSOR(cinfo, rx_cons);
			   printf(", ");
			   PRINT_FIELD_SMC_DIAG_CURSOR(cinfo, tx_prod);
			   printf(", ");
			   PRINT_FIELD_SMC_DIAG_CURSOR(cinfo, tx_cons);
			   printf(", rx_prod_flags=0xff");
			   printf(", rx_conn_state_flags=0xff");
			   printf(", tx_prod_flags=0xff");
			   printf(", tx_conn_state_flags=0xff");
			   printf(", ");
			   PRINT_FIELD_SMC_DIAG_CURSOR(cinfo, tx_prep);
			   printf(", ");
			   PRINT_FIELD_SMC_DIAG_CURSOR(cinfo, tx_sent);
			   printf(", ");
			   PRINT_FIELD_SMC_DIAG_CURSOR(cinfo, tx_fin);
			   printf("}"));

	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_smc_diag_msg, print_smc_diag_msg,
			   SMC_DIAG_LGRINFO, pattern, linfo,
			   printf("{lnk=[{");
			   PRINT_FIELD_U(linfo.lnk[0], link_id);
			   printf(", ibname=\"%s\"", linfo.lnk[0].ibname);
			   printf(", ");
			   PRINT_FIELD_U(linfo.lnk[0], ibport);
			   printf(", gid=\"%s\"", linfo.lnk[0].gid);
			   printf(", peer_gid=\"%s\"}]", linfo.lnk[0].peer_gid);
			   printf(", role=SMC_CLNT}"));

	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_smc_diag_msg, print_smc_diag_msg,
			   SMC_DIAG_DMBINFO, pattern, dinfo,
			   printf("{");
			   PRINT_FIELD_U(dinfo, linkid);
			   printf(", ");
			   PRINT_FIELD_X(dinfo, peer_gid);
			   printf(", ");
			   PRINT_FIELD_X(dinfo, my_gid);
			   printf(", ");
			   PRINT_FIELD_X(dinfo, token);
			   printf(", ");
			   PRINT_FIELD_X(dinfo, peer_token);
			   printf("}"));

	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_smc_diag_msg, print_smc_diag_msg,
			   SMC_DIAG_FALLBACK, pattern, fb1,
			   printf("{reason=0 /* SMC_CLC_DECL_??? */");
			   printf(", peer_diagnosis=0x3020000"
			          " /* SMC_CLC_DECL_IPSEC */}"));

	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_smc_diag_msg, print_smc_diag_msg,
			   SMC_DIAG_FALLBACK, pattern, fb2,
			   printf("{reason=0x3060000"
			          " /* SMC_CLC_DECL_OPTUNSUPP */");
			   printf(", peer_diagnosis=0x99999999"
			          " /* SMC_CLC_DECL_??? */}"));

	printf("+++ exited with 0 +++\n");
	return 0;
}
