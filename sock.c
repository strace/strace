/*
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include DEF_MPERS_TYPE(struct_ifconf)
#include DEF_MPERS_TYPE(struct_ifreq)

#include <sys/socket.h>
#include <net/if.h>

typedef struct ifconf struct_ifconf;
typedef struct ifreq struct_ifreq;

#include MPERS_DEFS

#include <linux/ioctl.h>
#include <linux/sockios.h>
#include <arpa/inet.h>

#include "print_fields.h"

#include "xlat/iffflags.h"

#define XLAT_MACROS_ONLY
#include "xlat/arp_hardware_types.h"
#undef XLAT_MACROS_ONLY

static void
print_ifreq(struct tcb *const tcp, const unsigned int code,
	    const kernel_ulong_t arg, const struct_ifreq *const ifr)
{
	switch (code) {
	case SIOCSIFADDR:
	case SIOCGIFADDR:
		PRINT_FIELD_SOCKADDR("", *ifr, ifr_addr, tcp);
		break;
	case SIOCSIFDSTADDR:
	case SIOCGIFDSTADDR:
		PRINT_FIELD_SOCKADDR("", *ifr, ifr_dstaddr, tcp);
		break;
	case SIOCSIFBRDADDR:
	case SIOCGIFBRDADDR:
		PRINT_FIELD_SOCKADDR("", *ifr, ifr_broadaddr, tcp);
		break;
	case SIOCSIFNETMASK:
	case SIOCGIFNETMASK:
		PRINT_FIELD_SOCKADDR("", *ifr, ifr_netmask, tcp);
		break;
	case SIOCSIFHWADDR:
	case SIOCGIFHWADDR:
		PRINT_FIELD_XVAL("ifr_hwaddr={", ifr->ifr_hwaddr, sa_family,
				 arp_hardware_types, "ARPHRD_???");
		PRINT_FIELD_HWADDR_SZ(", ", ifr->ifr_hwaddr, sa_data,
				      sizeof(ifr->ifr_hwaddr.sa_data),
				      ifr->ifr_hwaddr.sa_family);
		tprints("}");
		break;
	case SIOCSIFFLAGS:
	case SIOCGIFFLAGS:
		PRINT_FIELD_FLAGS("", *ifr, ifr_flags, iffflags, "IFF_???");
		break;
	case SIOCGIFINDEX:
		PRINT_FIELD_D("", *ifr, ifr_ifindex);
		break;
	case SIOCSIFMETRIC:
	case SIOCGIFMETRIC:
		PRINT_FIELD_D("", *ifr, ifr_metric);
		break;
	case SIOCSIFMTU:
	case SIOCGIFMTU:
		PRINT_FIELD_D("", *ifr, ifr_mtu);
		break;
	case SIOCSIFSLAVE:
	case SIOCGIFSLAVE:
		PRINT_FIELD_CSTRING("", *ifr, ifr_slave);
		break;
	case SIOCSIFNAME:
		PRINT_FIELD_CSTRING("", *ifr, ifr_newname);
		break;
	case SIOCGIFNAME:
		PRINT_FIELD_CSTRING("", *ifr, ifr_name);
		break;
	case SIOCSIFTXQLEN:
	case SIOCGIFTXQLEN:
		PRINT_FIELD_D("", *ifr, ifr_qlen);
		break;
	case SIOCSIFMAP:
	case SIOCGIFMAP:
		PRINT_FIELD_X("ifr_map={", ifr->ifr_map, mem_start);
		PRINT_FIELD_X(", ", ifr->ifr_map, mem_end);
		PRINT_FIELD_X(", ", ifr->ifr_map, base_addr);
		PRINT_FIELD_X(", ", ifr->ifr_map, irq);
		PRINT_FIELD_X(", ", ifr->ifr_map, dma);
		PRINT_FIELD_X(", ", ifr->ifr_map, port);
		tprints("}");
		break;
	}
}

static unsigned int
print_ifc_len(int len)
{
	const unsigned int n = (unsigned int) len / sizeof(struct_ifreq);

	if (len <= 0 || n * sizeof(struct_ifreq) != (unsigned int) len)
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

	PRINT_FIELD_CSTRING("{", *ifr, ifr_name);
	PRINT_FIELD_SOCKADDR(", ", *ifr, ifr_addr, tcp);
	tprints("}");

	return true;
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
decode_ifconf(struct tcb *const tcp, const kernel_ulong_t addr)
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

			PRINT_FIELD_PTR(", ", *entering_ifc, ifc_buf);

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

	if (!entering_ifc->ifc_buf || syserror(tcp)) {
		PRINT_FIELD_PTR(", ", *entering_ifc, ifc_buf);
		if (entering_ifc->ifc_buf != ifc->ifc_buf) {
			tprints(" => ");
			printaddr(ptr_to_kulong(ifc->ifc_buf));
		}
	} else {
		struct_ifreq ifr;

		tprints(", ifc_buf=");
		print_array(tcp, ptr_to_kulong(ifc->ifc_buf),
			    ifc->ifc_len / sizeof(struct_ifreq),
			    &ifr, sizeof(ifr),
			    tfetch_mem, print_ifconf_ifreq, NULL);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

MPERS_PRINTER_DECL(int, sock_ioctl,
		   struct tcb *tcp, const unsigned int code,
		   const kernel_ulong_t arg)
{
	struct_ifreq ifr;

	switch (code) {
	case SIOCGIFCONF:
		return decode_ifconf(tcp, arg);

	case SIOCBRADDBR:
	case SIOCBRDELBR:
		tprints(", ");
		printstr(tcp, arg);
		break;

	case FIOGETOWN:
	case SIOCATMARK:
	case SIOCGPGRP:
		if (entering(tcp))
			return 0;
		ATTRIBUTE_FALLTHROUGH;

	case FIOSETOWN:
	case SIOCSPGRP:
		tprints(", ");
		printnum_int(tcp, arg, "%d");
		break;

	case SIOCBRADDIF:
	case SIOCBRDELIF:
		tprints(", ");
		if (!umove_or_printaddr(tcp, arg, &ifr)) {
			PRINT_FIELD_IFINDEX("{", ifr, ifr_ifindex);
			tprints("}");
		}
		break;

	case SIOCSIFADDR:
	case SIOCSIFBRDADDR:
	case SIOCSIFDSTADDR:
	case SIOCSIFFLAGS:
	case SIOCSIFHWADDR:
	case SIOCSIFMAP:
	case SIOCSIFMETRIC:
	case SIOCSIFMTU:
	case SIOCSIFNAME:
	case SIOCSIFNETMASK:
	case SIOCSIFSLAVE:
	case SIOCSIFTXQLEN:
		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &ifr))
			break;

		PRINT_FIELD_CSTRING("{", ifr, ifr_name);
		tprints(", ");
		print_ifreq(tcp, code, arg, &ifr);
		tprints("}");
		break;

	case SIOCGIFADDR:
	case SIOCGIFBRDADDR:
	case SIOCGIFDSTADDR:
	case SIOCGIFFLAGS:
	case SIOCGIFHWADDR:
	case SIOCGIFINDEX:
	case SIOCGIFMAP:
	case SIOCGIFMETRIC:
	case SIOCGIFMTU:
	case SIOCGIFNAME:
	case SIOCGIFNETMASK:
	case SIOCGIFSLAVE:
	case SIOCGIFTXQLEN:
		if (entering(tcp)) {
			tprints(", ");
			if (umove_or_printaddr(tcp, arg, &ifr))
				break;

			if (SIOCGIFNAME == code) {
				PRINT_FIELD_D("{", ifr, ifr_ifindex);
			} else {
				PRINT_FIELD_CSTRING("{", ifr, ifr_name);
			}
			return 0;
		} else {
			if (!syserror(tcp) && !umove(tcp, arg, &ifr)) {
				tprints(", ");
				print_ifreq(tcp, code, arg, &ifr);
			}
			tprints("}");
			break;
		}

	case SIOCADDDLCI:
	case SIOCADDMULTI:
	case SIOCADDRT:
	case SIOCBONDCHANGEACTIVE:
	case SIOCBONDENSLAVE:
	case SIOCBONDINFOQUERY:
	case SIOCBONDRELEASE:
	case SIOCBONDSETHWADDR:
	case SIOCBONDSLAVEINFOQUERY:
	case SIOCDARP:
	case SIOCDELDLCI:
	case SIOCDELMULTI:
	case SIOCDELRT:
	case SIOCDIFADDR:
	case SIOCDRARP:
	case SIOCETHTOOL:
	case SIOCGARP:
#ifdef SIOCGHWTSTAMP
	case SIOCGHWTSTAMP:
#endif
	case SIOCGIFBR:
	case SIOCGIFCOUNT:
	case SIOCGIFENCAP:
	case SIOCGIFMEM:
	case SIOCGIFPFLAGS:
	case SIOCGIFVLAN:
	case SIOCGMIIPHY:
	case SIOCGMIIREG:
	case SIOCGRARP:
#ifdef SIOCGSKNS
	case SIOCGSKNS:
#endif
#ifdef SIOCGSTAMP_OLD
	case SIOCGSTAMP_OLD:
#endif
#ifdef SIOCGSTAMP_NEW
	case SIOCGSTAMP_NEW:
#endif
#ifdef SIOCGSTAMPNS_OLD
	case SIOCGSTAMPNS_OLD:
#endif
#ifdef SIOCGSTAMPNS_NEW
	case SIOCGSTAMPNS_NEW:
#endif
#ifdef SIOCOUTQNSD
	case SIOCOUTQNSD:
#endif
	case SIOCRTMSG:
	case SIOCSARP:
#ifdef SIOCSHWTSTAMP
	case SIOCSHWTSTAMP:
#endif
	case SIOCSIFBR:
	case SIOCSIFENCAP:
	case SIOCSIFHWBROADCAST:
	case SIOCSIFLINK:
	case SIOCSIFMEM:
	case SIOCSIFPFLAGS:
	case SIOCSIFVLAN:
	case SIOCSMIIREG:
	case SIOCSRARP:
	case SIOCWANDEV:
		tprints(", ");
		printaddr(arg);
		break;

	default:
		return RVAL_DECODED;
	}

	return RVAL_IOCTL_DECODED;
}
