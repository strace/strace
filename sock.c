/*
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
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
#include <sys/socket.h>
#if defined ALPHA || defined SH || defined SH64
# include <linux/ioctl.h>
#endif
#include <linux/sockios.h>
#include <arpa/inet.h>
#include <net/if.h>

#include "xlat/iffflags.h"

#define PRINT_IFREQ_ADDR(tcp, ifr, sockaddr)					\
	do {									\
		tprints(#sockaddr "=");						\
		print_sockaddr(tcp, &((ifr)->sockaddr),				\
			       sizeof((ifr)->sockaddr));			\
	} while (0)

static void
print_ifname(const char *ifname)
{
	print_quoted_string(ifname, IFNAMSIZ + 1, QUOTE_0_TERMINATED);
}

static void
print_ifreq(struct tcb *tcp, const unsigned int code, const long arg,
	    const struct ifreq *ifr)
{
	switch (code) {
	case SIOCSIFADDR:
	case SIOCGIFADDR:
		PRINT_IFREQ_ADDR(tcp, ifr, ifr_addr);
		break;
	case SIOCSIFDSTADDR:
	case SIOCGIFDSTADDR:
		PRINT_IFREQ_ADDR(tcp, ifr, ifr_dstaddr);
		break;
	case SIOCSIFBRDADDR:
	case SIOCGIFBRDADDR:
		PRINT_IFREQ_ADDR(tcp, ifr, ifr_broadaddr);
		break;
	case SIOCSIFNETMASK:
	case SIOCGIFNETMASK:
		PRINT_IFREQ_ADDR(tcp, ifr, ifr_netmask);
		break;
	case SIOCSIFHWADDR:
	case SIOCGIFHWADDR: {
		/* XXX Are there other hardware addresses
		   than 6-byte MACs?  */
		const unsigned char *bytes =
			(unsigned char *) &ifr->ifr_hwaddr.sa_data;
		tprintf("ifr_hwaddr=%02x:%02x:%02x:%02x:%02x:%02x",
			bytes[0], bytes[1], bytes[2],
			bytes[3], bytes[4], bytes[5]);
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
		tprintf("ifr_map={mem_start=%#lx, "
			"mem_end=%#lx, base_addr=%#x, "
			"irq=%u, dma=%u, port=%u}",
			ifr->ifr_map.mem_start,
			ifr->ifr_map.mem_end,
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
	const unsigned int n = (unsigned int) len / sizeof(struct ifreq);

	if (len < 0 || n * sizeof(struct ifreq) != (unsigned int) len)
		tprintf("%d", len);
	else
		tprintf("%u * sizeof(struct ifreq)", n);

	return n;
}

static int
decode_ifconf(struct tcb *tcp, const long addr)
{
	struct ifconf ifc;

	if (entering(tcp)) {
		tprints(", ");
		if (umove_or_printaddr(tcp, addr, &ifc))
			return RVAL_DECODED | 1;
		if (ifc.ifc_buf) {
			tprints("{");
			print_ifc_len(ifc.ifc_len);
		}
		return 1;
	}

	if (syserror(tcp) || umove(tcp, addr, &ifc) < 0) {
		if (ifc.ifc_buf)
			tprints("}");
		else
			printaddr(addr);
		return RVAL_DECODED | 1;
	}

	if (!ifc.ifc_buf) {
		tprints("{");
		print_ifc_len(ifc.ifc_len);
		tprints(", NULL}");
		return RVAL_DECODED | 1;
	}

	tprints(" => ");
	const unsigned int nifra = print_ifc_len(ifc.ifc_len);
	if (!nifra) {
		tprints("}");
		return RVAL_DECODED | 1;
	}

	struct ifreq ifra[nifra > max_strlen ? max_strlen : nifra];
	tprints(", ");
	if (umove_or_printaddr(tcp, (unsigned long) ifc.ifc_buf, &ifra)) {
		tprints("}");
		return RVAL_DECODED | 1;
	}

	tprints("[");
	unsigned int i;
	for (i = 0; i < ARRAY_SIZE(ifra); ++i) {
		if (i > 0)
			tprints(", ");
		tprints("{ifr_name=");
		print_ifname(ifra[i].ifr_name);
		tprints(", ");
		PRINT_IFREQ_ADDR(tcp, &ifra[i], ifr_addr);
		tprints("}");
	}
	if (i < nifra)
		tprints(", ...");
	tprints("]}");

	return RVAL_DECODED | 1;
}

int
sock_ioctl(struct tcb *tcp, const unsigned int code, const long arg)
{
	struct ifreq ifr;

	switch (code) {
	case SIOCGIFCONF:
		return decode_ifconf(tcp, arg);

#ifdef SIOCBRADDBR
	case SIOCBRADDBR:
	case SIOCBRDELBR:
		tprints(", ");
		printstr(tcp, arg, -1);
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
		break;

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
			return 1;
		} else {
			if (syserror(tcp)) {
				tprints("}");
				break;
			}

			tprints(", ");
			if (umove(tcp, arg, &ifr) < 0) {
				tprints("???}");
				break;
			}

			if (SIOCGIFNAME == code) {
				tprints("ifr_name=");
				print_ifname(ifr.ifr_name);
			} else {
				print_ifreq(tcp, code, arg, &ifr);
			}
			tprints("}");
			break;
		}

	default:
		return RVAL_DECODED;
	}

	return RVAL_DECODED | 1;
}
