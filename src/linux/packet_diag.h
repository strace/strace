/*
 * Copyright (c) 2017-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_LINUX_PACKET_DIAG_H
# define STRACE_LINUX_PACKET_DIAG_H

struct packet_diag_req {
	uint8_t sdiag_family;
	uint8_t sdiag_protocol;
	uint16_t pad;
	uint32_t pdiag_ino;
	uint32_t pdiag_show;
	uint32_t pdiag_cookie[2];
};

# define PACKET_SHOW_INFO	0x00000001
# define PACKET_SHOW_MCLIST	0x00000002
# define PACKET_SHOW_RING_CFG	0x00000004
# define PACKET_SHOW_FANOUT	0x00000008
# define PACKET_SHOW_MEMINFO	0x00000010
# define PACKET_SHOW_FILTER	0x00000020

struct packet_diag_msg {
	uint8_t pdiag_family;
	uint8_t pdiag_type;
	uint16_t pdiag_num;

	uint32_t pdiag_ino;
	uint32_t pdiag_cookie[2];
};

enum {
	PACKET_DIAG_INFO,
	PACKET_DIAG_MCLIST,
	PACKET_DIAG_RX_RING,
	PACKET_DIAG_TX_RING,
	PACKET_DIAG_FANOUT,
	PACKET_DIAG_UID,
	PACKET_DIAG_MEMINFO,
	PACKET_DIAG_FILTER,
};

struct packet_diag_info {
	uint32_t pdi_index;
	uint32_t pdi_version;
	uint32_t pdi_reserve;
	uint32_t pdi_copy_thresh;
	uint32_t pdi_tstamp;
	uint32_t pdi_flags;

# define PDI_RUNNING	0x1
# define PDI_AUXDATA	0x2
# define PDI_ORIGDEV	0x4
# define PDI_VNETHDR	0x8
# define PDI_LOSS	0x10
};

struct packet_diag_mclist {
	uint32_t pdmc_index;
	uint32_t pdmc_count;
	uint16_t pdmc_type;
	uint16_t pdmc_alen;
	uint8_t pdmc_addr[32]; /* MAX_ADDR_LEN */
};

struct packet_diag_ring {
	uint32_t pdr_block_size;
	uint32_t pdr_block_nr;
	uint32_t pdr_frame_size;
	uint32_t pdr_frame_nr;
	uint32_t pdr_retire_tmo;
	uint32_t pdr_sizeof_priv;
	uint32_t pdr_features;
};

#endif /* !STRACE_LINUX_PACKET_DIAG_H */
