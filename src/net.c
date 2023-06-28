/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-2000 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 1999-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <netinet/in.h>
#ifdef HAVE_NETINET_TCP_H
# include <netinet/tcp.h>
#endif
#ifdef HAVE_NETINET_UDP_H
# include <netinet/udp.h>
#endif
#ifdef HAVE_NETINET_SCTP_H
# include <netinet/sctp.h>
#endif
#include <arpa/inet.h>
#include <net/if.h>
#include <asm/types.h>

#include <linux/ip_vs.h>
#include "netlink.h"
#if defined(HAVE_LINUX_NETFILTER_ARP_ARP_TABLES_H)
# include <linux/netfilter_arp/arp_tables.h>
#endif
#if defined(HAVE_LINUX_NETFILTER_BRIDGE_EBTABLES_H)
# include <linux/netfilter_bridge/ebtables.h>
#endif
#if defined(HAVE_LINUX_NETFILTER_IPV4_IP_TABLES_H)
# include <linux/netfilter_ipv4/ip_tables.h>
#endif
#if defined(HAVE_LINUX_NETFILTER_IPV6_IP6_TABLES_H)
# include <linux/netfilter_ipv6/ip6_tables.h>
#endif
#include <linux/if_packet.h>
#include <linux/icmp.h>
#include <linux/vm_sockets.h>

#include "xlat/socktypes.h"
#include "xlat/sock_type_flags.h"
#ifndef SOCK_TYPE_MASK
# define SOCK_TYPE_MASK 0xf
#endif

#include "xlat/socketlayers.h"

#include "xlat/inet_protocols.h"

#define XLAT_MACROS_ONLY
#include "xlat/addrfams.h"
#include "xlat/ethernet_protocols.h"
#undef XLAT_MACROS_ONLY
#include "xlat/ax25_protocols.h"
#include "xlat/irda_protocols.h"
#include "xlat/can_protocols.h"
#include "xlat/bt_protocols.h"
#include "xlat/isdn_protocols.h"
#include "xlat/phonet_protocols.h"
#include "xlat/caif_protocols.h"
#include "xlat/nfc_protocols.h"
#include "xlat/kcm_protocols.h"
#include "xlat/smc_protocols.h"

static void
decode_sockbuf(struct tcb *const tcp, const int fd, const kernel_ulong_t addr,
	       const kernel_ulong_t addrlen)
{

	switch (verbose(tcp) ? getfdproto(tcp, fd) : SOCK_PROTO_UNKNOWN) {
	case SOCK_PROTO_NETLINK:
		decode_netlink(tcp, fd, addr, addrlen);
		break;
	default:
		printstrn(tcp, addr, addrlen);
	}
}

/*
 * low bits of the socket type define real socket type,
 * other bits are socket type flags.
 */
static void
tprint_sock_type(unsigned int flags)
{
	const char *str = xlookup(socktypes, flags & SOCK_TYPE_MASK);

	tprint_flags_begin();
	if (str) {
		print_xlat_ex(flags & SOCK_TYPE_MASK, str, XLAT_STYLE_DEFAULT);
		flags &= ~SOCK_TYPE_MASK;
		if (!flags)
			return;
		tprint_flags_or();
	}
	printflags_in(sock_type_flags, flags, "SOCK_???");
	tprint_flags_end();
}

SYS_FUNC(socket)
{
	/* domain */
	printxval(addrfams, tcp->u_arg[0], "AF_???");
	tprint_arg_next();

	/* type */
	tprint_sock_type(tcp->u_arg[1]);
	tprint_arg_next();

	/* protocol */
	switch (tcp->u_arg[0]) {
	case AF_INET:
	case AF_INET6:
		printxval(inet_protocols, tcp->u_arg[2], "IPPROTO_???");
		break;

	case AF_AX25:
		/* Those are not available in public headers.  */
		printxval_ex(ax25_protocols, tcp->u_arg[2], "AX25_P_???",
			     XLAT_STYLE_VERBOSE);
		break;

	case AF_NETLINK:
		printxval(netlink_protocols, tcp->u_arg[2], "NETLINK_???");
		break;

	case AF_PACKET:
		tprints_arg_begin("htons");
		printxval(ethernet_protocols, ntohs(tcp->u_arg[2]),
			  "ETH_P_???");
		tprint_arg_end();
		break;

	case AF_IRDA:
		printxval(can_protocols, tcp->u_arg[2], "IRDAPROTO_???");
		break;

	case AF_CAN:
		printxval(can_protocols, tcp->u_arg[2], "CAN_???");
		break;

	case AF_BLUETOOTH:
		printxval(bt_protocols, tcp->u_arg[2], "BTPROTO_???");
		break;

	case AF_RXRPC:
		printxval(addrfams, tcp->u_arg[2], "AF_???");
		break;

	case AF_ISDN:
		printxval(isdn_protocols, tcp->u_arg[2], "ISDN_P_???");
		break;

	case AF_PHONET:
		printxval(phonet_protocols, tcp->u_arg[2], "PN_PROTO_???");
		break;

	case AF_CAIF:
		printxval(caif_protocols, tcp->u_arg[2], "CAIFPROTO_???");
		break;

	case AF_NFC:
		printxval(nfc_protocols, tcp->u_arg[2], "NFC_SOCKPROTO_???");
		break;

	case AF_KCM:
		printxval(kcm_protocols, tcp->u_arg[2], "KCMPROTO_???");
		break;

	case AF_SMC:
		printxval(smc_protocols, tcp->u_arg[2], "SMCPROTO_???");
		break;

	default:
		PRINT_VAL_U(tcp->u_arg[2]);
		break;
	}

	return RVAL_DECODED | RVAL_FD;
}

static bool
fetch_socklen(struct tcb *const tcp, int *const plen,
	      const kernel_ulong_t sockaddr, const kernel_ulong_t socklen)
{
	return verbose(tcp) && sockaddr && socklen
	       && umove(tcp, socklen, plen) == 0;
}

static int
decode_sockname(struct tcb *tcp)
{
	int ulen, rlen;

	if (entering(tcp)) {
		/* sockfd */
		printfd(tcp, tcp->u_arg[0]);
		tprint_arg_next();

		if (fetch_socklen(tcp, &ulen, tcp->u_arg[1], tcp->u_arg[2])) {
			set_tcb_priv_ulong(tcp, ulen);
			return 0;
		} else {
			/* addr */
			printaddr(tcp->u_arg[1]);
			tprint_arg_next();

			/* addrlen */
			printaddr(tcp->u_arg[2]);

			return RVAL_DECODED;
		}
	}

	ulen = get_tcb_priv_ulong(tcp);

	if (syserror(tcp) || umove(tcp, tcp->u_arg[2], &rlen) < 0) {
		/* addr */
		printaddr(tcp->u_arg[1]);
		tprint_arg_next();

		/* addrlen */
		tprint_indirect_begin();
		PRINT_VAL_D(ulen);
		tprint_indirect_end();
	} else {
		/* addr */
		decode_sockaddr(tcp, tcp->u_arg[1], ulen > rlen ? rlen : ulen);
		tprint_arg_next();

		/* addrlen */
		tprint_indirect_begin();
		if (ulen != rlen) {
			PRINT_VAL_D(ulen);
			tprint_value_changed();
		}
		PRINT_VAL_D(rlen);
		tprint_indirect_end();
	}

	return RVAL_DECODED;
}

SYS_FUNC(accept)
{
	return decode_sockname(tcp) | RVAL_FD;
}

SYS_FUNC(accept4)
{
	int rc = decode_sockname(tcp);

	if (rc & RVAL_DECODED) {
		/* flags */
		tprint_arg_next();
		printflags(sock_type_flags, tcp->u_arg[3], "SOCK_???");
	}

	return rc | RVAL_FD;
}

SYS_FUNC(send)
{
	/* sockfd */
	printfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* buf */
	decode_sockbuf(tcp, tcp->u_arg[0], tcp->u_arg[1], tcp->u_arg[2]);
	tprint_arg_next();

	/* len */
	PRINT_VAL_U(tcp->u_arg[2]);
	tprint_arg_next();

	/* flags */
	printflags(msg_flags, tcp->u_arg[3], "MSG_???");

	return RVAL_DECODED;
}

SYS_FUNC(sendto)
{
	/* sockfd */
	printfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* buf */
	decode_sockbuf(tcp, tcp->u_arg[0], tcp->u_arg[1], tcp->u_arg[2]);
	tprint_arg_next();

	/* len */
	PRINT_VAL_U(tcp->u_arg[2]);
	tprint_arg_next();

	/* flags */
	printflags(msg_flags, tcp->u_arg[3], "MSG_???");
	tprint_arg_next();

	/* dest_addr */
	const int addrlen = tcp->u_arg[5];
	decode_sockaddr(tcp, tcp->u_arg[4], addrlen);
	tprint_arg_next();

	/* addrlen */
	PRINT_VAL_D(addrlen);

	return RVAL_DECODED;
}

SYS_FUNC(recv)
{
	if (entering(tcp)) {
		/* sockfd */
		printfd(tcp, tcp->u_arg[0]);
		tprint_arg_next();
	} else {
		/* buf */
		if (syserror(tcp)) {
			printaddr(tcp->u_arg[1]);
		} else {
			decode_sockbuf(tcp, tcp->u_arg[0], tcp->u_arg[1],
				       MIN((kernel_ulong_t) tcp->u_rval,
					   tcp->u_arg[2]));
		}
		tprint_arg_next();

		/* len */
		PRINT_VAL_U(tcp->u_arg[2]);
		tprint_arg_next();

		/* flags */
		printflags(msg_flags, tcp->u_arg[3], "MSG_???");
	}
	return 0;
}

SYS_FUNC(recvfrom)
{
	int ulen, rlen;

	if (entering(tcp)) {
		/* sockfd */
		printfd(tcp, tcp->u_arg[0]);
		tprint_arg_next();

		if (fetch_socklen(tcp, &ulen, tcp->u_arg[4], tcp->u_arg[5])) {
			set_tcb_priv_ulong(tcp, ulen);
		}
	} else {
		/* buf */
		if (syserror(tcp)) {
			printaddr(tcp->u_arg[1]);
		} else {
			decode_sockbuf(tcp, tcp->u_arg[0], tcp->u_arg[1],
				       MIN((kernel_ulong_t) tcp->u_rval,
					   tcp->u_arg[2]));
		}
		tprint_arg_next();

		/* len */
		PRINT_VAL_U(tcp->u_arg[2]);
		tprint_arg_next();

		/* flags */
		printflags(msg_flags, tcp->u_arg[3], "MSG_???");
		tprint_arg_next();

		ulen = get_tcb_priv_ulong(tcp);

		if (!fetch_socklen(tcp, &rlen, tcp->u_arg[4], tcp->u_arg[5])) {
			/* src_addr */
			printaddr(tcp->u_arg[4]);
			tprint_arg_next();

			/* addrlen */
			printaddr(tcp->u_arg[5]);

			return 0;
		}
		if (syserror(tcp)) {
			/* src_addr */
			printaddr(tcp->u_arg[4]);
			tprint_arg_next();

			/* addrlen */
			tprint_indirect_begin();
			PRINT_VAL_D(ulen);
			tprint_indirect_end();

			return 0;
		}

		/* src_addr */
		decode_sockaddr(tcp, tcp->u_arg[4], ulen > rlen ? rlen : ulen);
		tprint_arg_next();

		/* addrlen */
		tprint_indirect_begin();
		if (ulen != rlen) {
			PRINT_VAL_D(ulen);
			tprint_value_changed();
		}
		PRINT_VAL_D(rlen);
		tprint_indirect_end();
	}
	return 0;
}

SYS_FUNC(getsockname)
{
	return decode_sockname(tcp);
}

static void
decode_pair_fd(struct tcb *const tcp, const kernel_ulong_t addr)
{
	int fd;
	print_array(tcp, addr, 2, &fd, sizeof(fd),
		    tfetch_mem, print_fd_array_member, NULL);
}

static int
do_pipe(struct tcb *tcp, int flags_arg)
{
	if (exiting(tcp)) {
		/* pipefd */
		decode_pair_fd(tcp, tcp->u_arg[0]);
		if (flags_arg >= 0) {
			/* flags */
			tprint_arg_next();
			printflags(open_mode_flags, tcp->u_arg[flags_arg], "O_???");
		}
	}
	return 0;
}

SYS_FUNC(pipe)
{
#if HAVE_ARCH_GETRVAL2
	if (exiting(tcp) && !syserror(tcp)) {
		tprint_array_begin();
		printfd(tcp, tcp->u_rval);
		tprint_array_next();
		printfd(tcp, getrval2(tcp));
		tprint_array_end();
	}
	return 0;
#else
	return do_pipe(tcp, -1);
#endif
}

SYS_FUNC(pipe2)
{
	return do_pipe(tcp, 1);
}

SYS_FUNC(socketpair)
{
	if (entering(tcp)) {
		/* domain */
		printxval(addrfams, tcp->u_arg[0], "AF_???");
		tprint_arg_next();

		/* type */
		tprint_sock_type(tcp->u_arg[1]);
		tprint_arg_next();

		/* protocol */
		PRINT_VAL_U(tcp->u_arg[2]);
		tprint_arg_next();
	} else {
		/* sv */
		decode_pair_fd(tcp, tcp->u_arg[3]);
	}
	return 0;
}

#include "xlat/sock_options.h"
#include "xlat/getsock_options.h"
#include "xlat/setsock_options.h"
#include "xlat/sock_ip_options.h"
#include "xlat/getsock_ip_options.h"
#include "xlat/setsock_ip_options.h"
#include "xlat/sock_vsock_options.h"
#include "xlat/sock_ipv6_options.h"
#include "xlat/getsock_ipv6_options.h"
#include "xlat/setsock_ipv6_options.h"
#include "xlat/sock_ipx_options.h"
#include "xlat/sock_ax25_options.h"
#include "xlat/sock_netlink_options.h"
#include "xlat/sock_packet_options.h"
#include "xlat/sock_raw_options.h"
#include "xlat/sock_sctp_options.h"
#include "xlat/sock_tcp_options.h"
#include "xlat/sock_udp_options.h"
#include "xlat/sock_can_raw_options.h"
#include "xlat/sock_irda_options.h"
#include "xlat/sock_llc_options.h"
#include "xlat/sock_dccp_options.h"
#include "xlat/sock_tipc_options.h"
#include "xlat/sock_rxrpc_options.h"
#include "xlat/sock_pppol2tp_options.h"
#include "xlat/sock_bluetooth_options.h"
#include "xlat/sock_pnp_options.h"
#include "xlat/sock_rds_options.h"
#include "xlat/sock_iucv_options.h"
#include "xlat/sock_caif_options.h"
#include "xlat/sock_alg_options.h"
#include "xlat/sock_nfcllcp_options.h"
#include "xlat/sock_kcm_options.h"
#include "xlat/sock_tls_options.h"
#include "xlat/sock_xdp_options.h"

static void
print_sockopt_fd_level_name(struct tcb *tcp, int fd, unsigned int level,
			    unsigned int name, bool is_getsockopt)
{
	/* sockfd */
	printfd(tcp, fd);
	tprint_arg_next();

	/* level */
	printxval(socketlayers, level, "SOL_??");
	tprint_arg_next();

	/* optname */
	switch (level) {
	case SOL_SOCKET:
		printxvals(name, "SO_???", sock_options,
			   is_getsockopt ? getsock_options :
					   setsock_options, NULL);
		break;
	case SOL_IP:
		printxvals(name, "IP_???", sock_ip_options,
			   is_getsockopt ? getsock_ip_options :
					   setsock_ip_options, NULL);
		break;
	/*
	 * Yes, VMWare in their infinite wisdom have decided to use address
	 * family instead of a socket option layer for the socket option layer
	 * check, see net/vmw_vsock/af_vsock.c:vsock_connectible_[gs]etsockopt.
	 */
	case AF_VSOCK:
		printxval(sock_vsock_options, name, "SO_VM_???");
		break;
	case SOL_IPV6:
		printxvals(name, "IPV6_???", sock_ipv6_options,
			   is_getsockopt ? getsock_ipv6_options :
					   setsock_ipv6_options, NULL);
		break;
	case SOL_IPX:
		printxval(sock_ipx_options, name, "IPX_???");
		break;
	case SOL_AX25:
		printxval(sock_ax25_options, name, "AX25_???");
		break;
	case SOL_PACKET:
		printxval(sock_packet_options, name, "PACKET_???");
		break;
	case SOL_TCP:
		printxval(sock_tcp_options, name, "TCP_???");
		break;
	case SOL_CAN_RAW:
		printxval(sock_can_raw_options, name, "CAN_RAW_???");
		break;
	case SOL_SCTP:
		printxval(sock_sctp_options, name, "SCTP_???");
		break;
	case SOL_RAW:
		printxval(sock_raw_options, name, "RAW_???");
		break;
	case SOL_NETLINK:
		printxval(sock_netlink_options, name, "NETLINK_???");
		break;
	case SOL_UDP:
		printxval(sock_udp_options, name, "UDP_???");
		break;
	case SOL_IRDA:
		printxval(sock_irda_options, name, "IRLMP_???");
		break;
	case SOL_LLC:
		printxval(sock_llc_options, name, "LLC_OPT_???");
		break;
	case SOL_DCCP:
		printxval(sock_dccp_options, name, "DCCP_SOCKOPT_???");
		break;
	case SOL_TIPC:
		printxval(sock_tipc_options, name, "TIPC_???");
		break;
	case SOL_RXRPC:
		printxval(sock_rxrpc_options, name, "RXRPC_???");
		break;
	case SOL_PPPOL2TP:
		printxval(sock_pppol2tp_options, name, "PPPOL2TP_SO_???");
		break;
	case SOL_BLUETOOTH:
		printxval(sock_bluetooth_options, name, "BT_???");
		break;
	case SOL_PNPIPE:
		printxval(sock_pnp_options, name, "PNPIPE_???");
		break;
	case SOL_RDS:
		printxval(sock_rds_options, name, "RDS_???");
		break;
	case SOL_IUCV:
		printxval(sock_iucv_options, name, "SO_???");
		break;
	case SOL_CAIF:
		printxval(sock_caif_options, name, "CAIFSO_???");
		break;
	case SOL_ALG:
		printxval(sock_alg_options, name, "ALG_???");
		break;
	case SOL_NFC:
		printxval(sock_nfcllcp_options, name, "NFC_LLCP_???");
		break;
	case SOL_KCM:
		printxval(sock_kcm_options, name, "KCM_???");
		break;
	case SOL_TLS:
		printxval(sock_tls_options, name, "TLS_???");
		break;
	case SOL_XDP:
		printxval(sock_xdp_options, name, "XDP_???");
		break;

		/* Other SOL_* protocol levels still need work. */

	default:
		PRINT_VAL_U(name);
	}
}

static void
print_get_linger(struct tcb *const tcp, const kernel_ulong_t addr,
		 unsigned int len)
{
	struct linger linger;

	/*
	 * The kernel cannot return len > sizeof(linger) because struct linger
	 * cannot change, but extra safety won't harm either.
	 */
	if (len > sizeof(linger))
		len = sizeof(linger);
	if (umoven_or_printaddr(tcp, addr, len, &linger))
		return;

	MAYBE_PRINT_FIELD_LEN(tprint_struct_begin(),
			      linger, l_onoff, len, PRINT_FIELD_D);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      linger, l_linger, len, PRINT_FIELD_D);
	tprint_struct_end();
}

static void
print_get_ucred(struct tcb *const tcp, const kernel_ulong_t addr,
		unsigned int len)
{
	struct ucred uc;

	/*
	 * The kernel is very unlikely to return len > sizeof(uc)
	 * because struct ucred is very unlikely to change,
	 * but extra safety won't harm either.
	 */
	if (len > sizeof(uc))
		len = sizeof(uc);

	if (umoven_or_printaddr(tcp, addr, len, &uc))
		return;

	MAYBE_PRINT_FIELD_LEN(tprint_struct_begin(),
			      uc, pid, len, PRINT_FIELD_TGID, tcp);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      uc, uid, len, PRINT_FIELD_ID);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      uc, gid, len, PRINT_FIELD_ID);
	tprint_struct_end();
}

static void
print_get_error(struct tcb *const tcp, const kernel_ulong_t addr,
		const unsigned int len)
{
	unsigned int err;

	if (len < sizeof(err)) {
		printstr_ex(tcp, addr, len, QUOTE_FORCE_HEX);
		return;
	}

	if (umove_or_printaddr(tcp, addr, &err))
		return;

	tprint_indirect_begin();
	print_err(err, false);
	tprint_indirect_end();
}

#include "xlat/sockopt_txrehash_vals.h"

static void
print_txrehash(struct tcb *const tcp, const kernel_ulong_t addr, const int len)
{
	int val = 0;

	if (len < (int) sizeof(val)) {
		if (entering(tcp))
			printaddr(addr);
		else
			printstr_ex(tcp, addr, len, QUOTE_FORCE_HEX);
		return;
	}

	if (umove_or_printaddr(tcp, addr, &val))
		return;

	tprint_indirect_begin();
	printxval_d(sockopt_txrehash_vals, val, "SOCK_TXREHASH_???");
	tprint_indirect_end();
}

static void
print_port_range(struct tcb *const tcp, const kernel_ulong_t addr,
		 const unsigned int len)
{
	unsigned int ports;

	if (len != sizeof(ports)) {
		printstr_ex(tcp, addr, len, QUOTE_FORCE_HEX);
		return;
	}

	if (umove_or_printaddr(tcp, addr, &ports))
		return;

	tprint_indirect_begin();
	PRINT_VAL_X(ports);
	if (xlat_verbose(xlat_verbosity) != XLAT_STYLE_RAW) {
		unsigned short lo = ports & 0xffff;
		unsigned short hi = ports >> 16;

		if (ports && (!lo || !hi || lo <= hi))
			tprintf_comment("%.0hu..%.0hu", lo, hi);
	}
	tprint_indirect_end();
}


static void
print_ip_protocol(struct tcb *const tcp, const kernel_ulong_t addr,
		  const unsigned int len)
{
	unsigned int protocol;

	if (len != sizeof(protocol)) {
		printstr_ex(tcp, addr, len, QUOTE_FORCE_HEX);
		return;
	}

	if (umove_or_printaddr(tcp, addr, &protocol))
		return;

	tprint_indirect_begin();
	printxval(inet_protocols, protocol, "IPPROTO_???");
	tprint_indirect_end();
}


static void
print_tpacket_stats(struct tcb *const tcp, const kernel_ulong_t addr,
		    unsigned int len)
{
	struct tp_stats {
		unsigned int tp_packets, tp_drops, tp_freeze_q_cnt;
	} stats;

	/*
	 * The kernel may return len > sizeof(stats) if the kernel structure
	 * grew as it happened when tpacket_stats_v3 was introduced.
	 */
	if (len > sizeof(stats))
		len = sizeof(stats);

	if (umoven_or_printaddr(tcp, addr, len, &stats))
		return;

	MAYBE_PRINT_FIELD_LEN(tprint_struct_begin(),
			      stats, tp_packets, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      stats, tp_drops, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      stats, tp_freeze_q_cnt, len, PRINT_FIELD_U);
	tprint_struct_end();
}

#include "xlat/icmp_filter_flags.h"

static void
print_icmp_filter(struct tcb *const tcp, const kernel_ulong_t addr, int len)
{
	struct icmp_filter filter = { ~0U };

	if (len > (int) sizeof(filter))
		len = sizeof(filter);
	else if (len <= 0) {
		printaddr(addr);
		return;
	}

	if (umoven_or_printaddr(tcp, addr, len, &filter))
		return;

	uint32_t data32 = filter.data;
	static_assert(sizeof(filter.data) == sizeof(data32),
		      "struct icmp_filter.data is not 32-bit long");

	/* check whether more than half of the bits are set */
	if (popcount32(&data32, 1) > sizeof(data32) * 8 / 2) {
		/* show those bits that are NOT in the set */
		data32 = ~data32;
		tprints_string("~");
	}

	/* next_set_bit operates on current_wordsize words */
	unsigned long data;
	void *p;
	if (current_wordsize > sizeof(data32)) {
		data = data32;
		p = &data;
	} else {
		p = &data32;
	}

	tprint_bitset_begin();
	bool next = false;
	for (int i = 0;; ++i) {
		i = next_set_bit(p, i, sizeof(data32) * 8);
		if (i < 0)
			break;
		if (next)
			tprint_bitset_next();
		else
			next = true;
		printxval(icmp_filter_flags, i, "ICMP_???");
	}
	tprint_bitset_end();
}

static void
print_getsockopt(struct tcb *const tcp, const unsigned int level,
		 const unsigned int name, const kernel_ulong_t addr,
		 const int ulen, const int rlen)
{
	if (ulen <= 0 || rlen <= 0) {
		/*
		 * As the kernel neither accepts nor returns a negative
		 * length in case of successful getsockopt syscall
		 * invocation, negative values must have been forged
		 * by userspace.
		 */
		printaddr(addr);
		return;
	}

	if (addr && verbose(tcp))
	switch (level) {
	case SOL_SOCKET:
		switch (name) {
		case SO_LINGER:
			print_get_linger(tcp, addr, rlen);
			return;
		case SO_PEERCRED:
			print_get_ucred(tcp, addr, rlen);
			return;
		case SO_ATTACH_FILTER:
			/*
			 * The length returned by the kernel in case of
			 * successful getsockopt syscall invocation is struct
			 * sock_fprog.len that has type unsigned short,
			 * anything else must have been forged by userspace.
			 */
			if ((unsigned short) rlen == (unsigned int) rlen)
				print_sock_fprog(tcp, addr, rlen);
			else
				printaddr(addr);
			return;
		case SO_ERROR:
			print_get_error(tcp, addr, rlen);
			return;
		case SO_TXREHASH:
			print_txrehash(tcp, addr, rlen);
			return;

		/* All known int-like options */
		case SO_DEBUG:
		case SO_REUSEADDR:
		case SO_DONTROUTE:
		case SO_BROADCAST:
		case SO_SNDBUF:
		case SO_RCVBUF:
		case SO_KEEPALIVE:
		case SO_OOBINLINE:
		case SO_NO_CHECK:
		case SO_PRIORITY:
		case SO_BSDCOMPAT:
		case SO_REUSEPORT:
		case SO_PASSCRED:
		case SO_RCVLOWAT:
		case SO_SNDLOWAT:
		case SO_DETACH_FILTER:
		case SO_TIMESTAMP_OLD:
		case SO_ACCEPTCONN:
		case SO_SNDBUFFORCE:
		case SO_RCVBUFFORCE:
		case SO_PASSSEC:
		case SO_TIMESTAMPNS_OLD:
		case SO_MARK:
		case SO_TIMESTAMPING_OLD:
		case SO_RXQ_OVFL:
		case SO_WIFI_STATUS:
		case SO_PEEK_OFF:
		case SO_NOFCS:
		case SO_LOCK_FILTER:
		case SO_SELECT_ERR_QUEUE:
		case SO_BUSY_POLL:
		case SO_INCOMING_CPU:
		case SO_CNX_ADVICE:
		case SO_INCOMING_NAPI_ID:
		case SO_ZEROCOPY:
		case SO_TIMESTAMP_NEW:
		case SO_TIMESTAMPNS_NEW:
		case SO_TIMESTAMPING_NEW:
		case SO_DETACH_REUSEPORT_BPF:
		case SO_PREFER_BUSY_POLL:
		case SO_BUSY_POLL_BUDGET:
		case SO_RESERVE_MEM:
		case SO_RCVMARK:
			if (rlen >= (int) sizeof(int))
				printnum_int(tcp, addr, "%d");
			else
				printstr_ex(tcp, addr, rlen, QUOTE_FORCE_HEX);
			return;
		}
		break;

	case SOL_IP:
		switch (name) {
		case IP_LOCAL_PORT_RANGE:
			print_port_range(tcp, addr, rlen);
			return;
		case IP_PROTOCOL:
			print_ip_protocol(tcp, addr, rlen);
			return;
		}
		break;

	case SOL_PACKET:
		switch (name) {
		case PACKET_STATISTICS:
			print_tpacket_stats(tcp, addr, rlen);
			return;
		}
		break;

	case SOL_RAW:
		switch (name) {
		case ICMP_FILTER:
			print_icmp_filter(tcp, addr, rlen);
			return;
		}
		break;

	case SOL_NETLINK:
		switch (name) {
		case NETLINK_LIST_MEMBERSHIPS: {
			uint32_t buf;
			print_array(tcp, addr, MIN(ulen, rlen) / sizeof(buf),
				    &buf, sizeof(buf),
				    tfetch_mem, print_uint_array_member, 0);
			break;
			}
		default:
			printnum_int(tcp, addr, "%d");
			break;
		}
		return;
	}

	/* default arg printing */

	if (verbose(tcp)) {
		if (rlen == sizeof(int)) {
			printnum_int(tcp, addr, "%d");
		} else {
			printstrn(tcp, addr, rlen);
		}
	} else {
		printaddr(addr);
	}
}

SYS_FUNC(getsockopt)
{
	int ulen, rlen;

	if (entering(tcp)) {
		print_sockopt_fd_level_name(tcp, tcp->u_arg[0],
					    tcp->u_arg[1], tcp->u_arg[2], true);
		tprint_arg_next();

		if (verbose(tcp) && tcp->u_arg[4]
		    && umove(tcp, tcp->u_arg[4], &ulen) == 0) {
			set_tcb_priv_ulong(tcp, ulen);
			return 0;
		} else {
			/* optval */
			printaddr(tcp->u_arg[3]);
			tprint_arg_next();

			/* optlen */
			printaddr(tcp->u_arg[4]);
			return RVAL_DECODED;
		}
	} else {
		ulen = get_tcb_priv_ulong(tcp);

		if (umove(tcp, tcp->u_arg[4], &rlen) < 0) {
			/* optval */
			printaddr(tcp->u_arg[3]);
			tprint_arg_next();

			/* optlen */
			tprint_indirect_begin();
			PRINT_VAL_D(ulen);
			tprint_indirect_end();
		} else if (syserror(tcp)) {
			/* optval */
			printaddr(tcp->u_arg[3]);
			tprint_arg_next();

			/* optlen */
			tprint_indirect_begin();
			if (ulen != rlen) {
				PRINT_VAL_D(ulen);
				tprint_value_changed();
			}
			PRINT_VAL_D(rlen);
			tprint_indirect_end();
		} else {
			/* optval */
			print_getsockopt(tcp, tcp->u_arg[1], tcp->u_arg[2],
					 tcp->u_arg[3], ulen, rlen);
			tprint_arg_next();

			/* optlen */
			tprint_indirect_begin();
			if (ulen != rlen) {
				PRINT_VAL_D(ulen);
				tprint_value_changed();
			}
			PRINT_VAL_D(rlen);
			tprint_indirect_end();
		}
	}
	return 0;
}

static void
print_set_linger(struct tcb *const tcp, const kernel_ulong_t addr,
		 const int len)
{
	struct linger linger;

	if (len < (int) sizeof(linger)) {
		printaddr(addr);
	} else if (!umove_or_printaddr(tcp, addr, &linger)) {
		tprint_struct_begin();
		PRINT_FIELD_D(linger, l_onoff);
		tprint_struct_next();
		PRINT_FIELD_D(linger, l_linger);
		tprint_struct_end();
	}
}

static void
print_mreq(struct tcb *const tcp, const kernel_ulong_t addr,
	   const int len)
{
	struct ip_mreq mreq;

	if (len < (int) sizeof(mreq)) {
		printaddr(addr);
	} else if (!umove_or_printaddr(tcp, addr, &mreq)) {
		tprint_struct_begin();
		PRINT_FIELD_INET_ADDR(mreq, imr_multiaddr, AF_INET);
		tprint_struct_next();
		PRINT_FIELD_INET_ADDR(mreq, imr_interface, AF_INET);
		tprint_struct_end();
	}
}

static void
print_mreq6(struct tcb *const tcp, const kernel_ulong_t addr,
	    const int len)
{
	struct ipv6_mreq mreq;

	if (len < (int) sizeof(mreq)) {
		printaddr(addr);
	} else if (!umove_or_printaddr(tcp, addr, &mreq)) {
		tprint_struct_begin();
		PRINT_FIELD_INET_ADDR(mreq, ipv6mr_multiaddr, AF_INET6);
		tprint_struct_next();
		PRINT_FIELD_IFINDEX(mreq, ipv6mr_interface);
		tprint_struct_end();
	}
}

static void
print_tpacket_req(struct tcb *const tcp, const kernel_ulong_t addr, const int len)
{
	struct tpacket_req req;

	if (len != sizeof(req) ||
	    umove(tcp, addr, &req) < 0) {
		printaddr(addr);
	} else {
		tprint_struct_begin();
		PRINT_FIELD_U(req, tp_block_size);
		tprint_struct_next();
		PRINT_FIELD_U(req, tp_block_nr);
		tprint_struct_next();
		PRINT_FIELD_U(req, tp_frame_size);
		tprint_struct_next();
		PRINT_FIELD_U(req, tp_frame_nr);
		tprint_struct_end();
	}
}

#include "xlat/packet_mreq_type.h"

static void
print_packet_mreq(struct tcb *const tcp, const kernel_ulong_t addr, const int len)
{
	struct packet_mreq mreq;

	if (len != sizeof(mreq) ||
	    umove(tcp, addr, &mreq) < 0) {
		printaddr(addr);
	} else {
		tprint_struct_begin();
		PRINT_FIELD_IFINDEX(mreq, mr_ifindex);
		tprint_struct_next();
		PRINT_FIELD_XVAL(mreq, mr_type, packet_mreq_type,
				 "PACKET_MR_???");
		tprint_struct_next();
		PRINT_FIELD_U(mreq, mr_alen);
		tprint_struct_next();
		PRINT_FIELD_MAC_SZ(mreq, mr_address, mreq.mr_alen);
		tprint_struct_end();
	}
}

static void
print_setsockopt(struct tcb *const tcp, const unsigned int level,
		 const unsigned int name, const kernel_ulong_t addr,
		 const int len)
{
	if (addr && verbose(tcp))
	switch (level) {
	case SOL_SOCKET:
		switch (name) {
		case SO_LINGER:
			print_set_linger(tcp, addr, len);
			return;
		case SO_ATTACH_FILTER:
		case SO_ATTACH_REUSEPORT_CBPF:
			if ((unsigned int) len == get_sock_fprog_size())
				decode_sock_fprog(tcp, addr);
			else
				printaddr(addr);
			return;
		case SO_TXREHASH:
			print_txrehash(tcp, addr, len);
			return;

		/* All known int-like options */
		case SO_DEBUG:
		case SO_REUSEADDR:
		case SO_DONTROUTE:
		case SO_BROADCAST:
		case SO_SNDBUF:
		case SO_RCVBUF:
		case SO_KEEPALIVE:
		case SO_OOBINLINE:
		case SO_NO_CHECK:
		case SO_PRIORITY:
		case SO_BSDCOMPAT:
		case SO_REUSEPORT:
		case SO_PASSCRED:
		case SO_RCVLOWAT:
		case SO_SNDLOWAT:
		case SO_DETACH_FILTER:
		case SO_TIMESTAMP_OLD:
		case SO_ACCEPTCONN:
		case SO_SNDBUFFORCE:
		case SO_RCVBUFFORCE:
		case SO_PASSSEC:
		case SO_TIMESTAMPNS_OLD:
		case SO_MARK:
		case SO_TIMESTAMPING_OLD:
		case SO_RXQ_OVFL:
		case SO_WIFI_STATUS:
		case SO_PEEK_OFF:
		case SO_NOFCS:
		case SO_LOCK_FILTER:
		case SO_SELECT_ERR_QUEUE:
		case SO_BUSY_POLL:
		case SO_INCOMING_CPU:
		case SO_CNX_ADVICE:
		case SO_INCOMING_NAPI_ID:
		case SO_ZEROCOPY:
		case SO_TIMESTAMP_NEW:
		case SO_TIMESTAMPNS_NEW:
		case SO_TIMESTAMPING_NEW:
		case SO_DETACH_REUSEPORT_BPF:
		case SO_PREFER_BUSY_POLL:
		case SO_BUSY_POLL_BUDGET:
		case SO_RESERVE_MEM:
		case SO_RCVMARK:
			if (len < (int) sizeof(int))
				printaddr(addr);
			else
				printnum_int(tcp, addr, "%d");
			return;
		}
		break;

	case SOL_IP:
		switch (name) {
		case IP_ADD_MEMBERSHIP:
		case IP_DROP_MEMBERSHIP:
			print_mreq(tcp, addr, len);
			return;
		case MCAST_JOIN_GROUP:
		case MCAST_LEAVE_GROUP:
			print_group_req(tcp, addr, len);
			return;
		case IP_LOCAL_PORT_RANGE:
			print_port_range(tcp, addr, len);
			return;
		}
		break;

	case SOL_IPV6:
		switch (name) {
		case IPV6_ADD_MEMBERSHIP:
		case IPV6_DROP_MEMBERSHIP:
		case IPV6_JOIN_ANYCAST:
		case IPV6_LEAVE_ANYCAST:
			print_mreq6(tcp, addr, len);
			return;
		case MCAST_JOIN_GROUP:
		case MCAST_LEAVE_GROUP:
			print_group_req(tcp, addr, len);
			return;
		}
		break;

	case SOL_PACKET:
		switch (name) {
		case PACKET_RX_RING:
		case PACKET_TX_RING:
			print_tpacket_req(tcp, addr, len);
			return;
		case PACKET_ADD_MEMBERSHIP:
		case PACKET_DROP_MEMBERSHIP:
			print_packet_mreq(tcp, addr, len);
			return;
		}
		break;

	case SOL_RAW:
		switch (name) {
		case ICMP_FILTER:
			print_icmp_filter(tcp, addr, len);
			return;
		}
		break;

	case SOL_NETLINK:
		if (len < (int) sizeof(int))
			printaddr(addr);
		else
			printnum_int(tcp, addr, "%d");
		return;
	}

	/* default arg printing */

	if (verbose(tcp)) {
		if (len == sizeof(int)) {
			printnum_int(tcp, addr, "%d");
		} else {
			printstrn(tcp, addr, len);
		}
	} else {
		printaddr(addr);
	}
}

SYS_FUNC(setsockopt)
{
	print_sockopt_fd_level_name(tcp, tcp->u_arg[0],
				    tcp->u_arg[1], tcp->u_arg[2], false);
	tprint_arg_next();

	/* optval */
	print_setsockopt(tcp, tcp->u_arg[1], tcp->u_arg[2],
			 tcp->u_arg[3], tcp->u_arg[4]);
	tprint_arg_next();

	/* optlen */
	PRINT_VAL_D((int) tcp->u_arg[4]);

	return RVAL_DECODED;
}
