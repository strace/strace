/*
 * Copyright (c) 2019-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_TYPES_RTNL_MDB_H
# define STRACE_TYPES_RTNL_MDB_H

/* struct_br_mdb_entry needs a definition of struct in6_addr.  */
# include <netinet/in.h>

# include <linux/if_bridge.h>

/** Added by Linux commit v3.8-rc1~139^2~50 */
typedef struct {
	uint8_t  family; /** Added by Linux commit v3.8-rc1~139^2~3 */
	uint32_t ifindex;
} struct_br_port_msg;

/** Added by Linux commit v3.8-rc1~139^2~50 */
typedef struct {
	uint32_t ifindex;
	uint8_t  state; /** Added by Linux commit v3.8-rc1~40^2~30 */
	uint8_t  flags; /** Added by Linux commit v4.6-rc1~91^2~309^2~2 */
	uint16_t vid; /** Added by Linux commit v4.3-rc1~96^2~365 */
	struct {
		union {
			uint32_t /* __be32 */ ip4;
			struct in6_addr       ip6;
		} u;
		uint16_t /* __be16 */ proto;
	} addr;
} struct_br_mdb_entry;

#endif /* !STRACE_TYPES_RTNL_MDB_H */
