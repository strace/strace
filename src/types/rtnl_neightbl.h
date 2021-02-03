/*
 * Copyright (c) 2019-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_TYPES_RTNL_NEIGHTBL_H
# define STRACE_TYPES_RTNL_NEIGHTBL_H

/*
 * <linux/rtnetlink.h> used to require other headers be included beforehand,
 * include "netlink.h" that pulls in all necessary headers.
 */
# include "netlink.h"

/*
 * These types are defined in <linux/neighbour.h> nowadays, but that was not
 * always the case in the past.  Fortunately, when these types were moved
 * out of <linux/rtnetlink.h>, it was changed to include necessary headers
 * for backwards compatibility.
 */
# include <linux/rtnetlink.h>

typedef struct {
	uint16_t ndtc_key_len;
	uint16_t ndtc_entry_size;
	uint32_t ndtc_entries;
	uint32_t ndtc_last_flush;
	uint32_t ndtc_last_rand;
	uint32_t ndtc_hash_rnd;
	uint32_t ndtc_hash_mask;
	uint32_t ndtc_hash_chain_gc;
	uint32_t ndtc_proxy_qlen;
} struct_ndt_config;

typedef struct {
	uint64_t ndts_allocs;
	uint64_t ndts_destroys;
	uint64_t ndts_hash_grows;
	uint64_t ndts_res_failed;
	uint64_t ndts_lookups;
	uint64_t ndts_hits;
	uint64_t ndts_rcv_probes_mcast;
	uint64_t ndts_rcv_probes_ucast;
	uint64_t ndts_periodic_gc_runs;
	uint64_t ndts_forced_gc_runs;
	uint64_t ndts_table_fulls; /**< Added by v4.3-rc1~96^2~202 */
} struct_ndt_stats;

#endif /* !STRACE_TYPES_RTNL_NEIGHTBL_H */
