/*
 * Copyright (c) 2016 Fabien Siron <fabien.siron@epita.fr>
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2016-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "netlink_route.h"
#include "nlattr.h"
#include "print_fields.h"

#include "netlink.h"

#include <netinet/in.h>

#include <linux/rtnetlink.h>

#include "types/rtnl_link.h"

#include "xlat/in6_addr_gen_mode.h"
#include "xlat/inet_devconf_indices.h"
#include "xlat/inet6_devconf_indices.h"
#include "xlat/inet6_if_flags.h"
#include "xlat/rtnl_ifla_af_spec_inet_attrs.h"
#include "xlat/rtnl_ifla_af_spec_inet6_attrs.h"
#include "xlat/rtnl_ifla_brport_attrs.h"
#include "xlat/rtnl_ifla_events.h"
#include "xlat/rtnl_ifla_info_attrs.h"
#include "xlat/rtnl_ifla_info_data_bridge_attrs.h"
#include "xlat/rtnl_ifla_info_data_tun_attrs.h"
#include "xlat/rtnl_ifla_port_attrs.h"
#include "xlat/rtnl_ifla_vf_port_attrs.h"
#include "xlat/rtnl_ifla_xdp_attached_mode.h"
#include "xlat/rtnl_ifla_xdp_attrs.h"
#include "xlat/rtnl_link_attrs.h"
#include "xlat/snmp_icmp6_stats.h"
#include "xlat/snmp_ip_stats.h"
#include "xlat/tun_device_types.h"
#include "xlat/xdp_flags.h"

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
	struct_rtnl_link_stats st;
	const unsigned int min_size =
		offsetofend(struct_rtnl_link_stats, tx_compressed);
	const unsigned int def_size = sizeof(st);
	const unsigned int size =
		(len >= def_size) ? def_size :
				    ((len == min_size) ? min_size : 0);

	if (!size)
		return false;

	if (!umoven_or_printaddr(tcp, addr, size, &st)) {
		PRINT_FIELD_U("{", st, rx_packets);
		PRINT_FIELD_U(", ", st, tx_packets);
		PRINT_FIELD_U(", ", st, rx_bytes);
		PRINT_FIELD_U(", ", st, tx_bytes);
		PRINT_FIELD_U(", ", st, rx_errors);
		PRINT_FIELD_U(", ", st, tx_errors);
		PRINT_FIELD_U(", ", st, rx_dropped);
		PRINT_FIELD_U(", ", st, tx_dropped);
		PRINT_FIELD_U(", ", st, multicast);
		PRINT_FIELD_U(", ", st, collisions);

		PRINT_FIELD_U(", ", st, rx_length_errors);
		PRINT_FIELD_U(", ", st, rx_over_errors);
		PRINT_FIELD_U(", ", st, rx_crc_errors);
		PRINT_FIELD_U(", ", st, rx_frame_errors);
		PRINT_FIELD_U(", ", st, rx_fifo_errors);
		PRINT_FIELD_U(", ", st, rx_missed_errors);

		PRINT_FIELD_U(", ", st, tx_aborted_errors);
		PRINT_FIELD_U(", ", st, tx_carrier_errors);
		PRINT_FIELD_U(", ", st, tx_fifo_errors);
		PRINT_FIELD_U(", ", st, tx_heartbeat_errors);
		PRINT_FIELD_U(", ", st, tx_window_errors);

		PRINT_FIELD_U(", ", st, rx_compressed);
		PRINT_FIELD_U(", ", st, tx_compressed);

		if (len >= def_size)
			PRINT_FIELD_U(", ", st, rx_nohandler);
		tprints("}");
	}

	return true;
}

static bool
decode_ifla_bridge_id(struct tcb *const tcp,
		      const kernel_ulong_t addr,
		      const unsigned int len,
		      const void *const opaque_data)
{
	struct {
		uint8_t prio[2];
		uint8_t addr[6];
	} id;

	if (len < sizeof(id))
		return false;
	else if (!umove_or_printaddr(tcp, addr, &id)) {
		tprintf("{prio=[%u, %u]", id.prio[0], id.prio[1]);
		PRINT_FIELD_MAC(", ", id, addr);
		tprints("}");
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
	[IFLA_BRPORT_MESSAGE_AGE_TIMER]		= decode_nla_u64,
	[IFLA_BRPORT_FORWARD_DELAY_TIMER]	= decode_nla_u64,
	[IFLA_BRPORT_HOLD_TIMER]		= decode_nla_u64,
	[IFLA_BRPORT_FLUSH]			= NULL,
	[IFLA_BRPORT_MULTICAST_ROUTER]		= decode_nla_u8,
	[IFLA_BRPORT_PAD]			= NULL,
	[IFLA_BRPORT_MCAST_FLOOD]		= decode_nla_u8,
	[IFLA_BRPORT_MCAST_TO_UCAST]		= decode_nla_u8,
	[IFLA_BRPORT_VLAN_TUNNEL]		= decode_nla_u8,
	[IFLA_BRPORT_BCAST_FLOOD]		= decode_nla_u8,
	[IFLA_BRPORT_GROUP_FWD_MASK]		= decode_nla_u16,
	[IFLA_BRPORT_NEIGH_SUPPRESS]		= decode_nla_u8,
	[IFLA_BRPORT_ISOLATED]			= decode_nla_u8,
	[IFLA_BRPORT_BACKUP_PORT]		= decode_nla_ifindex,
};

static bool
decode_ifla_protinfo(struct tcb *const tcp,
		     const kernel_ulong_t addr,
		     const unsigned int len,
		     const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, rtnl_ifla_brport_attrs,
		      "IFLA_BRPORT_???",
		      ARRSZ_PAIR(ifla_brport_nla_decoders), opaque_data);

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
		PRINT_FIELD_X("{", map, mem_start);
		PRINT_FIELD_X(", ", map, mem_end);
		PRINT_FIELD_X(", ", map, base_addr);
		PRINT_FIELD_U(", ", map, irq);
		PRINT_FIELD_U(", ", map, dma);
		PRINT_FIELD_U(", ", map, port);
		tprints("}");
	}

	return true;
}

bool
decode_nla_linkinfo_kind(struct tcb *const tcp,
			 const kernel_ulong_t addr,
			 const unsigned int len,
			 const void *const opaque_data)
{
	struct ifla_linkinfo_ctx *ctx = (void *) opaque_data;

	memset(ctx->kind, '\0', sizeof(ctx->kind));

	if (umovestr(tcp, addr, sizeof(ctx->kind), ctx->kind) <= 0) {
		/*
		 * If we haven't seen NUL or an error occurred, set kind to
		 * an empty string.
		 */
		ctx->kind[0] = '\0';
	}

	printstr_ex(tcp, addr, len, QUOTE_0_TERMINATED);

	return true;
}

bool
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

	PRINT_FIELD_U("{", st, bus_error);
	PRINT_FIELD_U(", ", st, error_warning);
	PRINT_FIELD_U(", ", st, error_passive);
	PRINT_FIELD_U(", ", st, bus_off);
	PRINT_FIELD_U(", ", st, arbitration_lost);
	PRINT_FIELD_U(", ", st, restarts);
	tprints("}");

	return true;
}

bool
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

static const nla_decoder_t ifla_info_data_bridge_nla_decoders[] = {
	[IFLA_BR_UNSPEC]			= NULL,
	[IFLA_BR_FORWARD_DELAY]			= decode_nla_u32,
	[IFLA_BR_HELLO_TIME]			= decode_nla_u32,
	[IFLA_BR_MAX_AGE]			= decode_nla_u32,
	[IFLA_BR_AGEING_TIME]			= decode_nla_u32,
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
	[IFLA_BR_HELLO_TIMER]			= decode_nla_u64,
	[IFLA_BR_TCN_TIMER]			= decode_nla_u64,
	[IFLA_BR_TOPOLOGY_CHANGE_TIMER]		= decode_nla_u64,
	[IFLA_BR_GC_TIMER]			= decode_nla_u64,
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
	[IFLA_BR_MCAST_LAST_MEMBER_INTVL]	= decode_nla_u64,
	[IFLA_BR_MCAST_MEMBERSHIP_INTVL]	= decode_nla_u64,
	[IFLA_BR_MCAST_QUERIER_INTVL]		= decode_nla_u64,
	[IFLA_BR_MCAST_QUERY_INTVL]		= decode_nla_u64,
	[IFLA_BR_MCAST_QUERY_RESPONSE_INTVL]	= decode_nla_u64,
	[IFLA_BR_MCAST_STARTUP_QUERY_INTVL]	= decode_nla_u64,
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
};

bool
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

bool
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

bool
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

static const nla_decoder_t ifla_linkinfo_nla_decoders[] = {
	[IFLA_INFO_KIND]	= decode_nla_linkinfo_kind,
	[IFLA_INFO_DATA]	= decode_nla_linkinfo_data,
	[IFLA_INFO_XSTATS]	= decode_nla_linkinfo_xstats,
	[IFLA_INFO_SLAVE_KIND]	= decode_nla_str,
	[IFLA_INFO_SLAVE_DATA]	= NULL, /* unimplemented */
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
decode_rtnl_link_stats64(struct tcb *const tcp,
		         const kernel_ulong_t addr,
			 const unsigned int len,
			 const void *const opaque_data)
{
	struct_rtnl_link_stats64 st;
	const unsigned int min_size =
		offsetofend(struct_rtnl_link_stats64, tx_compressed);
	const unsigned int def_size = sizeof(st);
	const unsigned int size =
		(len >= def_size) ? def_size :
				    ((len == min_size) ? min_size : 0);

	if (!size)
		return false;

	if (!umoven_or_printaddr(tcp, addr, size, &st)) {
		PRINT_FIELD_U("{", st, rx_packets);
		PRINT_FIELD_U(", ", st, tx_packets);
		PRINT_FIELD_U(", ", st, rx_bytes);
		PRINT_FIELD_U(", ", st, tx_bytes);
		PRINT_FIELD_U(", ", st, rx_errors);
		PRINT_FIELD_U(", ", st, tx_errors);
		PRINT_FIELD_U(", ", st, rx_dropped);
		PRINT_FIELD_U(", ", st, tx_dropped);
		PRINT_FIELD_U(", ", st, multicast);
		PRINT_FIELD_U(", ", st, collisions);

		PRINT_FIELD_U(", ", st, rx_length_errors);
		PRINT_FIELD_U(", ", st, rx_over_errors);
		PRINT_FIELD_U(", ", st, rx_crc_errors);
		PRINT_FIELD_U(", ", st, rx_frame_errors);
		PRINT_FIELD_U(", ", st, rx_fifo_errors);
		PRINT_FIELD_U(", ", st, rx_missed_errors);

		PRINT_FIELD_U(", ", st, tx_aborted_errors);
		PRINT_FIELD_U(", ", st, tx_carrier_errors);
		PRINT_FIELD_U(", ", st, tx_fifo_errors);
		PRINT_FIELD_U(", ", st, tx_heartbeat_errors);
		PRINT_FIELD_U(", ", st, tx_window_errors);

		PRINT_FIELD_U(", ", st, rx_compressed);
		PRINT_FIELD_U(", ", st, tx_compressed);

		if (len >= def_size)
			PRINT_FIELD_U(", ", st, rx_nohandler);
		tprints("}");
	}

	return true;
}

static bool
decode_ifla_port_vsi(struct tcb *const tcp,
		     const kernel_ulong_t addr,
		     const unsigned int len,
		     const void *const opaque_data)
{
	struct_ifla_port_vsi vsi;

	if (len < sizeof(vsi))
		return false;
	if (umove_or_printaddr(tcp, addr, &vsi))
		return true;

	PRINT_FIELD_U("{", vsi, vsi_mgr_id);
	PRINT_FIELD_STRING(", ", vsi, vsi_type_id,
			   sizeof(vsi.vsi_type_id), QUOTE_FORCE_HEX);
	PRINT_FIELD_U(", ", vsi, vsi_type_version);

	if (!IS_ARRAY_ZERO(vsi.pad))
		PRINT_FIELD_HEX_ARRAY(", ", vsi, pad);

	tprints("}");

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

bool
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
		       tfetch_mem, print_int32_array_member, NULL,
		       PAF_PRINT_INDICES | XLAT_STYLE_FMT_D,
		       inet_devconf_indices, "IPV4_DEVCONF_???");

	return true;
}

static const nla_decoder_t ifla_inet_nla_decoders[] = {
	[IFLA_INET_CONF] = decode_ifla_inet_conf,
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
		       tfetch_mem, print_int32_array_member, NULL,
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
		       tfetch_mem, print_uint64_array_member, NULL,
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
	struct {
		uint32_t max_reasm_len;
		uint32_t tstamp;
		uint32_t reachable_time;
		uint32_t retrans_time;
	} ci;

	if (len < sizeof(ci))
		return false;
	else if (!umove_or_printaddr(tcp, addr, &ci)) {
		PRINT_FIELD_U("{", ci, max_reasm_len);
		PRINT_FIELD_U(", ", ci, tstamp);
		PRINT_FIELD_U(", ", ci, reachable_time);
		PRINT_FIELD_U(", ", ci, retrans_time);
		tprints("}");
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
		       tfetch_mem, print_uint64_array_member, NULL,
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
};

static const struct nla_decoder_table_desc {
	const struct xlat *xlat;
	const char *dflt;
	const nla_decoder_t *table;
	size_t size;
} ifla_af_spec_protos[] = {
	[AF_INET]	= {
		rtnl_ifla_af_spec_inet_attrs, "IFLA_INET_???",
		ARRSZ_PAIR(ifla_inet_nla_decoders),
	},
	[AF_INET6]	= {
		rtnl_ifla_af_spec_inet6_attrs, "IFLA_INET6_???",
		ARRSZ_PAIR(ifla_inet6_nla_decoders),
	},
};

static bool
decode_ifla_af(struct tcb *const tcp,
	       const kernel_ulong_t addr,
	       const unsigned int len,
	       const void *const opaque_data)
{
	uintptr_t proto = (uintptr_t) opaque_data;
	const struct nla_decoder_table_desc *desc
		= proto < ARRAY_SIZE(ifla_af_spec_protos)
			? ifla_af_spec_protos + proto : NULL;

	if (!desc || !desc->table)
		return false;

	decode_nlattr(tcp, addr, len,
		      desc->xlat, desc->dflt, desc->table, desc->size, NULL);

	return true;
}

static bool
decode_ifla_af_spec(struct tcb *const tcp,
		    const kernel_ulong_t addr,
		    const unsigned int len,
		    const void *const opaque_data)
{
	nla_decoder_t af_spec_decoder = &decode_ifla_af;

	decode_nlattr(tcp, addr, len, addrfams, "AF_???",
		      &af_spec_decoder, 0, opaque_data);

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
	[IFLA_VFINFO_LIST]	= NULL, /* unimplemented */
	[IFLA_STATS64]		= decode_rtnl_link_stats64,
	[IFLA_VF_PORTS]		= decode_ifla_vf_ports,
	[IFLA_PORT_SELF]	= decode_ifla_port,
	[IFLA_AF_SPEC]		= decode_ifla_af_spec,
	[IFLA_GROUP]		= decode_nla_u32,
	[IFLA_NET_NS_FD]	= decode_nla_fd,
	[IFLA_EXT_MASK]		= decode_nla_u32,
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
};

DECL_NETLINK_ROUTE_DECODER(decode_ifinfomsg)
{
	struct ifinfomsg ifinfo = { .ifi_family = family };
	size_t offset = sizeof(ifinfo.ifi_family);
	bool decode_nla = false;

	PRINT_FIELD_XVAL("{", ifinfo, ifi_family, addrfams, "AF_???");

	tprints(", ");
	if (len >= sizeof(ifinfo)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(ifinfo) - offset,
					 (char *) &ifinfo + offset)) {
			PRINT_FIELD_XVAL("", ifinfo, ifi_type,
					 arp_hardware_types, "ARPHRD_???");
			PRINT_FIELD_IFINDEX(", ", ifinfo, ifi_index);
			PRINT_FIELD_FLAGS(", ", ifinfo, ifi_flags,
					  iffflags, "IFF_???");
			PRINT_FIELD_X(", ", ifinfo, ifi_change);
			decode_nla = true;
		}
	} else
		tprints("...");
	tprints("}");

	offset = NLMSG_ALIGN(sizeof(ifinfo));
	if (decode_nla && len > offset) {
		tprints(", ");
		decode_nlattr(tcp, addr + offset, len - offset,
			      rtnl_link_attrs, "IFLA_???",
			      ARRSZ_PAIR(ifinfomsg_nla_decoders), &ifinfo);
	}
}
