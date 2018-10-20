/*
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-2018 The strace developers.
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
#include "print_fields.h"

#include <arpa/inet.h>

#include <linux/socket.h>
#include <linux/if.h>
#if defined ALPHA || defined SH || defined SH64
# include <linux/ioctl.h>
#endif
#include <linux/ax25.h>
#include <linux/netrom.h>
#include <linux/rose.h>
#include <linux/route.h>
#include <linux/ipv6_route.h>
#include <linux/sockios.h>
#include <linux/x25.h>

#include DEF_MPERS_TYPE(struct_ifconf)
#include DEF_MPERS_TYPE(struct_ifreq)

typedef struct ifconf struct_ifconf;
typedef struct ifreq struct_ifreq;

#include MPERS_DEFS

#include "xlat/iffflags.h"

#include "xlat/inet6_route_metrics.h"
#include "xlat/inet6_router_pref.h"
#include "xlat/netrom_route_types.h"
#include "xlat/route_flags.h"
#include "xlat/route_flags_inet6.h"

#define XLAT_MACROS_ONLY
# include "xlat/arp_hardware_types.h"
# include "xlat/route_nexthop_flags.h"
# include "xlat/routing_types.h"
#undef XLAT_MACROS_ONLY

static void
print_ifname(const char *ifname)
{
	print_quoted_string(ifname, IFNAMSIZ + 1, QUOTE_0_TERMINATED);
}

static void
print_ifreq(struct tcb *const tcp, const unsigned int code,
	    const kernel_ulong_t arg, const struct_ifreq *const ifr)
{
	switch (code) {
	case SIOCSIFADDR:
	case SIOCGIFADDR:
		PRINT_FIELD_SOCKADDR("", *ifr, ifr_addr);
		break;
	case SIOCSIFDSTADDR:
	case SIOCGIFDSTADDR:
		PRINT_FIELD_SOCKADDR("", *ifr, ifr_dstaddr);
		break;
	case SIOCSIFBRDADDR:
	case SIOCGIFBRDADDR:
		PRINT_FIELD_SOCKADDR("", *ifr, ifr_broadaddr);
		break;
	case SIOCSIFNETMASK:
	case SIOCGIFNETMASK:
		PRINT_FIELD_SOCKADDR("", *ifr, ifr_netmask);
		break;
	case SIOCSIFHWADDR:
	case SIOCGIFHWADDR: {
		PRINT_FIELD_XVAL("ifr_hwaddr={", ifr->ifr_hwaddr, sa_family,
				 arp_hardware_types, "ARPHRD_???");
		PRINT_FIELD_HWADDR_SZ(", ", ifr->ifr_hwaddr, sa_data,
				      sizeof(ifr->ifr_hwaddr.sa_data),
				      ifr->ifr_hwaddr.sa_family);
		tprints("}");
		break;
	}
	case SIOCSIFFLAGS:
	case SIOCGIFFLAGS:
		tprints("ifr_flags=");
		printflags(iffflags, (unsigned short) ifr->ifr_flags, "IFF_???");
		break;
	case SIOCSIFMETRIC:
	case SIOCGIFMETRIC:
		tprintf("ifr_metric=%d", ifr->ifr_metric);
		break;
	case SIOCSIFMTU:
	case SIOCGIFMTU:
		tprintf("ifr_mtu=%d", ifr->ifr_mtu);
		break;
	case SIOCSIFSLAVE:
	case SIOCGIFSLAVE:
		tprints("ifr_slave=");
		print_ifname(ifr->ifr_slave);
		break;
	case SIOCSIFTXQLEN:
	case SIOCGIFTXQLEN:
		tprintf("ifr_qlen=%d", ifr->ifr_qlen);
		break;
	case SIOCSIFMAP:
	case SIOCGIFMAP:
		tprintf("ifr_map={mem_start=%#" PRI_klx ", "
			"mem_end=%#" PRI_klx ", base_addr=%#x, "
			"irq=%u, dma=%u, port=%u}",
			(kernel_ulong_t) ifr->ifr_map.mem_start,
			(kernel_ulong_t) ifr->ifr_map.mem_end,
			(unsigned) ifr->ifr_map.base_addr,
			(unsigned) ifr->ifr_map.irq,
			(unsigned) ifr->ifr_map.dma,
			(unsigned) ifr->ifr_map.port);
		break;
	}
}

static unsigned int
print_ifc_len(int len)
{
	const unsigned int n = (unsigned int) len / sizeof(struct_ifreq);

	if (len < 0 || n * sizeof(struct_ifreq) != (unsigned int) len)
		tprintf("%d", len);
	else
		tprintf("%u * sizeof(struct ifreq)", n);

	return n;
}

static bool
print_ifconf_ifreq(struct tcb *tcp, void *elem_buf, size_t elem_size,
		   void *dummy)
{
	struct_ifreq *ifr = elem_buf;

	tprints("{ifr_name=");
	print_ifname(ifr->ifr_name);
	PRINT_FIELD_SOCKADDR(", ", *ifr, ifr_addr);
	tprints("}");

	return true;
}

static int
decode_set_ifreq(struct tcb *tcp, const int fd, const unsigned int code,
		 const kernel_ulong_t arg)
{
	struct_ifreq ifr;

	tprints(", ");
	if (umove_or_printaddr(tcp, arg, &ifr))
		break;

	tprints("{ifr_name=");
	print_ifname(ifr.ifr_name);
	tprints(", ");
	if (code == SIOCSIFNAME) {
		tprints("ifr_newname=");
		print_ifname(ifr.ifr_newname);
	} else {
		print_ifreq(tcp, code, arg, &ifr);
	}
	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
decode_get_ifreq(struct tcb *tcp, const int fd, const unsigned int code,
		 const kernel_ulong_t arg)
{
	struct_ifreq ifr;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &ifr))
			break;

		if (SIOCGIFNAME == code) {
			tprintf("{ifr_index=%d", ifr.ifr_ifindex);
		} else {
			tprints("{ifr_name=");
			print_ifname(ifr.ifr_name);
		}
		return 0;
	}

	if (syserror(tcp)) {
		tprints("}");
		return RVAL_IOCTL_DECODED;
	}

	tprints(", ");
	if (umove(tcp, arg, &ifr) < 0) {
		tprints("???}");
		return RVAL_IOCTL_DECODED;
	}
	}

	if (SIOCGIFNAME == code) {
		tprints("ifr_name=");
		print_ifname(ifr.ifr_name);
	} else {
		print_ifreq(tcp, code, arg, &ifr);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

/*
 * There are two different modes of operation:
 *
 * - Get buffer size.  In this case, the callee sets ifc_buf to NULL,
 *   and the kernel returns the buffer size in ifc_len.
 * - Get actual data.  In this case, the callee specifies the buffer address
 *   in ifc_buf and its size in ifc_len.  The kernel fills the buffer with
 *   the data, and its amount is returned in ifc_len.
 *
 * Note that, technically, the whole struct ifconf is overwritten,
 * so ifc_buf could be different on exit, but current ioctl handler
 * implementation does not touch it.
 */
static int
decode_ifconf(struct tcb *const tcp, const int fd, const unsigned int code,
	      uconst kernel_ulong_t addr)
{
	struct_ifconf *entering_ifc = NULL;
	struct_ifconf *ifc =
		entering(tcp) ? malloc(sizeof(*ifc)) : alloca(sizeof(*ifc));

	if (exiting(tcp)) {
		entering_ifc = get_tcb_priv_data(tcp);

		if (!entering_ifc) {
			error_func_msg("where is my ifconf?");
			return 0;
		}
	}

	if (!ifc || umove(tcp, addr, ifc) < 0) {
		if (entering(tcp)) {
			free(ifc);

			tprints(", ");
			printaddr(addr);
		} else {
			/*
			 * We failed to fetch the structure on exiting syscall,
			 * print whatever was fetched on entering syscall.
			 */
			if (!entering_ifc->ifc_buf)
				print_ifc_len(entering_ifc->ifc_len);

			tprints(", ifc_buf=");
			printaddr(ptr_to_kulong(entering_ifc->ifc_buf));

			tprints("}");
		}

		return RVAL_IOCTL_DECODED;
	}

	if (entering(tcp)) {
		tprints(", {ifc_len=");
		if (ifc->ifc_buf)
			print_ifc_len(ifc->ifc_len);

		set_tcb_priv_data(tcp, ifc, free);

		return 0;
	}

	/* exiting */

	if (entering_ifc->ifc_buf && (entering_ifc->ifc_len != ifc->ifc_len))
		tprints(" => ");
	if (!entering_ifc->ifc_buf || (entering_ifc->ifc_len != ifc->ifc_len))
		print_ifc_len(ifc->ifc_len);

	tprints(", ifc_buf=");

	if (!entering_ifc->ifc_buf || syserror(tcp)) {
		printaddr(ptr_to_kulong(entering_ifc->ifc_buf));
		if (entering_ifc->ifc_buf != ifc->ifc_buf) {
			tprints(" => ");
			printaddr(ptr_to_kulong(ifc->ifc_buf));
		}
	} else {
		struct_ifreq ifr;

		print_array(tcp, ptr_to_kulong(ifc->ifc_buf),
			    ifc->ifc_len / sizeof(struct_ifreq),
			    &ifr, sizeof(ifr),
			    tfetch_mem, print_ifconf_ifreq, NULL);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static void
decode_rtentry(struct tcb *tcp, kernel_ulong_t arg)
{
	struct rtentry e;

	if (umove_or_printaddr(tcp, arg, &e))
		return;

	tprints("{");
	if (e.rt_pad1) {
		PRINT_FIELD_X("", e, rt_pad1);
		tprints(", ");
	}
	PRINT_FIELD_SOCKADDR("", e, rt_dst);
	PRINT_FIELD_SOCKADDR(", ", e, rt_gateway);
	PRINT_FIELD_SOCKADDR(", ", e, rt_genmask);

	PRINT_FIELD_FLAGS(", ", e, rt_flags, route_flags, "RTF_???");

	if (e.rt_pad2)
		PRINT_FIELD_X(", ", e, rt_pad2);
	if (e.rt_pad3)
		PRINT_FIELD_X(", ", e, rt_pad3);
	if (e.rt_pad4) {
		tprintf(", rt_pad4=");
		printaddr((uintptr_t) e.rt_pad4);
	}

	PRINT_FIELD_U(", ", e, rt_metric);
	tprints(", rt_dev=");
	printstr_ex(tcp, (uintptr_t) e.rt_dev, IFNAMSIZ, QUOTE_0_TERMINATED);
	PRINT_FIELD_U(", ", e, rt_mtu);
	PRINT_FIELD_U(", ", e, rt_window);
	PRINT_FIELD_U(", ", e, rt_irtt);
	tprints("}");
}

static void
print_digipeaters(uint32_t val, const char *dps_name, ax25_address *dps,
		  size_t dps_sz)
{
	if (!val)
		return;

	size_t cnt = MIN(val, dps_sz);

	tprintf("%s=[", dps_name);
	for (size_t i = 0; i < cnt; i++) {
		if (i)
			tprints(", ");

		print_ax25_addr(dps + i);
	}
	tprints("]");
}

static void
decode_ax25_routes_struct(struct tcb *tcp, kernel_ulong_t arg)
{
	struct ax25_routes_struct e;

	if (umove_or_printaddr(tcp, arg, &e))
		return;

	PRINT_FIELD_AX25_ADDR("{", e, port_addr);
	PRINT_FIELD_AX25_ADDR(", ", e, dest_addr);

	PRINT_FIELD_U(", ", e, digi_count);
	print_digipeaters(e.digi_count, "digi_addr", ARRSZ_PAIR(e.digi_addr));
	tprints("}");
}

static void
decode_nr_route_struct(struct tcb *tcp, kernel_ulong_t arg)
{
	struct nr_route_struct e;

	if (umove_or_printaddr(tcp, arg, &e))
		return;

	PRINT_FIELD_XVAL("{", e, type, netrom_route_types, "NETROM_???");
	PRINT_FIELD_AX25_ADDR(", ", e, callsign);
	PRINT_FIELD_CSTRING(", ", e, device);
	PRINT_FIELD_U(", ", e, quality);
	PRINT_FIELD_CSTRING(", ", e, mnemonic);
	PRINT_FIELD_AX25_ADDR(", ", e, neighbour);
	PRINT_FIELD_U(", ", e, obs_count);
	PRINT_FIELD_U(", ", e, ndigis);
	print_digipeaters(e.ndigis, "digipeaters", ARRSZ_PAIR(e.digipeaters));
	tprints("}");
}

static void
decode_x25_route_struct(struct tcb *tcp, kernel_ulong_t arg)
{
	struct x25_route_struct e;

	if (umove_or_printaddr(tcp, arg, &e))
		return;

	PRINT_FIELD_X25_ADDR("{", e, address);
	PRINT_FIELD_U(", ", e, sigdigits);
	PRINT_FIELD_CSTRING(", ", e, device);
	tprints("}");
}

static void
print_inet6_route_pref(uint8_t pref)
{
	if (xlat_verbose(xlat_verbosity) != XLAT_STYLE_ABBREV)
		tprintf("%#x", pref << 27);

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW)
		return;

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE)
		tprints(" /* ");

	tprints("RTF_PREF(");
	printxval(inet6_router_pref, pref, "ICMPV6_ROUTER_PREF_???");
	tprints(")");

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE)
		tprints(" */");
}

static void
decode_in6_rtmsg(struct tcb *tcp, kernel_ulong_t arg)
{
	struct in6_rtmsg e;

	if (umove_or_printaddr(tcp, arg, &e))
		return;

	PRINT_FIELD_INET6_ADDR("{", e, rtmsg_dst);
	PRINT_FIELD_INET6_ADDR(", ", e, rtmsg_src);
	PRINT_FIELD_INET6_ADDR(", ", e, rtmsg_gateway);
	PRINT_FIELD_XVAL(", ", e, rtmsg_type, routing_types, "RTN_???");
	PRINT_FIELD_U(", ", e, rtmsg_dst_len);
	PRINT_FIELD_U(", ", e, rtmsg_src_len);
	PRINT_FIELD_XVAL(", ", e, rtmsg_metric, inet6_route_metrics,
			 "IP6_RT_PRIO_???");
	PRINT_FIELD_U(", ", e, rtmsg_info);

	uint32_t pref = (e.rtmsg_flags >> 27) & 0x3;
	uint32_t flags = e.rtmsg_flags & ~(0x3 << 27);

	tprints(", rtmsg_flags=");
	print_inet6_route_pref(pref);
	if (flags) {
		tprints("|");
		printflags_ex(flags, "RTF_???", XLAT_STYLE_DEFAULT,
			      route_nexthop_flags, route_flags_inet6, NULL);
	}
	PRINT_FIELD_IFINDEX(", ", e, rtmsg_ifindex);
	tprints("}");
}

static void
decode_rose_route_struct(struct tcb *tcp, kernel_ulong_t arg)
{
	struct rose_route_struct e;

	if (umove_or_printaddr(tcp, arg, &e))
		return;

	PRINT_FIELD_ROSE_ADDR("{", e, address);
	PRINT_FIELD_U(", ", e, mask);
	PRINT_FIELD_AX25_ADDR("{", e, neighbour);
	PRINT_FIELD_CSTRING(", ", e, device);
	PRINT_FIELD_U(", ", e, ndigis);
	print_digipeaters(e.ndigis, "digipeaters", ARRSZ_PAIR(e.digipeaters));
	tprints("}");
}

static int
decode_route_ioc(struct tcb *tcp, const int fd, const unsigned int code,
		 const kernel_ulong_t arg)
{
	typedef void (* handler)(struct tcb *tcp, kernel_ulong_t addr);

	/*
	 * Decoding of the argument depends on the network protocol
	 * of the socket associated fd they're called on.
	 * Unfortunately, we can't get it right away, so we should derive it
	 * from the socket protocol.
	 */
	static const handler handlers[] = {
		[AF_INET]	= decode_rtentry,
		[AF_AX25]	= decode_ax25_routes_struct,
		/* packet_ioctl explicitly calls inet_dgram_ops.ioctl */
		[AF_APPLETALK]	= decode_rtentry,
		[AF_NETROM]	= decode_nr_route_struct,
		[AF_X25]	= decode_x25_route_struct,
		[AF_INET6]	= decode_in6_rtmsg,
		[AF_ROSE]	= decode_rose_route_struct,
		[AF_PACKET]	= decode_rtentry,
	};

	enum sock_proto proto = getfdproto(tcp, fd);
	uint32_t family = MAX(get_family_by_proto(proto), 0);

	if (family < ARRAY_SIZE(handlers) && handlers[family]) {
		tprints(", ");
		handlers[family](tcp, arg);
		break;
	}

	return RVAL_DECODED;
}

MPERS_PRINTER_DECL(int, sock_ioctl,
		   struct tcb *tcp, const unsigned int code,
		   const kernel_ulong_t arg)
{

	switch (code) {
	case SIOCGIFCONF:
		return decode_ifconf(tcp, fd, code, arg);

#ifdef SIOCBRADDBR
	case SIOCBRADDBR:
	case SIOCBRDELBR:
		tprints(", ");
		printstr(tcp, arg);
		break;
#endif

#ifdef FIOSETOWN
	case FIOSETOWN:
#endif
#ifdef SIOCSPGRP
	case SIOCSPGRP:
#endif
		tprints(", ");
		printnum_int(tcp, arg, "%d");
		break;

#ifdef FIOGETOWN
	case FIOGETOWN:
#endif
#ifdef SIOCGPGRP
	case SIOCGPGRP:
#endif
#ifdef SIOCATMARK
	case SIOCATMARK:
#endif
		if (entering(tcp))
			return 0;
		tprints(", ");
		printnum_int(tcp, arg, "%d");
		break;

#ifdef SIOCBRADDIF
	case SIOCBRADDIF:
#endif
#ifdef SIOCBRDELIF
	case SIOCBRDELIF:
#endif
		/* no arguments */
		break;

	case SIOCSIFNAME:
	case SIOCSIFADDR:
	case SIOCSIFDSTADDR:
	case SIOCSIFBRDADDR:
	case SIOCSIFNETMASK:
	case SIOCSIFFLAGS:
	case SIOCSIFMETRIC:
	case SIOCSIFMTU:
	case SIOCSIFSLAVE:
	case SIOCSIFHWADDR:
	case SIOCSIFTXQLEN:
	case SIOCSIFMAP:
		return decode_set_ifreq(tcp, fd, code, arg);

	case SIOCGIFNAME:
	case SIOCGIFINDEX:
	case SIOCGIFADDR:
	case SIOCGIFDSTADDR:
	case SIOCGIFBRDADDR:
	case SIOCGIFNETMASK:
	case SIOCGIFFLAGS:
	case SIOCGIFMETRIC:
	case SIOCGIFMTU:
	case SIOCGIFSLAVE:
	case SIOCGIFHWADDR:
	case SIOCGIFTXQLEN:
	case SIOCGIFMAP:
		return decode_get_ifreq(tcp, fd, code, arg);

	case SIOCADDRT:
	case SIOCDELRT:
		return decode_route_ioc(tcp, fd, code, arg);

	default:
		return RVAL_DECODED;
	}

	return RVAL_IOCTL_DECODED;
}
