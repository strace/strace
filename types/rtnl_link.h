/*
 * Copyright (c) 2019-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_TYPES_IF_LINK_H
# define STRACE_TYPES_IF_LINK_H

/*
 * <linux/rtnetlink.h> used to require other headers be included beforehand,
 * include "netlink.h" that pulls in all necessary headers.
 */
# include "netlink.h"

/*
 * These types are defined in <linux/if_link.h> nowadays, but that was not
 * always the case in the past.  Fortunately, when these types were moved
 * out of <linux/rtnetlink.h>, it was changed to include necessary headers
 * for backwards compatibility.
 */
# include <linux/rtnetlink.h>

typedef struct {
	uint32_t rx_packets;
	uint32_t tx_packets;
	uint32_t rx_bytes;
	uint32_t tx_bytes;
	uint32_t rx_errors;
	uint32_t tx_errors;
	uint32_t rx_dropped;
	uint32_t tx_dropped;
	uint32_t multicast;
	uint32_t collisions;
	uint32_t rx_length_errors;
	uint32_t rx_over_errors;
	uint32_t rx_crc_errors;
	uint32_t rx_frame_errors;
	uint32_t rx_fifo_errors;
	uint32_t rx_missed_errors;
	uint32_t tx_aborted_errors;
	uint32_t tx_carrier_errors;
	uint32_t tx_fifo_errors;
	uint32_t tx_heartbeat_errors;
	uint32_t tx_window_errors;
	uint32_t rx_compressed;
	uint32_t tx_compressed;
	uint32_t rx_nohandler; /**< Added by v4.6-rc1~91^2~329^2~2 */
} struct_rtnl_link_stats;

/** Added by Linux commit v2.6.35-rc1~473^2~759 */
typedef struct {
	uint64_t rx_packets;
	uint64_t tx_packets;
	uint64_t rx_bytes;
	uint64_t tx_bytes;
	uint64_t rx_errors;
	uint64_t tx_errors;
	uint64_t rx_dropped;
	uint64_t tx_dropped;
	uint64_t multicast;
	uint64_t collisions;
	uint64_t rx_length_errors;
	uint64_t rx_over_errors;
	uint64_t rx_crc_errors;
	uint64_t rx_frame_errors;
	uint64_t rx_fifo_errors;
	uint64_t rx_missed_errors;
	uint64_t tx_aborted_errors;
	uint64_t tx_carrier_errors;
	uint64_t tx_fifo_errors;
	uint64_t tx_heartbeat_errors;
	uint64_t tx_window_errors;
	uint64_t rx_compressed;
	uint64_t tx_compressed;
	uint64_t rx_nohandler; /**< Added by v4.6-rc1~91^2~329^2~2 */
} struct_rtnl_link_stats64;

/** Added by Linux commit v4.4-rc1~141^2~231^2~18 */
typedef struct {
        uint8_t prio[2];
        uint8_t addr[6];
} struct_ifla_bridge_id;

/** Added by Linux commit v2.6.35-rc1~473^2~33 */
typedef struct {
	uint8_t vsi_mgr_id;
	uint8_t vsi_type_id[3];
	uint8_t vsi_type_version;
	uint8_t pad[3];
} struct_ifla_port_vsi;

#endif /* !STRACE_TYPES_IF_LINK_H */
