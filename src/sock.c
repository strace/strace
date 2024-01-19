/*
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-2023 The strace developers.
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

#include "xlat/iffflags.h"

#define XLAT_MACROS_ONLY
#include "xlat/arp_hardware_types.h"
#undef XLAT_MACROS_ONLY

static void
print_ifr_hwaddr(const typeof_field(struct_ifreq, ifr_hwaddr) *const p)
{
	tprint_struct_begin();
	PRINT_FIELD_XVAL(*p, sa_family, arp_hardware_types, "ARPHRD_???");
	tprint_struct_next();
	PRINT_FIELD_HWADDR_SZ(*p, sa_data, sizeof(p->sa_data), p->sa_family);
	tprint_struct_end();
}

static void
print_ifr_map(const typeof_field(struct_ifreq, ifr_map) *const p)
{
	tprint_struct_begin();
	PRINT_FIELD_X(*p, mem_start);
	tprint_struct_next();
	PRINT_FIELD_X(*p, mem_end);
	tprint_struct_next();
	PRINT_FIELD_X(*p, base_addr);
	tprint_struct_next();
	PRINT_FIELD_X(*p, irq);
	tprint_struct_next();
	PRINT_FIELD_X(*p, dma);
	tprint_struct_next();
	PRINT_FIELD_X(*p, port);
	tprint_struct_end();
}

static void
print_ifreq(struct tcb *const tcp, const unsigned int code,
	    const kernel_ulong_t arg, const struct_ifreq *const ifr)
{
	switch (code) {
	case SIOCSIFADDR:
	case SIOCGIFADDR:
		PRINT_FIELD_SOCKADDR(*ifr, ifr_addr, tcp);
		break;
	case SIOCSIFDSTADDR:
	case SIOCGIFDSTADDR:
		PRINT_FIELD_SOCKADDR(*ifr, ifr_dstaddr, tcp);
		break;
	case SIOCSIFBRDADDR:
	case SIOCGIFBRDADDR:
		PRINT_FIELD_SOCKADDR(*ifr, ifr_broadaddr, tcp);
		break;
	case SIOCSIFNETMASK:
	case SIOCGIFNETMASK:
		PRINT_FIELD_SOCKADDR(*ifr, ifr_netmask, tcp);
		break;
	case SIOCADDMULTI:
	case SIOCDELMULTI:
	case SIOCGIFHWADDR:
	case SIOCSIFHWADDR:
	case SIOCSIFHWBROADCAST:
		PRINT_FIELD_OBJ_PTR(*ifr, ifr_hwaddr, print_ifr_hwaddr);
		break;
	case SIOCSIFFLAGS:
	case SIOCGIFFLAGS:
		PRINT_FIELD_FLAGS(*ifr, ifr_flags, iffflags, "IFF_???");
		break;
	case SIOCGIFINDEX:
		PRINT_FIELD_D(*ifr, ifr_ifindex);
		break;
	case SIOCSIFMETRIC:
	case SIOCGIFMETRIC:
		PRINT_FIELD_D(*ifr, ifr_metric);
		break;
	case SIOCSIFMTU:
	case SIOCGIFMTU:
		PRINT_FIELD_D(*ifr, ifr_mtu);
		break;
	case SIOCSIFSLAVE:
	case SIOCGIFSLAVE:
		PRINT_FIELD_CSTRING(*ifr, ifr_slave);
		break;
	case SIOCSIFNAME:
		PRINT_FIELD_CSTRING(*ifr, ifr_newname);
		break;
	case SIOCGIFNAME:
		PRINT_FIELD_CSTRING(*ifr, ifr_name);
		break;
	case SIOCSIFTXQLEN:
	case SIOCGIFTXQLEN:
		PRINT_FIELD_D(*ifr, ifr_qlen);
		break;
	case SIOCSIFMAP:
	case SIOCGIFMAP:
		PRINT_FIELD_OBJ_PTR(*ifr, ifr_map, print_ifr_map);
		break;
	}
}

static unsigned int
print_ifc_len(int len)
{
	PRINT_VAL_D(len);

	const unsigned int n = (unsigned int) len / sizeof(struct_ifreq);
	if (len > 0 && n * sizeof(struct_ifreq) == (unsigned int) len) {
		tprint_comment_begin();
		PRINT_VAL_U(n);
		tprints_string(" * sizeof(struct ifreq)");
		tprint_comment_end();
	}

	return n;
}

static bool
print_ifconf_ifreq(struct tcb *tcp, void *elem_buf, size_t elem_size,
		   void *dummy)
{
	struct_ifreq *ifr = elem_buf;

	tprint_struct_begin();
	PRINT_FIELD_CSTRING(*ifr, ifr_name);
	tprint_struct_next();
	PRINT_FIELD_SOCKADDR(*ifr, ifr_addr, tcp);
	tprint_struct_end();

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

			tprint_arg_next();
			printaddr(addr);
		} else {
			/*
			 * We failed to fetch the structure on exiting syscall,
			 * print whatever was fetched on entering syscall.
			 */
			if (!entering_ifc->ifc_buf)
				print_ifc_len(entering_ifc->ifc_len);

			tprint_struct_next();
			PRINT_FIELD_PTR(*entering_ifc, ifc_buf);

			tprint_struct_end();
		}

		return RVAL_IOCTL_DECODED;
	}

	if (entering(tcp)) {
		tprint_arg_next();
		tprint_struct_begin();
		tprints_field_name("ifc_len");
		if (ifc->ifc_buf)
			print_ifc_len(ifc->ifc_len);

		set_tcb_priv_data(tcp, ifc, free);

		return 0;
	}

	/* exiting */

	if (entering_ifc->ifc_buf && (entering_ifc->ifc_len != ifc->ifc_len))
		tprint_value_changed();
	if (!entering_ifc->ifc_buf || (entering_ifc->ifc_len != ifc->ifc_len))
		print_ifc_len(ifc->ifc_len);

	if (!entering_ifc->ifc_buf || syserror(tcp)) {
		tprint_struct_next();
		PRINT_FIELD_PTR(*entering_ifc, ifc_buf);
		if (entering_ifc->ifc_buf != ifc->ifc_buf) {
			tprint_value_changed();
			printaddr(ptr_to_kulong(ifc->ifc_buf));
		}
	} else {
		struct_ifreq ifr;

		tprint_struct_next();
		tprints_field_name("ifc_buf");
		print_array(tcp, ptr_to_kulong(ifc->ifc_buf),
			    ifc->ifc_len / sizeof(struct_ifreq),
			    &ifr, sizeof(ifr),
			    tfetch_mem, print_ifconf_ifreq, NULL);
	}

	tprint_struct_end();

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
		tprint_arg_next();
		printstr_ex(tcp, arg, sizeof(ifr.ifr_name), QUOTE_0_TERMINATED);
		break;

	case FIOGETOWN:
	case SIOCATMARK:
	case SIOCGIFENCAP:
	case SIOCGPGRP:
#ifdef SIOCOUTQNSD
	case SIOCOUTQNSD:
#endif
		if (entering(tcp))
			return 0;
		ATTRIBUTE_FALLTHROUGH;

	case FIOSETOWN:
	case SIOCSIFENCAP:
	case SIOCSPGRP:
		tprint_arg_next();
		printnum_int(tcp, arg, "%d");
		break;

	case SIOCBRADDIF:
	case SIOCBRDELIF:
		tprint_arg_next();
		if (!umove_or_printaddr(tcp, arg, &ifr)) {
			tprint_struct_begin();
			PRINT_FIELD_IFINDEX(ifr, ifr_ifindex);
			tprint_struct_end();
		}
		break;

	case SIOCADDMULTI:
	case SIOCDELMULTI:
	case SIOCSIFADDR:
	case SIOCSIFBRDADDR:
	case SIOCSIFDSTADDR:
	case SIOCSIFFLAGS:
	case SIOCSIFHWADDR:
	case SIOCSIFHWBROADCAST:
	case SIOCSIFMAP:
	case SIOCSIFMETRIC:
	case SIOCSIFMTU:
	case SIOCSIFNAME:
	case SIOCSIFNETMASK:
	case SIOCSIFSLAVE:
	case SIOCSIFTXQLEN:
		tprint_arg_next();
		if (umove_or_printaddr(tcp, arg, &ifr))
			break;

		tprint_struct_begin();
		PRINT_FIELD_CSTRING(ifr, ifr_name);
		tprint_arg_next();
		print_ifreq(tcp, code, arg, &ifr);
		tprint_struct_end();
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
			tprint_arg_next();
			if (umove_or_printaddr(tcp, arg, &ifr))
				break;

			if (SIOCGIFNAME == code) {
				tprint_struct_begin();
				PRINT_FIELD_D(ifr, ifr_ifindex);
			} else {
				tprint_struct_begin();
				PRINT_FIELD_CSTRING(ifr, ifr_name);
			}
			return 0;
		} else {
			if (!syserror(tcp) && !umove(tcp, arg, &ifr)) {
				tprint_struct_next();
				print_ifreq(tcp, code, arg, &ifr);
			}
			tprint_struct_end();
			break;
		}

	case SIOCADDDLCI:
	case SIOCADDRT:
	case SIOCBONDCHANGEACTIVE:
	case SIOCBONDENSLAVE:
	case SIOCBONDINFOQUERY:
	case SIOCBONDRELEASE:
	case SIOCBONDSETHWADDR:
	case SIOCBONDSLAVEINFOQUERY:
	case SIOCDARP:
	case SIOCDELDLCI:
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
	case SIOCRTMSG:
	case SIOCSARP:
#ifdef SIOCSHWTSTAMP
	case SIOCSHWTSTAMP:
#endif
	case SIOCSIFBR:
	case SIOCSIFLINK:
	case SIOCSIFMEM:
	case SIOCSIFPFLAGS:
	case SIOCSIFVLAN:
	case SIOCSMIIREG:
	case SIOCSRARP:
	case SIOCWANDEV:
		tprint_arg_next();
		printaddr(arg);
		break;

	default:
		return RVAL_DECODED;
	}

	return RVAL_IOCTL_DECODED;
}
