/*
 * Copyright (c) 2016 Fabien Siron <fabien.siron@epita.fr>
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2016-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "netlink_route.h"
#include "nlattr.h"

#include "netlink.h"

#include <netinet/in.h>

#include <linux/rtnetlink.h>
#include <linux/if_bridge.h>
#include <linux/if_link.h>
#include <linux/if_bridge.h>

#include "xlat/in6_addr_gen_mode.h"
#include "xlat/inet_devconf_indices.h"
#include "xlat/inet6_devconf_indices.h"
#include "xlat/inet6_if_flags.h"
#include "xlat/rtnl_ifla_af_spec_bridge_attrs.h"
#include "xlat/rtnl_ifla_af_spec_bridge_vlan_tunnel_info_attrs.h"
#include "xlat/rtnl_ifla_af_spec_inet_attrs.h"
#include "xlat/rtnl_ifla_af_spec_inet6_attrs.h"
#include "xlat/rtnl_ifla_af_spec_mctp_attrs.h"
#include "xlat/rtnl_ifla_bridge_flags.h"
#include "xlat/rtnl_ifla_bridge_modes.h"
#include "xlat/rtnl_ifla_brport_attrs.h"
#include "xlat/rtnl_ifla_br_boolopts.h"
#include "xlat/rtnl_ifla_br_boolopt_flags.h"
#include "xlat/rtnl_ifla_br_mcast_querier_attrs.h"
#include "xlat/rtnl_ifla_events.h"
#include "xlat/rtnl_ifla_ext_filter_flags.h"
#include "xlat/rtnl_ifla_info_attrs.h"
#include "xlat/rtnl_ifla_info_data_bridge_attrs.h"
#include "xlat/rtnl_ifla_info_data_tun_attrs.h"
#include "xlat/rtnl_ifla_port_attrs.h"
#include "xlat/rtnl_ifla_proto_down_reason_attrs.h"
#include "xlat/rtnl_ifla_vf_info_attrs.h"
#include "xlat/rtnl_ifla_vf_link_states.h"
#include "xlat/rtnl_ifla_vf_port_attrs.h"
#include "xlat/rtnl_ifla_vf_stats_attrs.h"
#include "xlat/rtnl_ifla_vf_vlan_list_attrs.h"
#include "xlat/rtnl_ifla_vfinfo_list_attrs.h"
#include "xlat/rtnl_ifla_xdp_attached_mode.h"
#include "xlat/rtnl_ifla_xdp_attrs.h"
#include "xlat/rtnl_link_attrs.h"
#include "xlat/snmp_icmp6_stats.h"
#include "xlat/snmp_ip_stats.h"
#include "xlat/tun_device_types.h"
#include "xlat/xdp_flags.h"

#define XLAT_MACROS_ONLY
# include "xlat/addrfams.h"
#undef XLAT_MACROS_ONLY

static bool
decode_ifla_hwaddr(struct tcb *const tcp,
		   const kernel_ulong_t addr,
		   const unsigned int len,
		   const void *const opaque_data)
{
	const struct ifinfomsg *ifinfo = (const struct ifinfomsg *) opaque_data;

	return decode_nla_hwaddr_family(tcp, addr, len, ifinfo->ifi_family);
}

static bool
decode_rtnl_link_stats(struct tcb *const tcp,
		       const kernel_ulong_t addr,
		       const unsigned int len,
		       const void *const opaque_data)
{
	struct rtnl_link_stats st;
	const unsigned int min_size =
		offsetofend(struct rtnl_link_stats, tx_compressed);
	const unsigned int def_size = sizeof(st);
	const unsigned int size =
		(len >= def_size) ? def_size :
				    ((len == min_size) ? min_size : 0);

	if (!size)
		return false;

	if (!umoven_or_printaddr(tcp, addr, size, &st)) {
		tprint_struct_begin();
		PRINT_FIELD_U(st, rx_packets);
		tprint_struct_next();
		PRINT_FIELD_U(st, tx_packets);
		tprint_struct_next();
		PRINT_FIELD_U(st, rx_bytes);
		tprint_struct_next();
		PRINT_FIELD_U(st, tx_bytes);
		tprint_struct_next();
		PRINT_FIELD_U(st, rx_errors);
		tprint_struct_next();
		PRINT_FIELD_U(st, tx_errors);
		tprint_struct_next();
		PRINT_FIELD_U(st, rx_dropped);
		tprint_struct_next();
		PRINT_FIELD_U(st, tx_dropped);
		tprint_struct_next();
		PRINT_FIELD_U(st, multicast);
		tprint_struct_next();
		PRINT_FIELD_U(st, collisions);

		tprint_struct_next();
		PRINT_FIELD_U(st, rx_length_errors);
		tprint_struct_next();
		PRINT_FIELD_U(st, rx_over_errors);
		tprint_struct_next();
		PRINT_FIELD_U(st, rx_crc_errors);
		tprint_struct_next();
		PRINT_FIELD_U(st, rx_frame_errors);
		tprint_struct_next();
		PRINT_FIELD_U(st, rx_fifo_errors);
		tprint_struct_next();
		PRINT_FIELD_U(st, rx_missed_errors);

		tprint_struct_next();
		PRINT_FIELD_U(st, tx_aborted_errors);
		tprint_struct_next();
		PRINT_FIELD_U(st, tx_carrier_errors);
		tprint_struct_next();
		PRINT_FIELD_U(st, tx_fifo_errors);
		tprint_struct_next();
		PRINT_FIELD_U(st, tx_heartbeat_errors);
		tprint_struct_next();
		PRINT_FIELD_U(st, tx_window_errors);

		tprint_struct_next();
		PRINT_FIELD_U(st, rx_compressed);
		tprint_struct_next();
		PRINT_FIELD_U(st, tx_compressed);

		if (len >= def_size) {
			tprint_struct_next();
			PRINT_FIELD_U(st, rx_nohandler);
		}
		tprint_struct_end();
	}

	return true;
}

static bool
decode_ifla_bridge_id(struct tcb *const tcp,
		      const kernel_ulong_t addr,
		      const unsigned int len,
		      const void *const opaque_data)
{
	struct ifla_bridge_id id;

	if (len < sizeof(id))
		return false;
	else if (!umove_or_printaddr(tcp, addr, &id)) {
		tprint_struct_begin();
		PRINT_FIELD_ARRAY(id, prio, tcp,
				  print_uint_array_member);
		tprint_struct_next();
		PRINT_FIELD_MAC(id, addr);
		tprint_struct_end();
	}

	return true;
}

static const nla_decoder_t ifla_brport_nla_decoders[] = {
	[IFLA_BRPORT_STATE]			= decode_nla_u8,
	[IFLA_BRPORT_PRIORITY]			= decode_nla_u16,
	[IFLA_BRPORT_COST]			= decode_nla_u32,
	[IFLA_BRPORT_MODE]			= decode_nla_u8,
	[IFLA_BRPORT_GUARD]			= decode_nla_u8,
	[IFLA_BRPORT_PROTECT]			= decode_nla_u8,
	[IFLA_BRPORT_FAST_LEAVE]		= decode_nla_u8,
	[IFLA_BRPORT_LEARNING]			= decode_nla_u8,
	[IFLA_BRPORT_UNICAST_FLOOD]		= decode_nla_u8,
	[IFLA_BRPORT_PROXYARP]			= decode_nla_u8,
	[IFLA_BRPORT_LEARNING_SYNC]		= decode_nla_u8,
	[IFLA_BRPORT_PROXYARP_WIFI]		= decode_nla_u8,
	[IFLA_BRPORT_ROOT_ID]			= decode_ifla_bridge_id,
	[IFLA_BRPORT_BRIDGE_ID]			= decode_ifla_bridge_id,
	[IFLA_BRPORT_DESIGNATED_PORT]		= decode_nla_u16,
	[IFLA_BRPORT_DESIGNATED_COST]		= decode_nla_u16,
	[IFLA_BRPORT_ID]			= decode_nla_u16,
	[IFLA_BRPORT_NO]			= decode_nla_u16,
	[IFLA_BRPORT_TOPOLOGY_CHANGE_ACK]	= decode_nla_u8,
	[IFLA_BRPORT_CONFIG_PENDING]		= decode_nla_u8,
	[IFLA_BRPORT_MESSAGE_AGE_TIMER]		= decode_nla_clock_t,
	[IFLA_BRPORT_FORWARD_DELAY_TIMER]	= decode_nla_clock_t,
	[IFLA_BRPORT_HOLD_TIMER]		= decode_nla_clock_t,
	[IFLA_BRPORT_FLUSH]			= NULL,
	[IFLA_BRPORT_MULTICAST_ROUTER]		= decode_nla_u8,
	[IFLA_BRPORT_PAD]			= NULL,
	[IFLA_BRPORT_MCAST_FLOOD]		= decode_nla_u8,
	[IFLA_BRPORT_MCAST_TO_UCAST]		= decode_nla_u8,
	[IFLA_BRPORT_VLAN_TUNNEL]		= decode_nla_u8,
	[IFLA_BRPORT_BCAST_FLOOD]		= decode_nla_u8,
	[IFLA_BRPORT_GROUP_FWD_MASK]		= decode_nla_x16,
	[IFLA_BRPORT_NEIGH_SUPPRESS]		= decode_nla_u8,
	[IFLA_BRPORT_ISOLATED]			= decode_nla_u8,
	[IFLA_BRPORT_BACKUP_PORT]		= decode_nla_ifindex,
	[IFLA_BRPORT_MRP_RING_OPEN]		= decode_nla_u8,
	[IFLA_BRPORT_MRP_IN_OPEN]		= decode_nla_u8,
	[IFLA_BRPORT_MCAST_EHT_HOSTS_LIMIT]	= decode_nla_u32,
	[IFLA_BRPORT_MCAST_EHT_HOSTS_CNT]	= decode_nla_u32,
	[IFLA_BRPORT_LOCKED]			= decode_nla_u8,
	[IFLA_BRPORT_MAB]			= decode_nla_u8,
	[IFLA_BRPORT_MCAST_N_GROUPS]		= decode_nla_u32,
	[IFLA_BRPORT_MCAST_MAX_GROUPS]		= decode_nla_u32,
	[IFLA_BRPORT_NEIGH_VLAN_SUPPRESS]	= decode_nla_u8,
};

static bool
decode_ifla_inet6_flags(struct tcb *const tcp,
		        const kernel_ulong_t addr,
		        const unsigned int len,
		        const void *const opaque_data)
{
	static const struct decode_nla_xlat_opts opts = {
		inet6_if_flags, "IF_???",
		.size = 4,
	};

	return decode_nla_flags(tcp, addr, len, &opts);
}

static bool
decode_ifla_inet6_conf(struct tcb *const tcp,
		       const kernel_ulong_t addr,
		       const unsigned int len,
		       const void *const opaque_data)
{
	int elem;
	size_t cnt = len / sizeof(elem);

	if (!cnt)
		return false;

	print_array_ex(tcp, addr, cnt, &elem, sizeof(elem),
		       tfetch_mem, print_int_array_member, NULL,
		       PAF_PRINT_INDICES | XLAT_STYLE_FMT_D,
		       inet6_devconf_indices, "DEVCONF_???");

	return true;
}

static bool
decode_ifla_inet6_stats(struct tcb *const tcp,
		        const kernel_ulong_t addr,
		        const unsigned int len,
		        const void *const opaque_data)
{
	uint64_t elem;
	size_t cnt = len / sizeof(elem);

	if (!cnt)
		return false;

	print_array_ex(tcp, addr, cnt, &elem, sizeof(elem),
		       tfetch_mem, print_uint_array_member, NULL,
		       PAF_PRINT_INDICES | XLAT_STYLE_FMT_U,
		       snmp_ip_stats, "IPSTATS_MIB_???");

	return true;
}

static bool
decode_ifla_inet6_cacheinfo(struct tcb *const tcp,
			    const kernel_ulong_t addr,
			    const unsigned int len,
			    const void *const opaque_data)
{
	struct ifla_cacheinfo ci;

	if (len < sizeof(ci))
		return false;
	else if (!umove_or_printaddr(tcp, addr, &ci)) {
		tprint_struct_begin();
		PRINT_FIELD_U(ci, max_reasm_len);
		tprint_struct_next();
		PRINT_FIELD_U(ci, tstamp);
		tprint_struct_next();
		PRINT_FIELD_U(ci, reachable_time);
		tprint_struct_next();
		PRINT_FIELD_U(ci, retrans_time);
		tprint_struct_end();
	}

	return true;
}

static bool
decode_ifla_inet6_icmp6_stats(struct tcb *const tcp,
			      const kernel_ulong_t addr,
			      const unsigned int len,
			      const void *const opaque_data)
{
	uint64_t elem;
	size_t cnt = len / sizeof(elem);

	if (!cnt)
		return false;

	print_array_ex(tcp, addr, cnt, &elem, sizeof(elem),
		       tfetch_mem, print_uint_array_member, NULL,
		       PAF_PRINT_INDICES | XLAT_STYLE_FMT_U,
		       snmp_icmp6_stats, "ICMP6_MIB_???");

	return true;
}

static bool
decode_ifla_inet6_agm(struct tcb *const tcp,
		      const kernel_ulong_t addr,
		      const unsigned int len,
		      const void *const opaque_data)
{
	static const struct decode_nla_xlat_opts opts = {
		in6_addr_gen_mode, "IN6_ADDR_GEN_MODE_???",
		.size = 1,
	};

	return decode_nla_xval(tcp, addr, len, &opts);
}

static const nla_decoder_t ifla_inet6_nla_decoders[] = {
	[IFLA_INET6_FLAGS]		= decode_ifla_inet6_flags,
	[IFLA_INET6_CONF]		= decode_ifla_inet6_conf,
	[IFLA_INET6_STATS]		= decode_ifla_inet6_stats,
	[IFLA_INET6_MCAST]		= NULL, /* unused */
	[IFLA_INET6_CACHEINFO]		= decode_ifla_inet6_cacheinfo,
	[IFLA_INET6_ICMP6STATS]		= decode_ifla_inet6_icmp6_stats,
	[IFLA_INET6_TOKEN]		= decode_nla_in6_addr,
	[IFLA_INET6_ADDR_GEN_MODE]	= decode_ifla_inet6_agm,
	[IFLA_INET6_RA_MTU]		= decode_nla_u32,
};

static bool
decode_ifla_protinfo(struct tcb *const tcp,
		     const kernel_ulong_t addr,
		     const unsigned int len,
		     const void *const opaque_data)
{
	static const struct af_spec_decoder_desc protos[] = {
		{ AF_BRIDGE, rtnl_ifla_brport_attrs,  "IFLA_BRPORT_???",
		  ARRSZ_PAIR(ifla_brport_nla_decoders) },
		/*
		 * For AF_INET6, it is the same as for IFLA_AF_SPEC, see
		 * the call sites of net/ipv6/addrconf.c:inet6_fill_ifla6_attrs.
		 */
		{ AF_INET6, rtnl_ifla_af_spec_inet6_attrs,  "IFLA_INET6_???",
		  ARRSZ_PAIR(ifla_inet6_nla_decoders) },
	};

	const struct ifinfomsg *ifinfo = (const struct ifinfomsg *) opaque_data;

	decode_nla_af_spec(tcp, addr, len,
			   ifinfo->ifi_family, ARRSZ_PAIR(protos));

	return true;
}

static bool
decode_rtnl_link_ifmap(struct tcb *const tcp,
		       const kernel_ulong_t addr,
		       const unsigned int len,
		       const void *const opaque_data)
{
	struct rtnl_link_ifmap map;
	const unsigned int sizeof_ifmap =
		offsetofend(struct rtnl_link_ifmap, port);

	if (len < sizeof_ifmap)
		return false;
	else if (!umoven_or_printaddr(tcp, addr, sizeof_ifmap, &map)) {
		tprint_struct_begin();
		PRINT_FIELD_X(map, mem_start);
		tprint_struct_next();
		PRINT_FIELD_X(map, mem_end);
		tprint_struct_next();
		PRINT_FIELD_X(map, base_addr);
		tprint_struct_next();
		PRINT_FIELD_U(map, irq);
		tprint_struct_next();
		PRINT_FIELD_U(map, dma);
		tprint_struct_next();
		PRINT_FIELD_U(map, port);
		tprint_struct_end();
	}

	return true;
}

static void
update_ctx_str(struct tcb *const tcp,
	       const kernel_ulong_t addr, const unsigned int len,
	       char *const str, const size_t sz)
{
	memset(str, '\0', sz);

	if (len >= sz)
		return;

	if (umoven(tcp, addr, len, str) < 0 || strnlen(str, sz) > len) {
		/*
		 * If we haven't seen NUL or an error occurred, set str
		 * to an empty string.
		 */
		str[0] = '\0';
		return;
	}
}

static bool
decode_nla_linkinfo_kind(struct tcb *const tcp,
			 const kernel_ulong_t addr,
			 const unsigned int len,
			 const void *const opaque_data)
{
	struct ifla_linkinfo_ctx *ctx = (void *) opaque_data;

	update_ctx_str(tcp, addr, len, ARRSZ_PAIR(ctx->kind));

	printstr_ex(tcp, addr, len, QUOTE_0_TERMINATED);

	return true;
}

static bool
decode_nla_linkinfo_xstats_can(struct tcb *const tcp,
			       const kernel_ulong_t addr,
			       const unsigned int len,
			       const void *const opaque_data)
{
	struct strace_can_device_stats {
		uint32_t bus_error;
		uint32_t error_warning;
		uint32_t error_passive;
		uint32_t bus_off;
		uint32_t arbitration_lost;
		uint32_t restarts;
	} st;
	const unsigned int def_size = sizeof(st);
	const unsigned int size = (len >= def_size) ? def_size : 0;

	if (!size)
		return false;

	if (umoven_or_printaddr(tcp, addr, size, &st))
		return true;

	tprint_struct_begin();
	PRINT_FIELD_U(st, bus_error);
	tprint_struct_next();
	PRINT_FIELD_U(st, error_warning);
	tprint_struct_next();
	PRINT_FIELD_U(st, error_passive);
	tprint_struct_next();
	PRINT_FIELD_U(st, bus_off);
	tprint_struct_next();
	PRINT_FIELD_U(st, arbitration_lost);
	tprint_struct_next();
	PRINT_FIELD_U(st, restarts);
	tprint_struct_end();

	return true;
}

static bool
decode_nla_linkinfo_xstats(struct tcb *const tcp,
			   const kernel_ulong_t addr,
			   const unsigned int len,
			   const void *const opaque_data)
{
	struct ifla_linkinfo_ctx *ctx = (void *) opaque_data;
	nla_decoder_t func = NULL;

	if (!strcmp(ctx->kind, "can"))
		func = decode_nla_linkinfo_xstats_can;

	if (func)
		return func(tcp, addr, len, opaque_data);

	return false;
}

static bool
decode_ifla_br_boolopt(struct tcb *const tcp,
		       const kernel_ulong_t addr,
		       const unsigned int len,
		       const void *const opaque_data)
{
	struct br_boolopt_multi bom;

	if (len < sizeof(bom))
		return false;

	if (umoven_or_printaddr(tcp, addr, sizeof(bom), &bom))
		return true;

	tprint_struct_begin();
	PRINT_FIELD_FLAGS(bom, optval, rtnl_ifla_br_boolopt_flags,
			  "1<<BR_BOOLOPT_???");
	tprint_struct_next();
	PRINT_FIELD_FLAGS(bom, optmask, rtnl_ifla_br_boolopt_flags,
			  "1<<BR_BOOLOPT_???");
	tprint_struct_end();

	return true;
}

static const nla_decoder_t ifla_br_mcast_querier_decoders[] = {
	[BRIDGE_QUERIER_UNSPEC]			= NULL,
	[BRIDGE_QUERIER_IP_ADDRESS]		= decode_nla_in_addr,
	[BRIDGE_QUERIER_IP_PORT]		= decode_nla_ifindex,
	[BRIDGE_QUERIER_IP_OTHER_TIMER]		= decode_nla_clock_t,
	[BRIDGE_QUERIER_PAD]			= NULL,
	[BRIDGE_QUERIER_IPV6_ADDRESS]		= decode_nla_in6_addr,
	[BRIDGE_QUERIER_IPV6_PORT]		= decode_nla_ifindex,
	[BRIDGE_QUERIER_IPV6_OTHER_TIMER]	= decode_nla_clock_t,
};

static bool
decode_ifla_br_mcast_qstate(struct tcb *const tcp,
			    const kernel_ulong_t addr,
			    const unsigned int len,
			    const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, rtnl_ifla_br_mcast_querier_attrs,
		      "BRIDGE_QUERIER_???",
		      ARRSZ_PAIR(ifla_br_mcast_querier_decoders), opaque_data);

	return true;
}

static const nla_decoder_t ifla_info_data_bridge_nla_decoders[] = {
	[IFLA_BR_UNSPEC]			= NULL,
	[IFLA_BR_FORWARD_DELAY]			= decode_nla_clock_t,
	[IFLA_BR_HELLO_TIME]			= decode_nla_clock_t,
	[IFLA_BR_MAX_AGE]			= decode_nla_clock_t,
	[IFLA_BR_AGEING_TIME]			= decode_nla_clock_t,
	[IFLA_BR_STP_STATE]			= decode_nla_u32,
	[IFLA_BR_PRIORITY]			= decode_nla_u16,
	[IFLA_BR_VLAN_FILTERING]		= decode_nla_u8,
	[IFLA_BR_VLAN_PROTOCOL]			= decode_nla_ether_proto,
	[IFLA_BR_GROUP_FWD_MASK]		= decode_nla_x16,
	[IFLA_BR_ROOT_ID]			= decode_ifla_bridge_id,
	[IFLA_BR_BRIDGE_ID]			= decode_ifla_bridge_id,
	[IFLA_BR_ROOT_PORT]			= decode_nla_u16,
	[IFLA_BR_ROOT_PATH_COST]		= decode_nla_u32,
	[IFLA_BR_TOPOLOGY_CHANGE]		= decode_nla_u8,
	[IFLA_BR_TOPOLOGY_CHANGE_DETECTED]	= decode_nla_u8,
	[IFLA_BR_HELLO_TIMER]			= decode_nla_clock_t,
	[IFLA_BR_TCN_TIMER]			= decode_nla_clock_t,
	[IFLA_BR_TOPOLOGY_CHANGE_TIMER]		= decode_nla_clock_t,
	[IFLA_BR_GC_TIMER]			= decode_nla_clock_t,
	[IFLA_BR_GROUP_ADDR]			= decode_nla_hwaddr_nofamily,
	[IFLA_BR_FDB_FLUSH]			= NULL, /* unspecified */
	[IFLA_BR_MCAST_ROUTER]			= decode_nla_u8,
	[IFLA_BR_MCAST_SNOOPING]		= decode_nla_u8,
	[IFLA_BR_MCAST_QUERY_USE_IFADDR]	= decode_nla_u8,
	[IFLA_BR_MCAST_QUERIER]			= decode_nla_u8,
	[IFLA_BR_MCAST_HASH_ELASTICITY]		= decode_nla_u32,
	[IFLA_BR_MCAST_HASH_MAX]		= decode_nla_u32,
	[IFLA_BR_MCAST_LAST_MEMBER_CNT]		= decode_nla_u32,
	[IFLA_BR_MCAST_STARTUP_QUERY_CNT]	= decode_nla_u32,
	[IFLA_BR_MCAST_LAST_MEMBER_INTVL]	= decode_nla_clock_t,
	[IFLA_BR_MCAST_MEMBERSHIP_INTVL]	= decode_nla_clock_t,
	[IFLA_BR_MCAST_QUERIER_INTVL]		= decode_nla_clock_t,
	[IFLA_BR_MCAST_QUERY_INTVL]		= decode_nla_clock_t,
	[IFLA_BR_MCAST_QUERY_RESPONSE_INTVL]	= decode_nla_clock_t,
	[IFLA_BR_MCAST_STARTUP_QUERY_INTVL]	= decode_nla_clock_t,
	[IFLA_BR_NF_CALL_IPTABLES]		= decode_nla_u8,
	[IFLA_BR_NF_CALL_IP6TABLES]		= decode_nla_u8,
	[IFLA_BR_NF_CALL_ARPTABLES]		= decode_nla_u8,
	[IFLA_BR_VLAN_DEFAULT_PVID]		= decode_nla_u16,
	[IFLA_BR_PAD]				= NULL,
	[IFLA_BR_VLAN_STATS_ENABLED]		= decode_nla_u8,
	[IFLA_BR_MCAST_STATS_ENABLED]		= decode_nla_u8,
	[IFLA_BR_MCAST_IGMP_VERSION]		= decode_nla_u8,
	[IFLA_BR_MCAST_MLD_VERSION]		= decode_nla_u8,
	[IFLA_BR_VLAN_STATS_PER_PORT]		= decode_nla_u8,
	[IFLA_BR_MULTI_BOOLOPT]			= decode_ifla_br_boolopt,
	[IFLA_BR_MCAST_QUERIER_STATE]		= decode_ifla_br_mcast_qstate,
};

static bool
decode_nla_linkinfo_data_bridge(struct tcb *const tcp,
				const kernel_ulong_t addr,
				const unsigned int len,
				const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, rtnl_ifla_info_data_bridge_attrs,
		      "IFLA_BR_???",
		      ARRSZ_PAIR(ifla_info_data_bridge_nla_decoders),
		      opaque_data);

	return true;
}

static bool
decode_nla_tun_type(struct tcb *const tcp,
		    const kernel_ulong_t addr,
		    const unsigned int len,
		    const void *const opaque_data)
{
	static const struct decode_nla_xlat_opts opts = {
		.xlat = tun_device_types,
		.dflt = "IFF_???",
		.size = 1,
	};

	return decode_nla_xval(tcp, addr, len, &opts);
}

static const nla_decoder_t ifla_info_data_tun_nla_decoders[] = {
	[IFLA_TUN_UNSPEC]		= NULL,
	[IFLA_TUN_OWNER]		= decode_nla_uid,
	[IFLA_TUN_GROUP]		= decode_nla_gid,
	[IFLA_TUN_TYPE]			= decode_nla_tun_type,
	[IFLA_TUN_PI]			= decode_nla_u8,
	[IFLA_TUN_VNET_HDR]		= decode_nla_u8,
	[IFLA_TUN_PERSIST]		= decode_nla_u8,
	[IFLA_TUN_MULTI_QUEUE]		= decode_nla_u8,
	[IFLA_TUN_NUM_QUEUES]		= decode_nla_u32,
	[IFLA_TUN_NUM_DISABLED_QUEUES]	= decode_nla_u32,
};

static bool
decode_nla_linkinfo_data_tun(struct tcb *const tcp,
			     const kernel_ulong_t addr,
			     const unsigned int len,
			     const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, rtnl_ifla_info_data_tun_attrs,
		      "IFLA_TUN_???",
		      ARRSZ_PAIR(ifla_info_data_tun_nla_decoders),
		      opaque_data);

	return true;
}

static bool
decode_nla_linkinfo_data(struct tcb *const tcp,
			 const kernel_ulong_t addr,
			 const unsigned int len,
			 const void *const opaque_data)
{
	struct ifla_linkinfo_ctx *ctx = (void *) opaque_data;
	nla_decoder_t func = NULL;

	if (!strcmp(ctx->kind, "bridge"))
		func = decode_nla_linkinfo_data_bridge;
	else if (!strcmp(ctx->kind, "tun"))
		func = decode_nla_linkinfo_data_tun;

	if (func)
		return func(tcp, addr, len, opaque_data);

	return false;
}

static bool
decode_nla_linkinfo_slave_kind(struct tcb *const tcp,
			       const kernel_ulong_t addr,
			       const unsigned int len,
			       const void *const opaque_data)
{
	struct ifla_linkinfo_ctx *ctx = (void *) opaque_data;

	update_ctx_str(tcp, addr, len, ARRSZ_PAIR(ctx->slave_kind));

	printstr_ex(tcp, addr, len, QUOTE_0_TERMINATED);

	return true;
}

static bool
decode_nla_linkinfo_slave_data_bridge(struct tcb *const tcp,
				      const kernel_ulong_t addr,
				      const unsigned int len,
				      const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, rtnl_ifla_brport_attrs,
		      "IFLA_BRPORT_???",
		      ARRSZ_PAIR(ifla_brport_nla_decoders),
		      opaque_data);

	return true;
}

static bool
decode_nla_linkinfo_slave_data(struct tcb *const tcp,
			       const kernel_ulong_t addr,
			       const unsigned int len,
			       const void *const opaque_data)
{
	struct ifla_linkinfo_ctx *ctx = (void *) opaque_data;
	nla_decoder_t func = NULL;

	if (!strcmp(ctx->slave_kind, "bridge"))
		func = decode_nla_linkinfo_slave_data_bridge;

	if (func)
		return func(tcp, addr, len, opaque_data);

	return false;
}

static const nla_decoder_t ifla_linkinfo_nla_decoders[] = {
	[IFLA_INFO_KIND]	= decode_nla_linkinfo_kind,
	[IFLA_INFO_DATA]	= decode_nla_linkinfo_data,
	[IFLA_INFO_XSTATS]	= decode_nla_linkinfo_xstats,
	[IFLA_INFO_SLAVE_KIND]	= decode_nla_linkinfo_slave_kind,
	[IFLA_INFO_SLAVE_DATA]	= decode_nla_linkinfo_slave_data,
};

static bool
decode_ifla_linkinfo(struct tcb *const tcp,
		     const kernel_ulong_t addr,
		     const unsigned int len,
		     const void *const opaque_data)
{
	struct ifla_linkinfo_ctx ctx = { .kind = "", };

	decode_nlattr(tcp, addr, len, rtnl_ifla_info_attrs,
		      "IFLA_INFO_???", ARRSZ_PAIR(ifla_linkinfo_nla_decoders),
		      &ctx);

	return true;
}

static bool
decode_ifla_vf_mac(struct tcb *const tcp,
		   const kernel_ulong_t addr,
		   const unsigned int len,
		   const void *const opaque_data)
{
	struct ifla_vf_mac ivm;

	if (len < sizeof(ivm))
		return false;
	if (umove_or_printaddr(tcp, addr, &ivm))
		return true;

	tprint_struct_begin();
	PRINT_FIELD_U(ivm, vf);
	tprint_struct_next();
	PRINT_FIELD_MAC(ivm, mac);
	tprint_struct_end();

	return true;
}

static bool
decode_ifla_vf_vlan(struct tcb *const tcp,
		    const kernel_ulong_t addr,
		    const unsigned int len,
		    const void *const opaque_data)
{
	struct ifla_vf_vlan ivv;

	if (len < sizeof(ivv))
		return false;
	if (umove_or_printaddr(tcp, addr, &ivv))
		return true;

	tprint_struct_begin();
	PRINT_FIELD_U(ivv, vf);
	tprint_struct_next();
	PRINT_FIELD_U(ivv, vlan);
	tprint_struct_next();
	PRINT_FIELD_U(ivv, qos);
	tprint_struct_end();

	return true;
}

static bool
decode_ifla_vf_tx_rate(struct tcb *const tcp,
		       const kernel_ulong_t addr,
		       const unsigned int len,
		       const void *const opaque_data)
{
	struct ifla_vf_tx_rate ivtr;

	if (len < sizeof(ivtr))
		return false;
	if (umove_or_printaddr(tcp, addr, &ivtr))
		return true;

	tprint_struct_begin();
	PRINT_FIELD_U(ivtr, vf);
	tprint_struct_next();
	PRINT_FIELD_U(ivtr, rate);
	tprint_struct_end();

	return true;
}

static bool
decode_ifla_vf_spoofchk(struct tcb *const tcp,
			const kernel_ulong_t addr,
			const unsigned int len,
			const void *const opaque_data)
{
	struct ifla_vf_spoofchk ivs;

	if (len < sizeof(ivs))
		return false;
	if (umove_or_printaddr(tcp, addr, &ivs))
		return true;

	tprint_struct_begin();
	PRINT_FIELD_U(ivs, vf);
	tprint_struct_next();
	PRINT_FIELD_U(ivs, setting);
	tprint_struct_end();

	return true;
}

static bool
decode_ifla_vf_link_state(struct tcb *const tcp,
			  const kernel_ulong_t addr,
			  const unsigned int len,
			  const void *const opaque_data)
{
	struct ifla_vf_link_state ivls;

	if (len < sizeof(ivls))
		return false;
	if (umove_or_printaddr(tcp, addr, &ivls))
		return true;

	tprint_struct_begin();
	PRINT_FIELD_U(ivls, vf);
	tprint_struct_next();
	PRINT_FIELD_XVAL(ivls, link_state, rtnl_ifla_vf_link_states,
			 "IFLA_VF_LINK_STATE_???");
	tprint_struct_end();

	return true;
}

static bool
decode_ifla_vf_rate(struct tcb *const tcp,
		    const kernel_ulong_t addr,
		    const unsigned int len,
		    const void *const opaque_data)
{
	struct ifla_vf_rate ivr;

	if (len < sizeof(ivr))
		return false;
	if (umove_or_printaddr(tcp, addr, &ivr))
		return true;

	tprint_struct_begin();
	PRINT_FIELD_U(ivr, vf);
	tprint_struct_next();
	PRINT_FIELD_U(ivr, min_tx_rate);
	tprint_struct_next();
	PRINT_FIELD_U(ivr, max_tx_rate);
	tprint_struct_end();

	return true;
}

static bool
decode_ifla_vf_rss_query_en(struct tcb *const tcp,
			    const kernel_ulong_t addr,
			    const unsigned int len,
			    const void *const opaque_data)
{
	struct ifla_vf_rss_query_en ivrqe;

	if (len < sizeof(ivrqe))
		return false;
	if (umove_or_printaddr(tcp, addr, &ivrqe))
		return true;

	tprint_struct_begin();
	PRINT_FIELD_U(ivrqe, vf);
	tprint_struct_next();
	PRINT_FIELD_U(ivrqe, setting);
	tprint_struct_end();

	return true;
}

static const nla_decoder_t ifla_vf_stats_nla_decoders[] = {
	[IFLA_VF_STATS_RX_PACKETS]	= decode_nla_u64,
	[IFLA_VF_STATS_TX_PACKETS]	= decode_nla_u64,
	[IFLA_VF_STATS_RX_BYTES]	= decode_nla_u64,
	[IFLA_VF_STATS_TX_BYTES]	= decode_nla_u64,
	[IFLA_VF_STATS_BROADCAST]	= decode_nla_u64,
	[IFLA_VF_STATS_MULTICAST]	= decode_nla_u64,
	[IFLA_VF_STATS_PAD]		= NULL,
	[IFLA_VF_STATS_RX_DROPPED]	= decode_nla_u64,
	[IFLA_VF_STATS_TX_DROPPED]	= decode_nla_u64,
};

static bool
decode_ifla_vf_stats(struct tcb *const tcp,
		     const kernel_ulong_t addr,
		     const unsigned int len,
		     const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, rtnl_ifla_vf_stats_attrs,
		      "IFLA_VF_STATS_???",
		      ARRSZ_PAIR(ifla_vf_stats_nla_decoders),
		      opaque_data);

	return true;
}

static bool
decode_ifla_vf_trust(struct tcb *const tcp,
		     const kernel_ulong_t addr,
		     const unsigned int len,
		     const void *const opaque_data)
{
	struct ifla_vf_trust ivt;

	if (len < sizeof(ivt))
		return false;
	if (umove_or_printaddr(tcp, addr, &ivt))
		return true;

	tprint_struct_begin();
	PRINT_FIELD_U(ivt, vf);
	tprint_struct_next();
	PRINT_FIELD_U(ivt, setting);
	tprint_struct_end();

	return true;
}

static bool
decode_ifla_vf_guid(struct tcb *const tcp,
		    const kernel_ulong_t addr,
		    const unsigned int len,
		    const void *const opaque_data)
{
	/*
	 * This all is broken because struct ifla_vf_guid.guid is not naturally
	 * aligned;  trying to handle both possible attribute sizes.
	 */
	union {
		struct {
			uint32_t vf;
			uint64_t guid;
		} ATTRIBUTE_PACKED ivg_32;
		struct {
			uint32_t vf;
			uint64_t ATTRIBUTE_ALIGNED(8) guid;
		} ivg_64;
	} ivg;

	static_assert(sizeof(struct ifla_vf_guid) == sizeof(ivg.ivg_32)
		      || sizeof(struct ifla_vf_guid) == sizeof(ivg.ivg_64),
		      "Unexpected struct ifla_vf_guid size");
	CHECK_TYPE_SIZE(ivg.ivg_32, 12);
	CHECK_TYPE_SIZE(ivg.ivg_64, 16);

	switch (len) {
	case sizeof(ivg.ivg_32):
	case sizeof(ivg.ivg_64):
		break;
	default:
		return false;
	}

	if (umoven_or_printaddr(tcp, addr, len, &ivg))
		return true;

	switch (len) {
	case sizeof(ivg.ivg_32):
		tprint_struct_begin();
		PRINT_FIELD_U(ivg.ivg_32, vf);
		tprint_struct_next();
		PRINT_FIELD_X(ivg.ivg_32, guid);
		tprint_struct_end();
		break;

	case sizeof(ivg.ivg_64):
		tprint_struct_begin();
		PRINT_FIELD_U(ivg.ivg_64, vf);
		tprint_struct_next();
		PRINT_FIELD_X(ivg.ivg_64, guid);
		tprint_struct_end();
		break;
	}

	return true;
}

static bool
decode_ifla_vf_vlan_info(struct tcb *const tcp,
			 const kernel_ulong_t addr,
			 const unsigned int len,
			 const void *const opaque_data)
{
	struct ifla_vf_vlan_info ivvi;

	if (len < sizeof(ivvi))
		return false;
	if (umove_or_printaddr(tcp, addr, &ivvi))
		return true;

	tprint_struct_begin();
	PRINT_FIELD_U(ivvi, vf);
	tprint_struct_next();
	PRINT_FIELD_U(ivvi, vlan);
	tprint_struct_next();
	PRINT_FIELD_U(ivvi, qos);
	tprint_struct_next();
	tprints_field_name("vlan_proto");
	tprints_arg_begin("htons");
	printxval(ethernet_protocols, ntohs(ivvi.vlan_proto), "ETH_P_???");
	tprint_arg_end();
	tprint_struct_end();

	return true;
}

static const nla_decoder_t ifla_vf_vlan_list_nla_decoders[] = {
	[IFLA_VF_VLAN_INFO_UNSPEC]	= NULL,
	[IFLA_VF_VLAN_INFO]		= decode_ifla_vf_vlan_info,
};

static bool
decode_ifla_vf_vlan_list(struct tcb *const tcp,
			 const kernel_ulong_t addr,
			 const unsigned int len,
			 const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, rtnl_ifla_vf_vlan_list_attrs,
		      "IFLA_VF_VLAN_INFO_???",
		      ARRSZ_PAIR(ifla_vf_vlan_list_nla_decoders),
		      opaque_data);

	return true;
}

static bool
decode_ifla_vf_broadcast(struct tcb *const tcp,
			 const kernel_ulong_t addr,
			 const unsigned int len,
			 const void *const opaque_data)
{
	struct ifla_vf_broadcast ivb;

	if (len < sizeof(ivb))
		return false;
	if (umove_or_printaddr(tcp, addr, &ivb))
		return true;

	tprint_struct_begin();
	PRINT_FIELD_MAC(ivb, broadcast);
	tprint_struct_end();

	return true;
}

static const nla_decoder_t ifla_vf_info_nla_decoders[] = {
	[IFLA_VF_UNSPEC]	= NULL,
	[IFLA_VF_MAC]		= decode_ifla_vf_mac,
	[IFLA_VF_VLAN]		= decode_ifla_vf_vlan,
	[IFLA_VF_TX_RATE]	= decode_ifla_vf_tx_rate,
	[IFLA_VF_SPOOFCHK]	= decode_ifla_vf_spoofchk,
	[IFLA_VF_LINK_STATE]	= decode_ifla_vf_link_state,
	[IFLA_VF_RATE]		= decode_ifla_vf_rate,
	[IFLA_VF_RSS_QUERY_EN]	= decode_ifla_vf_rss_query_en,
	[IFLA_VF_STATS]		= decode_ifla_vf_stats,
	[IFLA_VF_TRUST]		= decode_ifla_vf_trust,
	[IFLA_VF_IB_NODE_GUID]	= decode_ifla_vf_guid,
	[IFLA_VF_IB_PORT_GUID]	= decode_ifla_vf_guid,
	[IFLA_VF_VLAN_LIST]	= decode_ifla_vf_vlan_list,
	[IFLA_VF_BROADCAST]	= decode_ifla_vf_broadcast,
};

static bool
decode_ifla_vf_info(struct tcb *const tcp,
		    const kernel_ulong_t addr,
		    const unsigned int len,
		    const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, rtnl_ifla_vf_info_attrs,
		      "IFLA_VF_???", ARRSZ_PAIR(ifla_vf_info_nla_decoders),
		      opaque_data);

	return true;
}

static const nla_decoder_t ifla_vfinfo_list_nla_decoders[] = {
	[IFLA_VF_INFO_UNSPEC]	= NULL,
	[IFLA_VF_INFO]		= decode_ifla_vf_info,
};

static bool
decode_ifla_vfinfo_list(struct tcb *const tcp,
			const kernel_ulong_t addr,
			const unsigned int len,
			const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, rtnl_ifla_vfinfo_list_attrs,
		      "IFLA_VF_INFO_???",
		      ARRSZ_PAIR(ifla_vfinfo_list_nla_decoders),
		      opaque_data);

	return true;
}

bool
decode_nla_rtnl_link_stats64(struct tcb *const tcp,
			     const kernel_ulong_t addr,
			     const unsigned int len,
			     const void *const opaque_data)
{
	struct rtnl_link_stats64 st;
	const unsigned int min_size =
		offsetofend(struct rtnl_link_stats64, tx_compressed);
	const unsigned int rx_nohandler_size =
		offsetofend(struct rtnl_link_stats64, rx_nohandler);
	const unsigned int def_size = sizeof(st);
	const unsigned int size =
		(len >= def_size)
			? def_size
			: ((len == rx_nohandler_size)
				? rx_nohandler_size
				: ((len == min_size) ? min_size : 0));

	if (!size)
		return false;

	if (!umoven_or_printaddr(tcp, addr, size, &st)) {
		tprint_struct_begin();
		PRINT_FIELD_U(st, rx_packets);
		tprint_struct_next();
		PRINT_FIELD_U(st, tx_packets);
		tprint_struct_next();
		PRINT_FIELD_U(st, rx_bytes);
		tprint_struct_next();
		PRINT_FIELD_U(st, tx_bytes);
		tprint_struct_next();
		PRINT_FIELD_U(st, rx_errors);
		tprint_struct_next();
		PRINT_FIELD_U(st, tx_errors);
		tprint_struct_next();
		PRINT_FIELD_U(st, rx_dropped);
		tprint_struct_next();
		PRINT_FIELD_U(st, tx_dropped);
		tprint_struct_next();
		PRINT_FIELD_U(st, multicast);
		tprint_struct_next();
		PRINT_FIELD_U(st, collisions);

		tprint_struct_next();
		PRINT_FIELD_U(st, rx_length_errors);
		tprint_struct_next();
		PRINT_FIELD_U(st, rx_over_errors);
		tprint_struct_next();
		PRINT_FIELD_U(st, rx_crc_errors);
		tprint_struct_next();
		PRINT_FIELD_U(st, rx_frame_errors);
		tprint_struct_next();
		PRINT_FIELD_U(st, rx_fifo_errors);
		tprint_struct_next();
		PRINT_FIELD_U(st, rx_missed_errors);

		tprint_struct_next();
		PRINT_FIELD_U(st, tx_aborted_errors);
		tprint_struct_next();
		PRINT_FIELD_U(st, tx_carrier_errors);
		tprint_struct_next();
		PRINT_FIELD_U(st, tx_fifo_errors);
		tprint_struct_next();
		PRINT_FIELD_U(st, tx_heartbeat_errors);
		tprint_struct_next();
		PRINT_FIELD_U(st, tx_window_errors);

		tprint_struct_next();
		PRINT_FIELD_U(st, rx_compressed);
		tprint_struct_next();
		PRINT_FIELD_U(st, tx_compressed);

		if (len >= rx_nohandler_size) {
			tprint_struct_next();
			PRINT_FIELD_U(st, rx_nohandler);

			if (len >= def_size) {
				tprint_struct_next();
				PRINT_FIELD_U(st, rx_otherhost_dropped);
			}
		}
		tprint_struct_end();
	}

	return true;
}

static bool
decode_ifla_port_vsi(struct tcb *const tcp,
		     const kernel_ulong_t addr,
		     const unsigned int len,
		     const void *const opaque_data)
{
	struct ifla_port_vsi vsi;

	if (len < sizeof(vsi))
		return false;
	if (umove_or_printaddr(tcp, addr, &vsi))
		return true;

	tprint_struct_begin();
	PRINT_FIELD_U(vsi, vsi_mgr_id);
	tprint_struct_next();
	PRINT_FIELD_STRING(vsi, vsi_type_id, sizeof(vsi.vsi_type_id),
			   QUOTE_FORCE_HEX);
	tprint_struct_next();
	PRINT_FIELD_U(vsi, vsi_type_version);

	if (!IS_ARRAY_ZERO(vsi.pad)) {
		tprint_struct_next();
		PRINT_FIELD_HEX_ARRAY(vsi, pad);
	}

	tprint_struct_end();

	return true;
}

static const nla_decoder_t ifla_port_nla_decoders[] = {
	[IFLA_PORT_VF]			= decode_nla_u32,
	[IFLA_PORT_PROFILE]		= decode_nla_str,
	[IFLA_PORT_VSI_TYPE]		= decode_ifla_port_vsi,
	[IFLA_PORT_INSTANCE_UUID]	= NULL, /* default parser */
	[IFLA_PORT_HOST_UUID]		= NULL, /* default parser */
	[IFLA_PORT_REQUEST]		= decode_nla_u8,
	[IFLA_PORT_RESPONSE]		= decode_nla_u16
};

static bool
decode_ifla_port(struct tcb *const tcp,
		 const kernel_ulong_t addr,
		 const unsigned int len,
		 const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, rtnl_ifla_port_attrs,
		      "IFLA_VF_PORT_???", ARRSZ_PAIR(ifla_port_nla_decoders),
		      opaque_data);

	return true;
}

static const nla_decoder_t ifla_vf_port_nla_decoders[] = {
	[IFLA_VF_PORT] = decode_ifla_port
};

static bool
decode_ifla_vf_ports(struct tcb *const tcp,
		     const kernel_ulong_t addr,
		     const unsigned int len,
		     const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, rtnl_ifla_vf_port_attrs,
		      "IFLA_VF_PORT_???", ARRSZ_PAIR(ifla_vf_port_nla_decoders),
		      opaque_data);

	return true;
}

static bool
decode_ifla_ext_mask(struct tcb *const tcp,
		     const kernel_ulong_t addr,
		     const unsigned int len,
		     const void *const opaque_data)
{
	static const struct decode_nla_xlat_opts opts = {
		rtnl_ifla_ext_filter_flags, "RTEXT_FILTER_???", .size = 4,
	};

	return decode_nla_flags(tcp, addr, len, &opts);
}

static bool
decode_ifla_xdp_flags(struct tcb *const tcp,
		      const kernel_ulong_t addr,
		      const unsigned int len,
		      const void *const opaque_data)
{
	uint32_t flags;

	if (len < sizeof(flags))
		return false;
	else if (!umove_or_printaddr(tcp, addr, &flags))
		printflags(xdp_flags, flags, "XDP_FLAGS_???");

	return true;
}

static bool
decode_ifla_xdp_attached(struct tcb *const tcp,
			 const kernel_ulong_t addr,
			 const unsigned int len,
			 const void *const opaque_data)
{
	static const struct decode_nla_xlat_opts opts = {
		.xlat = rtnl_ifla_xdp_attached_mode,
		.dflt = "XDP_ATTACHED_???",
		.size = 1,
	};

	return decode_nla_xval(tcp, addr, len, &opts);
}

static const nla_decoder_t ifla_xdp_nla_decoders[] = {
	[IFLA_XDP_FD]		= decode_nla_fd,
	[IFLA_XDP_ATTACHED]	= decode_ifla_xdp_attached,
	[IFLA_XDP_FLAGS]	= decode_ifla_xdp_flags,
	[IFLA_XDP_PROG_ID]	= decode_nla_u32,
	[IFLA_XDP_DRV_PROG_ID]  = decode_nla_u32,
	[IFLA_XDP_SKB_PROG_ID]  = decode_nla_u32,
	[IFLA_XDP_HW_PROG_ID]   = decode_nla_u32,
	[IFLA_XDP_EXPECTED_FD]	= decode_nla_fd,
};

static bool
decode_ifla_xdp(struct tcb *const tcp,
		const kernel_ulong_t addr,
		const unsigned int len,
		const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, rtnl_ifla_xdp_attrs,
		      "IFLA_XDP_???", ARRSZ_PAIR(ifla_xdp_nla_decoders),
		      opaque_data);

	return true;
}

static bool
decode_ifla_event(struct tcb *const tcp,
		  const kernel_ulong_t addr,
		  const unsigned int len,
		  const void *const opaque_data)
{
	uint32_t ev;

	if (len < sizeof(ev))
		return false;
	else if (!umove_or_printaddr(tcp, addr, &ev))
		printxval(rtnl_ifla_events, ev, "IFLA_EVENT_???");

	return true;
}


static bool
decode_ifla_inet_conf(struct tcb *const tcp,
		      const kernel_ulong_t addr,
		      const unsigned int len,
		      const void *const opaque_data)
{
	int elem;
	size_t cnt = len / sizeof(elem);

	if (!cnt)
		return false;

	print_array_ex(tcp, addr, cnt, &elem, sizeof(elem),
		       tfetch_mem, print_int_array_member, NULL,
		       PAF_PRINT_INDICES | XLAT_STYLE_FMT_D,
		       inet_devconf_indices, "IPV4_DEVCONF_???");

	return true;
}

static const nla_decoder_t ifla_inet_nla_decoders[] = {
	[IFLA_INET_CONF] = decode_ifla_inet_conf,
};

static const nla_decoder_t ifla_mctp_nla_decoders[] = {
	[IFLA_MCTP_UNSPEC]	= NULL,
	[IFLA_MCTP_NET]		= decode_nla_u32,
};

static bool
decode_ifla_af(struct tcb *const tcp,
	       const kernel_ulong_t addr,
	       const unsigned int len,
	       const void *const opaque_data)
{
	static const struct af_spec_decoder_desc protos[] = {
		{ AF_INET,  rtnl_ifla_af_spec_inet_attrs,  "IFLA_INET_???",
		  ARRSZ_PAIR(ifla_inet_nla_decoders) },
		{ AF_INET6, rtnl_ifla_af_spec_inet6_attrs, "IFLA_INET6_???",
		  ARRSZ_PAIR(ifla_inet6_nla_decoders) },
		{ AF_MCTP,  rtnl_ifla_af_spec_mctp_attrs,  "IFLA_MCTP_???",
		  ARRSZ_PAIR(ifla_mctp_nla_decoders) },
	};

	decode_nla_af_spec(tcp, addr, len,
			   (uintptr_t) opaque_data, ARRSZ_PAIR(protos));

	return true;
}

static bool
decode_ifla_bridge_flags(struct tcb *const tcp,
			 const kernel_ulong_t addr,
			 const unsigned int len,
			 const void *const opaque_data)
{
	static const struct decode_nla_xlat_opts opts = {
		rtnl_ifla_bridge_flags, "BRIDGE_FLAGS_???", .size = 2,
	};

	return decode_nla_flags(tcp, addr, len, &opts);
}

static bool
decode_ifla_bridge_mode(struct tcb *const tcp,
			const kernel_ulong_t addr,
			const unsigned int len,
			const void *const opaque_data)
{
	static const struct decode_nla_xlat_opts opts = {
		rtnl_ifla_bridge_modes, "BRIDGE_MODE_???", .size = 2,
	};

	return decode_nla_xval(tcp, addr, len, &opts);
}

static bool
decode_ifla_bridge_vlan_info(struct tcb *const tcp,
			     const kernel_ulong_t addr,
			     const unsigned int len,
			     const void *const opaque_data)
{
	struct bridge_vlan_info bvi;

	if (len < sizeof(bvi))
		return false;

	if (umove_or_printaddr(tcp, addr, &bvi))
		return true;

	tprint_struct_begin();
	PRINT_FIELD_FLAGS(bvi, flags, nl_bridge_vlan_flags,
			  "BRIDGE_VLAN_INFO_???");
	tprint_struct_next();
	PRINT_FIELD_U(bvi, vid);
	tprint_struct_end();

	if (len > sizeof(bvi)) {
		tprint_array_next();
		printstr_ex(tcp, addr + sizeof(bvi), len - sizeof(bvi),
			    QUOTE_FORCE_HEX);
	}

	return true;
}

static bool
decode_bridge_vlan_info_flags(struct tcb *const tcp,
			      const kernel_ulong_t addr,
			      const unsigned int len,
			      const void *const opaque_data)
{
	static const struct decode_nla_xlat_opts opts = {
		nl_bridge_vlan_flags, "BRIDGE_VLAN_INFO_???", .size = 2,
	};

	return decode_nla_flags(tcp, addr, len, &opts);
}

static const nla_decoder_t ifla_af_spec_bridge_vlan_tunnel_info_decoders[] = {
	[IFLA_BRIDGE_VLAN_TUNNEL_UNSPEC]	= NULL,
	[IFLA_BRIDGE_VLAN_TUNNEL_ID]		= decode_nla_u32,
	[IFLA_BRIDGE_VLAN_TUNNEL_VID]		= decode_nla_u16,
	[IFLA_BRIDGE_VLAN_TUNNEL_FLAGS]		= decode_bridge_vlan_info_flags,
};

static bool
decode_ifla_bridge_vlan_tunnel_info(struct tcb *const tcp,
				    const kernel_ulong_t addr,
				    const unsigned int len,
				    const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len,
		      rtnl_ifla_af_spec_bridge_vlan_tunnel_info_attrs,
		      "IFLA_BRIDGE_VLAN_TUNNEL_???",
		      ARRSZ_PAIR(ifla_af_spec_bridge_vlan_tunnel_info_decoders),
		      opaque_data);

	return true;
}

static const nla_decoder_t ifla_af_spec_bridge_nla_decoders[] = {
	[IFLA_BRIDGE_FLAGS]		= decode_ifla_bridge_flags,
	[IFLA_BRIDGE_MODE]		= decode_ifla_bridge_mode,
	[IFLA_BRIDGE_VLAN_INFO]		= decode_ifla_bridge_vlan_info,
	[IFLA_BRIDGE_VLAN_TUNNEL_INFO]	= decode_ifla_bridge_vlan_tunnel_info,
	[IFLA_BRIDGE_MRP]		= NULL, /* unimplemented */
	[IFLA_BRIDGE_CFM]		= NULL, /* unimplemented */
	[IFLA_BRIDGE_MST]		= NULL, /* unimplemented */
};

/**
 * In a wonderful world of netlink interfaces (thanks, the author
 * of Linux commit v3.8-rc1~139^2~542 who completely ignored the original
 * IFLA_AF_SPEC attribute description provided in if_link.h since
 * v2.6.38-rc1~476^2~532!), IFLA_AF_SPEC has different structure depending
 * on context, cf. the original IFLA_AF_SPEC comment in if_link.h
 * and the IFLA_AF_SPEC description in if_bridge.h:
 *
 * <if_link.h>
 * IFLA_AF_SPEC
 *   Contains nested attributes for address family specific attributes.
 *   Each address family may create a attribute with the address family
 *   number as type and create its own attribute structure in it.
 *
 *   Example:
 *   [IFLA_AF_SPEC] = {
 *       [AF_INET] = {
 *           [IFLA_INET_CONF] = ...,
 *       },
 *       [AF_INET6] = {
 *           [IFLA_INET6_FLAGS] = ...,
 *           [IFLA_INET6_CONF] = ...,
 *       }
 *   }
 *
 * <if_bridge.h>
 * Bridge management nested attributes
 * [IFLA_AF_SPEC] = {
 *     [IFLA_BRIDGE_FLAGS]
 *     [IFLA_BRIDGE_MODE]
 *     [IFLA_BRIDGE_VLAN_INFO]
 * }
 */
static bool
decode_ifla_af_spec(struct tcb *const tcp,
		    const kernel_ulong_t addr,
		    const unsigned int len,
		    const void *const opaque_data)
{
	const struct ifinfomsg *ifinfo = (const struct ifinfomsg *) opaque_data;

	/* AF_BRIDGE is a special snowflake */
	if (ifinfo->ifi_family == AF_BRIDGE) {
		decode_nlattr(tcp, addr, len, rtnl_ifla_af_spec_bridge_attrs,
			      "IFLA_BRIDGE_???",
			      ARRSZ_PAIR(ifla_af_spec_bridge_nla_decoders),
			      opaque_data);
	} else {
		nla_decoder_t af_spec_decoder = &decode_ifla_af;

		decode_nlattr(tcp, addr, len, addrfams, "AF_???",
			      &af_spec_decoder, 0, 0);
	}

	return true;
}

static bool
decode_ifla_prop_list_attr(struct tcb *const tcp,
			   const kernel_ulong_t addr,
			   const unsigned int len,
			   const void *const opaque_data)
{
	const uintptr_t type = (uintptr_t) opaque_data;

	switch (type) {
	case IFLA_ALT_IFNAME:
		return decode_nla_str(tcp, addr, len, NULL);
	default:
		return false;
	}

	return true;
}

static bool
decode_ifla_prop_list(struct tcb *const tcp,
		      const kernel_ulong_t addr,
		      const unsigned int len,
		      const void *const opaque_data)
{
	nla_decoder_t ifla_prop_list_decoder = &decode_ifla_prop_list_attr;

	/*
	 * We're using the zero-size decoder list in order to avoid large table,
	 * as IFLA_ALT_IFNAME is the only attribute type we need to decode
	 * inside the IFLA_PROP_LIST attribute so far, and it has rather large
	 * value of 53.
	 */
	decode_nlattr(tcp, addr, len, rtnl_link_attrs, "IFLA_???",
		      &ifla_prop_list_decoder, 0, NULL);

	return true;
}

static const nla_decoder_t ifla_proto_down_reason_nla_decoders[] = {
	[IFLA_PROTO_DOWN_REASON_UNSPEC]	= NULL,
	[IFLA_PROTO_DOWN_REASON_MASK]	= decode_nla_x32,
	[IFLA_PROTO_DOWN_REASON_VALUE]	= decode_nla_x32,
};

static bool
decode_ifla_proto_down_reason(struct tcb *const tcp,
			      const kernel_ulong_t addr,
			      const unsigned int len,
			      const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, rtnl_ifla_proto_down_reason_attrs,
		      "IFLA_PROTO_DOWN_REASON_???",
		      ARRSZ_PAIR(ifla_proto_down_reason_nla_decoders),
		      opaque_data);

	return true;
}

static const nla_decoder_t ifinfomsg_nla_decoders[] = {
	[IFLA_ADDRESS]		= decode_ifla_hwaddr,
	[IFLA_BROADCAST]	= decode_ifla_hwaddr,
	[IFLA_IFNAME]		= decode_nla_str,
	[IFLA_MTU]		= decode_nla_u32,
	[IFLA_LINK]		= decode_nla_u32,
	[IFLA_QDISC]		= decode_nla_str,
	[IFLA_STATS]		= decode_rtnl_link_stats,
	[IFLA_COST]		= NULL, /* unused */
	[IFLA_PRIORITY]		= NULL, /* unused */
	[IFLA_MASTER]		= decode_nla_u32,
	[IFLA_WIRELESS]		= NULL, /* unimplemented */
	[IFLA_PROTINFO]		= decode_ifla_protinfo,
	[IFLA_TXQLEN]		= decode_nla_u32,
	[IFLA_MAP]		= decode_rtnl_link_ifmap,
	[IFLA_WEIGHT]		= decode_nla_u32,
	[IFLA_OPERSTATE]	= decode_nla_u8,
	[IFLA_LINKMODE]		= decode_nla_u8,
	[IFLA_LINKINFO]		= decode_ifla_linkinfo,
	[IFLA_NET_NS_PID]	= decode_nla_u32,
	[IFLA_IFALIAS]		= decode_nla_str,
	[IFLA_NUM_VF]		= decode_nla_u32,
	[IFLA_VFINFO_LIST]	= decode_ifla_vfinfo_list,
	[IFLA_STATS64]		= decode_nla_rtnl_link_stats64,
	[IFLA_VF_PORTS]		= decode_ifla_vf_ports,
	[IFLA_PORT_SELF]	= decode_ifla_port,
	[IFLA_AF_SPEC]		= decode_ifla_af_spec,
	[IFLA_GROUP]		= decode_nla_u32,
	[IFLA_NET_NS_FD]	= decode_nla_fd,
	[IFLA_EXT_MASK]		= decode_ifla_ext_mask,
	[IFLA_PROMISCUITY]	= decode_nla_u32,
	[IFLA_NUM_TX_QUEUES]	= decode_nla_u32,
	[IFLA_NUM_RX_QUEUES]	= decode_nla_u32,
	[IFLA_CARRIER]		= decode_nla_u8,
	[IFLA_PHYS_PORT_ID]	= NULL, /* default parser */
	[IFLA_CARRIER_CHANGES]	= decode_nla_u32,
	[IFLA_PHYS_SWITCH_ID]	= NULL, /* default parser */
	[IFLA_LINK_NETNSID]	= decode_nla_s32,
	[IFLA_PHYS_PORT_NAME]	= decode_nla_str,
	[IFLA_PROTO_DOWN]	= decode_nla_u8,
	[IFLA_GSO_MAX_SEGS]	= decode_nla_u32,
	[IFLA_GSO_MAX_SIZE]	= decode_nla_u32,
	[IFLA_PAD]		= NULL,
	[IFLA_XDP]		= decode_ifla_xdp,
	[IFLA_EVENT]		= decode_ifla_event,
	[IFLA_NEW_NETNSID]	= decode_nla_s32,
	[IFLA_IF_NETNSID]	= decode_nla_s32,
	[IFLA_CARRIER_UP_COUNT]	= decode_nla_u32,
	[IFLA_CARRIER_DOWN_COUNT]	= decode_nla_u32,
	[IFLA_NEW_IFINDEX]	= decode_nla_ifindex,
	[IFLA_MIN_MTU]		= decode_nla_u32,
	[IFLA_MAX_MTU]		= decode_nla_u32,
	[IFLA_PROP_LIST]	= decode_ifla_prop_list,
	[IFLA_ALT_IFNAME]	= decode_nla_str,
	[IFLA_PERM_ADDRESS]	= decode_ifla_hwaddr,
	[IFLA_PROTO_DOWN_REASON]	= decode_ifla_proto_down_reason,
	[IFLA_PARENT_DEV_NAME]	= decode_nla_str,
	[IFLA_PARENT_DEV_BUS_NAME]	= decode_nla_str,
	[IFLA_GRO_MAX_SIZE]	= decode_nla_u32,
	[IFLA_TSO_MAX_SIZE]	= decode_nla_u32,
	[IFLA_TSO_MAX_SEGS]	= decode_nla_u32,
	[IFLA_ALLMULTI]		= decode_nla_u32,
	[IFLA_DEVLINK_PORT]	= decode_nla_u32,
	[IFLA_GSO_IPV4_MAX_SIZE]	= decode_nla_u32,
	[IFLA_GRO_IPV4_MAX_SIZE]	= decode_nla_u32,
};

DECL_NETLINK_ROUTE_DECODER(decode_ifinfomsg)
{
	struct ifinfomsg ifinfo = { .ifi_family = family };
	size_t offset = sizeof(ifinfo.ifi_family);
	bool decode_nla = false;

	tprint_struct_begin();
	PRINT_FIELD_XVAL(ifinfo, ifi_family, addrfams, "AF_???");
	tprint_struct_next();

	if (len >= sizeof(ifinfo)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(ifinfo) - offset,
					 (char *) &ifinfo + offset)) {
			PRINT_FIELD_XVAL(ifinfo, ifi_type,
					 arp_hardware_types, "ARPHRD_???");
			tprint_struct_next();
			PRINT_FIELD_IFINDEX(ifinfo, ifi_index);
			tprint_struct_next();
			PRINT_FIELD_FLAGS(ifinfo, ifi_flags,
					  iffflags, "IFF_???");
			tprint_struct_next();
			PRINT_FIELD_X(ifinfo, ifi_change);
			decode_nla = true;
		}
	} else
		tprint_more_data_follows();
	tprint_struct_end();

	offset = NLMSG_ALIGN(sizeof(ifinfo));
	if (decode_nla && len > offset) {
		tprint_array_next();
		decode_nlattr(tcp, addr + offset, len - offset,
			      rtnl_link_attrs, "IFLA_???",
			      ARRSZ_PAIR(ifinfomsg_nla_decoders), &ifinfo);
	}
}
