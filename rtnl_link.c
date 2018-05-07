/*
 * Copyright (c) 2016 Fabien Siron <fabien.siron@epita.fr>
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "defs.h"
#include "netlink_route.h"
#include "nlattr.h"
#include "print_fields.h"

#include "netlink.h"
#ifdef HAVE_LINUX_IF_LINK_H
# include <linux/if_link.h>
#endif
#include <linux/rtnetlink.h>

#include "xlat/rtnl_ifla_brport_attrs.h"
#include "xlat/rtnl_ifla_events.h"
#include "xlat/rtnl_ifla_info_attrs.h"
#include "xlat/rtnl_ifla_port_attrs.h"
#include "xlat/rtnl_ifla_vf_port_attrs.h"
#include "xlat/rtnl_ifla_xdp_attrs.h"
#include "xlat/rtnl_link_attrs.h"
#include "xlat/xdp_flags.h"

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
#ifdef HAVE_STRUCT_RTNL_LINK_STATS_RX_NOHANDLER
		if (len >= def_size)
			PRINT_FIELD_U(", ", st, rx_nohandler);
#endif
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
#ifdef HAVE_STRUCT_IFLA_BRIDGE_ID
	struct ifla_bridge_id id;

	if (len < sizeof(id))
		return false;
	else if (!umove_or_printaddr(tcp, addr, &id)) {
		tprintf("{prio=[%u, %u], addr=%02x:%02x:%02x:%02x:%02x:%02x}",
			id.prio[0], id.prio[1],
			id.addr[0], id.addr[1], id.addr[2],
			id.addr[3], id.addr[4], id.addr[5]);
	}

	return true;
#else
	return false;
#endif
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
	[IFLA_BRPORT_BCAST_FLOOD]		= decode_nla_u8
};

static bool
decode_ifla_protinfo(struct tcb *const tcp,
		     const kernel_ulong_t addr,
		     const unsigned int len,
		     const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, rtnl_ifla_brport_attrs,
		      "IFLA_BRPORT_???", ifla_brport_nla_decoders,
		      ARRAY_SIZE(ifla_brport_nla_decoders), opaque_data);

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

static const nla_decoder_t ifla_linkinfo_nla_decoders[] = {
	[IFLA_INFO_KIND]	= decode_nla_str,
	[IFLA_INFO_DATA]	= NULL, /* unimplemented */
	[IFLA_INFO_XSTATS]	= NULL, /* unimplemented */
	[IFLA_INFO_SLAVE_KIND]	= decode_nla_str,
	[IFLA_INFO_SLAVE_DATA]	= NULL, /* unimplemented */
};

static bool
decode_ifla_linkinfo(struct tcb *const tcp,
		     const kernel_ulong_t addr,
		     const unsigned int len,
		     const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, rtnl_ifla_info_attrs,
		      "IFLA_INFO_???", ifla_linkinfo_nla_decoders,
		      ARRAY_SIZE(ifla_linkinfo_nla_decoders), opaque_data);

	return true;
}

static bool
decode_rtnl_link_stats64(struct tcb *const tcp,
		         const kernel_ulong_t addr,
			 const unsigned int len,
			 const void *const opaque_data)
{
#ifdef HAVE_STRUCT_RTNL_LINK_STATS64
	struct rtnl_link_stats64 st;
	const unsigned int min_size =
		offsetofend(struct rtnl_link_stats64, tx_compressed);
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
#ifdef HAVE_STRUCT_RTNL_LINK_STATS64_RX_NOHANDLER
		if (len >= def_size)
			PRINT_FIELD_U(", ", st, rx_nohandler);
#endif
		tprints("}");
	}

	return true;
#else
	return false;
#endif
}

static bool
decode_ifla_port_vsi(struct tcb *const tcp,
		     const kernel_ulong_t addr,
		     const unsigned int len,
		     const void *const opaque_data)
{
#ifdef HAVE_STRUCT_IFLA_PORT_VSI
	struct ifla_port_vsi vsi;

	if (len < sizeof(vsi))
		return false;
	else if (!umove_or_printaddr(tcp, addr, &vsi)) {
		PRINT_FIELD_U("{", vsi, vsi_mgr_id);
		PRINT_FIELD_STRING(", ", vsi, vsi_type_id,
				   sizeof(vsi.vsi_type_id), QUOTE_FORCE_HEX);
		PRINT_FIELD_U(", ", vsi, vsi_type_version);
		tprints("}");
	}

	return true;
#else
	return false;
#endif
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
		      "IFLA_VF_PORT_???", ifla_port_nla_decoders,
		      ARRAY_SIZE(ifla_port_nla_decoders), opaque_data);

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
		      "IFLA_VF_PORT_???", ifla_vf_port_nla_decoders,
		      ARRAY_SIZE(ifla_vf_port_nla_decoders), opaque_data);

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

static const nla_decoder_t ifla_xdp_nla_decoders[] = {
	[IFLA_XDP_FD]		= decode_nla_fd,
	[IFLA_XDP_ATTACHED]	= decode_nla_u8,
	[IFLA_XDP_FLAGS]	= decode_ifla_xdp_flags,
	[IFLA_XDP_PROG_ID]	= decode_nla_u32
};

static bool
decode_ifla_xdp(struct tcb *const tcp,
		const kernel_ulong_t addr,
		const unsigned int len,
		const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, rtnl_ifla_xdp_attrs,
		      "IFLA_XDP_???", ifla_xdp_nla_decoders,
		      ARRAY_SIZE(ifla_xdp_nla_decoders), opaque_data);

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

static const nla_decoder_t ifinfomsg_nla_decoders[] = {
	[IFLA_ADDRESS]		= NULL, /* unimplemented */
	[IFLA_BROADCAST]	= NULL, /* unimplemented */
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
	[IFLA_AF_SPEC]		= NULL, /* unimplemented */
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
			      ifinfomsg_nla_decoders,
			      ARRAY_SIZE(ifinfomsg_nla_decoders), NULL);
	}
}
