/*
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_LINUX_NETLINK_DIAG_H
# define STRACE_LINUX_NETLINK_DIAG_H

struct netlink_diag_req {
	uint8_t sdiag_family;
	uint8_t sdiag_protocol;
	uint16_t pad;
	uint32_t ndiag_ino;
	uint32_t ndiag_show;
	uint32_t ndiag_cookie[2];
};

struct netlink_diag_msg {
	uint8_t ndiag_family;
	uint8_t ndiag_type;
	uint8_t ndiag_protocol;
	uint8_t ndiag_state;

	uint32_t ndiag_portid;
	uint32_t ndiag_dst_portid;
	uint32_t ndiag_dst_group;
	uint32_t ndiag_ino;
	uint32_t ndiag_cookie[2];
};

struct netlink_diag_ring {
	uint32_t ndr_block_size;
	uint32_t ndr_block_nr;
	uint32_t ndr_frame_size;
	uint32_t ndr_frame_nr;
};

enum {
	NETLINK_DIAG_MEMINFO,
	NETLINK_DIAG_GROUPS,
	NETLINK_DIAG_RX_RING,
	NETLINK_DIAG_TX_RING,
	NETLINK_DIAG_FLAGS,
};

# define NDIAG_SHOW_MEMINFO		0x00000001
# define NDIAG_SHOW_GROUPS		0x00000002
# define NDIAG_SHOW_RING_CFG		0x00000004 /* deprecated since 4.6 */
# define NDIAG_SHOW_FLAGS		0x00000008
# define NDIAG_PROTO_ALL			((uint8_t) ~0)

/* flags */
# define NDIAG_FLAG_CB_RUNNING		0x00000001
# define NDIAG_FLAG_PKTINFO		0x00000002
# define NDIAG_FLAG_BROADCAST_ERROR	0x00000004
# define NDIAG_FLAG_NO_ENOBUFS		0x00000008
# define NDIAG_FLAG_LISTEN_ALL_NSID	0x00000010
# define NDIAG_FLAG_CAP_ACK		0x00000020

#endif /* !STRACE_LINUX_NETLINK_DIAG_H */
