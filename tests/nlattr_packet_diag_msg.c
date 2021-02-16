/*
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
#include <net/if.h>
#include "test_nlattr.h"
#include <sys/socket.h>
#include <linux/filter.h>
#include <linux/packet_diag.h>
#include <linux/rtnetlink.h>
#include <linux/sock_diag.h>

static void
init_packet_diag_msg(struct nlmsghdr *const nlh, const unsigned int msg_len)
{
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = msg_len,
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_DUMP
	);

	struct packet_diag_msg *const msg = NLMSG_DATA(nlh);
	SET_STRUCT(struct packet_diag_msg, msg,
		.pdiag_family = AF_PACKET,
		.pdiag_type = SOCK_STREAM,
		.pdiag_num = 3,
	);
}

static void
print_packet_diag_msg(const unsigned int msg_len)
{
	printf("{nlmsg_len=%u, nlmsg_type=SOCK_DIAG_BY_FAMILY"
	       ", nlmsg_flags=NLM_F_DUMP, nlmsg_seq=0, nlmsg_pid=0}"
	       ", {pdiag_family=AF_PACKET"
	       ", pdiag_type=SOCK_STREAM, pdiag_num=ETH_P_ALL"
	       ", pdiag_ino=0, pdiag_cookie=[0, 0]}",
	       msg_len);
}

static void
print_packet_diag_mclist(const struct packet_diag_mclist *const dml, size_t i)
{
	printf("{pdmc_index=" IFINDEX_LO_STR);
	printf(", ");
	PRINT_FIELD_U(*dml, pdmc_count);
	printf(", ");
	PRINT_FIELD_U(*dml, pdmc_type);
	printf(", ");
	PRINT_FIELD_U(*dml, pdmc_alen);
	printf(", pdmc_addr=");
	print_quoted_hex(dml->pdmc_addr, dml->pdmc_alen);
	printf("}");
}

static const struct sock_filter filter[] = {
	BPF_STMT(BPF_LD|BPF_B|BPF_ABS, SKF_AD_OFF+SKF_AD_PKTTYPE),
	BPF_STMT(BPF_RET|BPF_K, 0x2a)
};

static void
print_sock_filter(const struct sock_filter *const f, size_t i)
{
	if (f == filter)
		printf("BPF_STMT(BPF_LD|BPF_B|BPF_ABS"
		       ", SKF_AD_OFF+SKF_AD_PKTTYPE)");
	else
		printf("BPF_STMT(BPF_RET|BPF_K, 0x2a)");
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	struct packet_diag_info pinfo = {
		.pdi_index = ifindex_lo(),
		.pdi_version = 2,
		.pdi_reserve = 0xcfaacdaf,
		.pdi_copy_thresh = 0xdabacdaf,
		.pdi_tstamp = 0xeafbaadf,
		.pdi_flags = PDI_RUNNING
	};
	const struct packet_diag_mclist dml[] = {
		{
			.pdmc_index = ifindex_lo(),
			.pdmc_count = 0xabcdaefc,
			.pdmc_type = 0xcdaf,
			.pdmc_alen = 4,
			.pdmc_addr = "1234"
		},
		{
			.pdmc_index = ifindex_lo(),
			.pdmc_count = 0xdaefeafc,
			.pdmc_type = 0xadef,
			.pdmc_alen = 4,
			.pdmc_addr = "5678"
		}
	};
	static const struct packet_diag_ring pdr = {
		.pdr_block_size = 0xabcdafed,
		.pdr_block_nr = 0xbcadefae,
		.pdr_frame_size = 0xcabdfeac,
		.pdr_frame_nr = 0xdeaeadef,
		.pdr_retire_tmo = 0xedbafeac,
		.pdr_sizeof_priv = 0xfeadeacd,
		.pdr_features = 0xadebadea
	};

	int fd = create_nl_socket(NETLINK_SOCK_DIAG);
	const unsigned int hdrlen = sizeof(struct packet_diag_msg);
	void *const nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen),
					 NLA_HDRLEN + sizeof(dml));

	static char pattern[4096];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);

	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_packet_diag_msg, print_packet_diag_msg,
			   PACKET_DIAG_INFO, pattern, pinfo,
			   printf("{pdi_index=%s", IFINDEX_LO_STR);
			   printf(", pdi_version=TPACKET_V3");
			   printf(", ");
			   PRINT_FIELD_U(pinfo, pdi_reserve);
			   printf(", ");
			   PRINT_FIELD_U(pinfo, pdi_copy_thresh);
			   printf(", ");
			   PRINT_FIELD_U(pinfo, pdi_tstamp);
			   printf(", pdi_flags=PDI_RUNNING}"));

	TEST_NLATTR_ARRAY(fd, nlh0, hdrlen,
			  init_packet_diag_msg, print_packet_diag_msg,
			  PACKET_DIAG_MCLIST, pattern, dml,
			  print_packet_diag_mclist);

	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_packet_diag_msg, print_packet_diag_msg,
			   PACKET_DIAG_RX_RING, pattern, pdr,
			   printf("{");
			   PRINT_FIELD_U(pdr, pdr_block_size);
			   printf(", ");
			   PRINT_FIELD_U(pdr, pdr_block_nr);
			   printf(", ");
			   PRINT_FIELD_U(pdr, pdr_frame_size);
			   printf(", ");
			   PRINT_FIELD_U(pdr, pdr_frame_nr);
			   printf(", ");
			   PRINT_FIELD_U(pdr, pdr_retire_tmo);
			   printf(", ");
			   PRINT_FIELD_U(pdr, pdr_sizeof_priv);
			   printf(", ");
			   PRINT_FIELD_U(pdr, pdr_features);
			   printf("}"));

	TEST_NLATTR_ARRAY(fd, nlh0, hdrlen,
			  init_packet_diag_msg, print_packet_diag_msg,
			  PACKET_DIAG_FILTER, pattern, filter,
			  print_sock_filter);

	printf("+++ exited with 0 +++\n");
	return 0;
}
