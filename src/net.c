/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-2000 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 1999-2021 The strace developers.
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

#include <linux/sock_diag.h>
#include <linux/inet_diag.h>

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

#include "xlat/sockopt_tcpct_flags.h"
#include "xlat/tcp_ca_states.h"
#include "xlat/tcp_info_options.h"
#include "xlat/tcp_repair_vals.h"
#include "xlat/tcp_zerocopy_flags.h"
#include "xlat/tcp_zerocopy_msg_flags.h"


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

	if (str) {
		print_xlat_ex(flags & SOCK_TYPE_MASK, str, XLAT_STYLE_DEFAULT);
		flags &= ~SOCK_TYPE_MASK;
		if (!flags)
			return;
		tprints("|");
	}
	printflags(sock_type_flags, flags, "SOCK_???");
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
#include "xlat/sock_mptcp_options.h"

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
	case SOL_MPTCP:
		printxval(sock_xdp_options, name, "MPTCP_???");
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

	if (umoven_or_printaddr(tcp, addr, MIN(len, sizeof(linger)), &linger))
		return;

	MAYBE_PRINT_FIELD_LEN(tprint_struct_begin(),
			      linger, l_onoff, len, PRINT_FIELD_D);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      linger, l_linger, len, PRINT_FIELD_D);
	if (len > sizeof(linger)) {
		print_nonzero_bytes(tcp, tprint_struct_next, addr,
				    sizeof(linger), MIN(linger, get_pagesize()),
				    QUOTE_FORCE_HEX);
	}
	tprint_struct_end();
}

static void
print_get_ucred(struct tcb *const tcp, const kernel_ulong_t addr,
		unsigned int len)
{
	struct ucred uc;

	if (umoven_or_printaddr(tcp, addr, MIN(len, sizeof(uc)), &uc))
		return;

	MAYBE_PRINT_FIELD_LEN(tprint_struct_begin(),
			      uc, pid, len, PRINT_FIELD_TGID, tcp);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      uc, uid, len, PRINT_FIELD_ID);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      uc, gid, len, PRINT_FIELD_ID);
	if (len > sizeof(uc)) {
		print_nonzero_bytes(tcp, tprint_struct_next, addr, sizeof(uc),
				    MIN(uc, get_pagesize()), QUOTE_FORCE_HEX);
	}
	tprint_struct_end();
}

static void
print_get_error(struct tcb *const tcp, const kernel_ulong_t addr,
		unsigned int len)
{
	unsigned int err;

	if (len > sizeof(err))
		len = sizeof(err);

	if (umoven_or_printaddr(tcp, addr, len, &err))
		return;

	tprint_indirect_begin();
	print_err(err, false);
	tprint_indirect_end();
}

static void
print_tcp_info(struct tcb *const tcp, const kernel_ulong_t addr,
	       unsigned int len)
{
	struct tcp_info {
		uint8_t  tcpi_state;
		uint8_t  tcpi_ca_state;
		uint8_t  tcpi_retransmits;
		uint8_t  tcpi_probes;
		uint8_t  tcpi_backoff;
		uint8_t  tcpi_options;
		uint8_t  tcpi_snd_wscale : 4,
		         tcpi_rcv_wscale : 4;
		uint8_t  tcpi_delivery_rate_app_limited:1,
		         tcpi_fastopen_client_fail:2
			 __unused:5;

		uint32_t tcpi_rto;
		uint32_t tcpi_ato;
		uint32_t tcpi_snd_mss;
		uint32_t tcpi_rcv_mss;

		uint32_t tcpi_unacked;
		uint32_t tcpi_sacked;
		uint32_t tcpi_lost;
		uint32_t tcpi_retrans;
		uint32_t tcpi_fackets;

		uint32_t tcpi_last_data_sent;
		uint32_t tcpi_last_ack_sent;
		uint32_t tcpi_last_data_recv;
		uint32_t tcpi_last_ack_recv;

		uint32_t tcpi_pmtu;
		uint32_t tcpi_rcv_ssthresh;
		uint32_t tcpi_rtt;
		uint32_t tcpi_rttvar;
		uint32_t tcpi_snd_ssthresh;
		uint32_t tcpi_snd_cwnd;
		uint32_t tcpi_advmss;
		uint32_t tcpi_reordering;

		uint32_t tcpi_rcv_rtt;
		uint32_t tcpi_rcv_space;

		uint32_t tcpi_total_retrans;

		uint64_t tcpi_pacing_rate;
		uint64_t tcpi_max_pacing_rate;
		uint64_t tcpi_bytes_acked;
		uint64_t tcpi_bytes_received;
		uint32_t tcpi_segs_out;
		uint32_t tcpi_segs_in;

		uint32_t tcpi_notsent_bytes;
		uint32_t tcpi_min_rtt;
		uint32_t tcpi_data_segs_in;
		uint32_t tcpi_data_segs_out;

		uint64_t tcpi_delivery_rate;

		uint64_t tcpi_busy_time;
		uint64_t tcpi_rwnd_limited;
		uint64_t tcpi_sndbuf_limited;

		uint32_t tcpi_delivered;
		uint32_t tcpi_delivered_ce;

		uint64_t tcpi_bytes_sent;
		uint64_t tcpi_bytes_retrans;
		uint32_t tcpi_dsack_dups;
		uint32_t tcpi_reord_seen;

		uint32_t tcpi_rcv_ooopack;

		uint32_t tcpi_snd_wnd;
	} ti;

	if (umoven_or_printaddr(tcp, addr, MIN(len, sizeof(ti)), &ti))
		return;

	MAYBE_PRINT_FIELD_LEN(tprint_struct_begin(),
			      ti, tcpi_state, len, PRINT_FIELD_XVAL,
			      tcp_states, "TCP_???");
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_ca_state, len, PRINT_FIELD_XVAL,
			      tcp_ca_states, "TCP_CA_???");
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_retransmits, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_probes, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_backoff, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_options, len, PRINT_FIELD_FLAGS,
			      tcp_ca_states, "TCPI_OPT_???");
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_snd_wscale, len, PRINT_FIELD_U);

	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_rcv_wscale, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_rcv_wscale, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_delivery_rate_app_limited, len,
			      PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_fastopen_client_fail, len,
			      PRINT_FIELD_U);
	if (ti.__unused)
		tprintf_commend("bits 3..7: %#x", ti.__unused);

	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_rto, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_ato, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_snd_mss, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_rcv_mss, len, PRINT_FIELD_U);

	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_unacked, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_sacked, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_lost, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_retrans, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_fackets, len, PRINT_FIELD_U);

	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_last_data_sent, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_last_ack_sent, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_last_data_recv, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_last_ack_recv, len, PRINT_FIELD_U);

	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_pmtu, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_rcv_ssthresh, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_rtt, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_rttvar, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_snd_ssthresh, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_snd_cwnd, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_advmss, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_reordering, len, PRINT_FIELD_U);

	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_rcv_rtt, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_rcv_space, len, PRINT_FIELD_U);

	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_total_retrans, len, PRINT_FIELD_U);

	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_pacing_rate, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_max_pacing_rate, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_bytes_acked, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_bytes_received, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_segs_out, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_segs_in, len, PRINT_FIELD_U);

	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_notsent_bytes, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_min_rtt, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_data_segs_in, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_data_segs_out, len, PRINT_FIELD_U);

	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_delivery_rate, len, PRINT_FIELD_U);

	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_busy_time, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_rwnd_limited, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_sndbuf_limited, len, PRINT_FIELD_U);

	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_delivered, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_delivered_ce, len, PRINT_FIELD_U);

	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_bytes_sent, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_bytes_retrans, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_dsack_dups, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_reord_seen, len, PRINT_FIELD_U);

	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_rcv_ooopack, len, PRINT_FIELD_U);

	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      ti, tcpi_snd_wnd, len, PRINT_FIELD_U);

	if (len > sizeof(ti)) {
		print_nonzero_bytes(tcp, tprint_struct_next, addr, sizeof(ti),
				    MIN(len, get_pagesize()), QUOTE_FORCE_HEX);
	}

	tprint_struct_end();
}

static void
print_tcpct(struct tcb *const tcp, const kernel_ulong_t addr, unsigned int len)
{
	struct tcp_cookie_transactions {
		uint16_t tcpct_flags;                    /* see above */
		uint8_t  __tcpct_pad1;                   /* zero */
		uint8_t  tcpct_cookie_desired;           /* bytes */
		uint16_t tcpct_s_data_desired;           /* bytes of variable data */
		uint16_t tcpct_used;                     /* bytes in value */
		uint8_t  tcpct_value[536 /* TCP_MSS_DEFAULT */];
	} tct;

	/*
	 * [gs]etsockopt(TCP_COOKIE_TRANSACTIONS) returned -EINVAL
	 * if optlen < sizeof(struct tcp_cookie_transactions), so we can
	 * opt out of trying to decode partial structure as well
	 * (that also means no need for MAYBE_PRINT_FIELD_LEN).
	 */
	if (len < sizeof(tct)) {
		printaddr(addr);
		return;
	}

	if (umove_or_printaddr(tcp, addr, &tct))
		return;

	tprint_struct_begin();
	PRINT_FIELD_FLAGS_VERBOSE(tct, tcpct_flags, sockopt_tcpct_flags,
				  "TCP_COOKIE_???");
	if (tct.__tcpct_pad1) {
		tprint_struct_next();
		PRINT_FIELD_X(tct, __tcpct_pad1);
	}
	tprint_struct_next();
	PRINT_FIELD_U(tct, tcpct_cookie_desired);
	tprint_struct_next();
	PRINT_FIELD_U(tct, tcpct_s_data_desired);
	tprint_struct_next();
	PRINT_FIELD_U(tct, tcpct_used);
	tprint_struct_next();
	PRINT_FIELD_HEX_ARRAY_UPTO(tct, tcpct_value,
				   MIN(tct.tcpct_used, sizeof(tcpct_value)));
	if (len > sizeof(tct)) {
		print_nonzero_bytes(tcp, tprint_struct_next, addr, sizeof(tct),
				    MIN(len, get_pagesize()), QUOTE_FORCE_HEX);
	}
	tprint_struct_end();
}

static void
print_tcp_repair(struct tcb *const tcp, const kernel_ulong_t addr,
		 unsigned int len)
{
	int repair;

	if (len > sizeof(repair))
		len = sizeof(repair);

	if (umoven_or_printaddr(tcp, addr, len, &err))
		return;

	tprint_indirect_begin();
	printxval_d(tcp_repair_vals, repair, "TCP_REPAIR_???");
	tprint_indirect_end();
}

static void
print_tcp_cc_info(struct tcb *const tcp, const kernel_ulong_t addr,
		  unsigned int len)
{
	union tcp_cc_info tci;

	if (len > sizeof(ti))
		len = sizeof(ti);

	if (umoven_or_printaddr(tcp, addr, len, &ti))
		return;

	/*
	 * We have no idea what is the actual congestion algorithm used so far
	 * (TODO: it is possible to get it via inet_diag
	 * (idiag_ext |= 1<<INET_DIAG_CONG should do the trick), but the current
	 * API is unsuitable for performing such a request, and also we need
	 * to pass over socket fd through here), so we decode all the union
	 * members.
	 */

	tprint_union_begin();
	PRINT_FIELD_OBJ_TCB_PTR(tci, vegas, tcp, print_tcpvegas_info, len);
	tprint_union_next();
	PRINT_FIELD_OBJ_TCB_PTR(tci, dctcp, tcp, print_tcp_dctcp_info, len);
	tprint_union_next();
	PRINT_FIELD_OBJ_TCB_PTR(tci, bbr, tcp, print_tcp_bbr_info, len);
	tprint_union_end();
}

static void
print_tcp_repair_window(struct tcb *const tcp, const kernel_ulong_t addr,
			unsigned int len)
{
	struct tcp_repair_window trw;

	if (len > sizeof(trw))
		len = sizeof(trw);

	if (umoven_or_printaddr(tcp, addr, len, &trw))
		return;

	MAYBE_PRINT_FIELD_LEN(tprint_struct_begin(),
			      trw, snd_wl1, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      trw, snd_wnd, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      trw, max_window, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      trw, rcv_wnd, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      trw, rcv_wup, len, PRINT_FIELD_U);
	if (len > sizeof(trw)) {
		print_nonzero_bytes(tcp, tprint_struct_next, addr, sizeof(trw),
				    MIN(len, get_pagesize()), QUOTE_FORCE_HEX);
	}
	tprint_struct_end();
}

static long
print_tcp_zc(struct tcb *const tcp, const kernel_ulong_t addr, unsigned int len)
{
	struct tcp_zerocopy_receive {
		uint64_t address;		/* in     */
		uint32_t length;		/* in/out */
		uint32_t recv_skip_hint;	/*    out */
		uint32_t inq;			/*    out */
		int32_t  err;			/*    out */
		uint64_t copybuf_address;	/* in     */
		int32_t  copybuf_len;		/* in/out */
		uint32_t flags;			/* in     */
		uint64_t msg_control;		/* in/out */
		uint64_t msg_controllen;	/* in/out */
		uint32_t msg_flags;		/* in/out */
		uint32_t reserved;
	} tzc;

	if (umoven(tcp, addr, MIN(len, sizeof(tzc)), &tzc))
		return RVAL_DECODED;

	if (exiting(tcp))
		tprint_value_changed();

	if (entering(tcp)) {
		MAYBE_PRINT_FIELD_LEN(tprint_struct_begin(),
				      tzc, address, len, PRINT_FIELD_ADDR64);
	}
	MAYBE_PRINT_FIELD_LEN(entering(tcp) ? tprint_struct_next()
					    : tprint_struct_begin(),
			      tzc, length, len, PRINT_FIELD_X);
	if (exiting(tcp)) {
		MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
				      tzc, recv_skip_hint, len, PRINT_FIELD_U);
		MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
				      tzc, inq, len, PRINT_FIELD_U);
		MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
				      tzc, err, len, PRINT_FIELD_ERR_D);
	}
	if (entering(tcp)) {
		MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
				      tzc, copybuf_addr, len,
				      PRINT_FIELD_ADDR64);
	}
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      tzc, copybuf_len, len, PRINT_FIELD_X);
	if (entering(tcp)) {
		MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
				      tzc, flags, len, PRINT_FIELD_FLAGS,
				      tcp_zerocopy_flags,
				      "TCP_RECEIVE_ZEROCOPY_FLAG_???");
	}
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      tzc, msg_control, len, PRINT_FIELD_OBJ_TCB_PTR,
			      tcp, decode_msg_control, tzc.msg_controllen);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      tzc, msg_controllen, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      tzc, msg_flags, len, PRINT_FIELD_FLAGS_VERBOSE,
			      tcp_zerocopy_msg_flags, "TCP_CMSG_???");
	if (tzc.reserved) {
		PRINT_FIELD_X(tzc, reserved);
	}
	if (len > sizeof(tzc)) {
		print_nonzero_bytes(tcp, tprint_struct_next, addr, sizeof(tzc),
				    MIN(len, get_pagesize()), QUOTE_FORCE_HEX);
	}
	tprint_struct_end();

	return 0;
}

static void
print_tpacket_stats(struct tcb *const tcp, const kernel_ulong_t addr,
		    unsigned int len)
{
	struct tp_stats {
		unsigned int tp_packets, tp_drops, tp_freeze_q_cnt;
	} stats;

	if (umoven_or_printaddr(tcp, addr, MIN(len, sizeof(stats)), &stats))
		return;

	MAYBE_PRINT_FIELD_LEN(tprint_struct_begin(),
			      stats, tp_packets, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      stats, tp_drops, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      stats, tp_freeze_q_cnt, len, PRINT_FIELD_U);
	if (len > sizeof(stats)) {
		print_nonzero_bytes(tcp, tprint_struct_next, addr,
				    sizeof(stats), MIN(stats, get_pagesize()),
				    QUOTE_FORCE_HEX);
	}
	tprint_struct_end();
}

#include "xlat/icmpfilterflags.h"

static void
print_icmp_filter(struct tcb *const tcp, const kernel_ulong_t addr,
		  unsigned int len)
{
	struct icmp_filter filter = { ~0U };

	if (len > sizeof(filter))
		len = sizeof(filter);

	if (umoven_or_printaddr(tcp, addr, len, &filter))
		return;

	tprint_struct_begin();
	tprints_field_name("data");
	tprints("~(");
	printflags(icmpfilterflags, ~filter.data, "ICMP_???");
	tprints(")");
	tprint_struct_end();
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
		}
		break;

	case SOL_TCP:
		switch (name) {
		case TCP_INFO:
			print_tcp_info(tcp, addr, rlen);
			return;

		/* Removed in Linux commit v3.10-rc1~66^2~439 */
		case TCP_COOKIE_TRANSACTIONS:
			print_tcpct(tcp, addr, rlen)
			return;

		case TCP_REPAIR:
			print_tcp_repair(tcp, addr, rlen);
			return;

		case TCP_CC_INFO:
			print_tcp_cc_info(tcp, addr, rlen);
			return;

		case TCP_SAVED_SYN:
			printstr_ex(tcp, addr, rlen, QUOTE_FORCE_HEX);
			return;

		case TCP_REPAIR_WINDOW:
			print_tcp_repair_window(tcp, addr, rlen);
			return;

		case TCP_ULP:
			printstrn(tcp, addr, rlen);
			return;

		case TCP_FASTOPEN_KEY:
			printstr_ex(tcp, addr, rlen, QUOTE_FORCE_HEX);
			return;

		case TCP_ZEROCOPY_RECEIVE:
			print_tcp_zc(tcp, addr, rlen);
			return;

		/* Return int */
		case TCP_NODELAY:
		case TCP_MAXSEG:
		case TCP_CORK:
		case TCP_KEEPIDLE:
		case TCP_KEEPINTVL:
		case TCP_KEEPCNT:
		case TCP_SYNCNT:
		case TCP_LINGER2:
		case TCP_DEFER_ACCEPT:
		case TCP_WINDOW_CLAMP:
		case TCP_QUICKACK:
		case TCP_CONGESTION:
		/* setsockopt only: TCP_MD5SIG */
		case TCP_THIN_LINEAR_TIMEOUTS:
		case TCP_THIN_DUPACK:
		case TCP_USER_TIMEOUT:
		case TCP_REPAIR_QUEUE:
		case TCP_QUEUE_SEQ:
		/* setsockopt only: TCP_REPAIR_OPTIONS */
		case TCP_FASTOPEN:
		case TCP_TIMESTAMP:
		case TCP_NOTSENT_LOWAT:
		case TCP_SAVE_SYN:
		case TCP_FASTOPEN_CONNECT:
		/* setsockopt only: TCP_MD5SIG_EXT */
		/* setsockopt only: TCP_FASTOPEN_KEY */
		case TCP_FASTOPEN_NO_COOKIE:
		case TCP_INQ:
		case TCP_TX_DELAY:
		default:
			break;
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

	case SOL_MPTCP:
		switch (name) {
		case MPTCP_INFO:
			
		case MPTCP_TCPINFO:
		case MPTCP_SUBFLOW_ADDRS:
		}
		break;
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
	const int fd = tcp->u_arg[0];
	const int level = tcp->u_arg[1];
	const int optname = tcp->u_arg[2];
	const kernel_ulong_t optval = tcp->u_arg[3];
	const kernel_ulong_t optlen = tcp->u_arg[4];
	int ulen;
	int rlen;

	if (entering(tcp)) {
		long ret = RVAL_DECODED;

		print_sockopt_fd_level_name(tcp, fd, level, optname, true);
		tprint_arg_next();

		if (verbose(tcp) && optlen && umove(tcp, optlen, &ulen) == 0) {
			ret = 0;

			set_tcb_priv_ulong(tcp, ulen);

			/*
			 * optval: so far, only TCP_ZEROCOPY_RECEIVE is RW,
			 *         the rest is saner and only returns data.
			 */
			if (level == SOL_TCP && optname == TCP_ZEROCOPY_RECEIVE)
				ret = print_tcp_zc(tcp, optval, ulen);
		}

		if (ret) {
			/* optval */
			printaddr(optval);
			tprint_arg_next();

			/* optlen */
			printaddr(optlen);
		}

		return RVAL_DECODED;
	} else {
		rlen = ulen = get_tcb_priv_ulong(tcp);

		if (umove(tcp, optlen, &rlen) < 0 || syserror(tcp)) {
			/* optval */
			printaddr(optval);
			tprint_arg_next();

			/* optlen */
			tprint_indirect_begin();
			PRINT_VAL_D(ulen);
			if (level == SOL_TCP && optname == TCP_SAVED_SYN
			    && ulen != rlen) {
				tprint_value_changed();
				PRINT_VAL_D(rlen);
			}
			tprint_indirect_end();
		} else {
			/* optval */
			print_getsockopt(tcp, level, optname, optval,
					 ulen, rlen);
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

#ifdef PACKET_ADD_MEMBERSHIP
# include "xlat/packet_mreq_type.h"

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
#endif /* PACKET_ADD_MEMBERSHIP */

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
#ifdef PACKET_ADD_MEMBERSHIP
		case PACKET_ADD_MEMBERSHIP:
		case PACKET_DROP_MEMBERSHIP:
			print_packet_mreq(tcp, addr, len);
			return;
#endif /* PACKET_ADD_MEMBERSHIP */
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
	const int fd = tcp->u_arg[0];
	const int level = tcp->u_arg[1];
	const int optname = tcp->u_arg[2];
	const kernel_ulong_t optval = tcp->u_arg[3];
	const int optlen = tcp->u_arg[4];

	print_sockopt_fd_level_name(tcp, fd, level, optname, false);
	tprint_arg_next();

	/* optval */
	print_setsockopt(tcp, level, optname, optval, optlen);
	tprint_arg_next();

	/* optlen */
	PRINT_VAL_D(optlen);

	return RVAL_DECODED;
}
