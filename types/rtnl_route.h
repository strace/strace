/*
 * Copyright (c) 2019-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_TYPES_RTNL_ROUTE_H
# define STRACE_TYPES_RTNL_ROUTE_H

/*
 * <linux/rtnetlink.h> used to require other headers be included beforehand,
 * include "netlink.h" that pulls in all necessary headers.
 */
# include "netlink.h"
# include <linux/rtnetlink.h>

/** Added by Linux commit v3.8-rc1~139^2~90 */
typedef struct {
	uint64_t mfcs_packets;
	uint64_t mfcs_bytes;
	uint64_t mfcs_wrong_if;
} struct_rta_mfc_stats;

/** Added by Linux commit v4.1-rc1~128^2~350^2~1 */
typedef struct {
	uint16_t /* __kernel_sa_family_t */ rtvia_family;
	uint8_t rtvia_addr[0];
} struct_rtvia;

#endif /* !STRACE_TYPES_RTNL_ROUTE_H */
