/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-2000 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2005-2016 Dmitry V. Levin <ldv@altlinux.org>
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
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <linux/netlink.h>
#include <linux/if_packet.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>

#ifdef HAVE_NETIPX_IPX_H
# include <netipx/ipx.h>
#else
# include <linux/ipx.h>
#endif

#include "xlat/addrfams.h"
#include "xlat/arp_hardware_types.h"
#include "xlat/ethernet_protocols.h"
#include "xlat/af_packet_types.h"

#ifdef HAVE_BLUETOOTH_BLUETOOTH_H
# include <bluetooth/bluetooth.h>
# include <bluetooth/hci.h>
# include <bluetooth/l2cap.h>
# include <bluetooth/rfcomm.h>
# include <bluetooth/sco.h>

# include "xlat/hci_channels.h"
#endif

#define SIZEOF_SA_FAMILY sizeof(((struct sockaddr *) 0)->sa_family)

static void
print_sockaddr_data_un(const void *const buf, const int addrlen)
{
	const struct sockaddr_un *const sa_un = buf;
	const int un_len = addrlen > (int) sizeof(*sa_un)
			   ? (int) sizeof(*sa_un) : addrlen;
	const int path_len = un_len - SIZEOF_SA_FAMILY;

	tprints("sun_path=");
	if (sa_un->sun_path[0]) {
		print_quoted_string(sa_un->sun_path, path_len + 1,
				    QUOTE_0_TERMINATED);
	} else {
		tprints("@");
		print_quoted_string(sa_un->sun_path + 1, path_len - 1, 0);
	}
}

static void
print_sockaddr_data_in(const void *const buf, const int addrlen)
{
	const struct sockaddr_in *const sa_in = buf;

	tprintf("sin_port=htons(%u), sin_addr=inet_addr(\"%s\")",
		ntohs(sa_in->sin_port), inet_ntoa(sa_in->sin_addr));
}

#define SIN6_MIN_LEN offsetof(struct sockaddr_in6, sin6_scope_id)

static void
print_sockaddr_data_in6(const void *const buf, const int addrlen)
{
	const struct sockaddr_in6 *const sa_in6 = buf;

	char string_addr[100];
	inet_ntop(AF_INET6, &sa_in6->sin6_addr,
		  string_addr, sizeof(string_addr));
	tprintf("sin6_port=htons(%u), inet_pton(AF_INET6"
		", \"%s\", &sin6_addr), sin6_flowinfo=htonl(%u)",
		ntohs(sa_in6->sin6_port), string_addr,
		ntohl(sa_in6->sin6_flowinfo));

	if (addrlen <= (int) SIN6_MIN_LEN)
		return;

	tprints(", sin6_scope_id=");
#if defined IN6_IS_ADDR_LINKLOCAL && defined IN6_IS_ADDR_MC_LINKLOCAL
	if (IN6_IS_ADDR_LINKLOCAL(&sa_in6->sin6_addr)
	    || IN6_IS_ADDR_MC_LINKLOCAL(&sa_in6->sin6_addr))
		print_ifindex(sa_in6->sin6_scope_id);
	else
#endif
		tprintf("%u", sa_in6->sin6_scope_id);
}

static void
print_sockaddr_data_ipx(const void *const buf, const int addrlen)
{
	const struct sockaddr_ipx *const sa_ipx = buf;
	unsigned int i;

	tprintf("sipx_port=htons(%u)"
		", sipx_network=htonl(%#08x)"
		", sipx_node=[",
		ntohs(sa_ipx->sipx_port),
		ntohl(sa_ipx->sipx_network));
	for (i = 0; i < IPX_NODE_LEN; ++i) {
		tprintf("%s%#02x", i ? ", " : "",
			sa_ipx->sipx_node[i]);
	}
	tprintf("], sipx_type=%#02x", sa_ipx->sipx_type);
}

static void
print_sockaddr_data_nl(const void *const buf, const int addrlen)
{
	const struct sockaddr_nl *const sa_nl = buf;

	tprintf("nl_pid=%d, nl_groups=%#08x",
		sa_nl->nl_pid, sa_nl->nl_groups);
}

static void
print_sockaddr_data_ll(const void *const buf, const int addrlen)
{
	const struct sockaddr_ll *const sa_ll = buf;

	tprints("sll_protocol=htons(");
	printxval(ethernet_protocols, ntohs(sa_ll->sll_protocol), "ETH_P_???");
	tprints("), sll_ifindex=");
	print_ifindex(sa_ll->sll_ifindex);
	tprints(", sll_hatype=");
	printxval(arp_hardware_types, sa_ll->sll_hatype, "ARPHRD_???");
	tprints(", sll_pkttype=");
	printxval(af_packet_types, sa_ll->sll_pkttype, "PACKET_???");
	tprintf(", sll_halen=%u", sa_ll->sll_halen);
	if (sa_ll->sll_halen) {
		const unsigned int oob_halen =
			addrlen - offsetof(struct sockaddr_ll, sll_addr);
		unsigned int i;

		tprints(", sll_addr=[");
		for (i = 0; i < sa_ll->sll_halen; ++i) {
			if (i)
				tprints(", ");
			if (i >= oob_halen) {
				tprints("...");
				break;
			}
			tprintf("%#02x", sa_ll->sll_addr[i]);
		}
		tprints("]");
	}
}

static void
print_sockaddr_data_raw(const void *const buf, const int addrlen)
{
	const char *const data = buf + SIZEOF_SA_FAMILY;
	const int datalen = addrlen - SIZEOF_SA_FAMILY;

	tprints("sa_data=");
	print_quoted_string(data, datalen, 0);
}

#ifdef HAVE_BLUETOOTH_BLUETOOTH_H
static void
print_sockaddr_data_bt(const void *const buf, const int addrlen)
{
	switch (addrlen) {
		case sizeof(struct sockaddr_hci): {
			const struct sockaddr_hci *const hci = buf;
			tprintf("hci_dev=htobs(%hu), hci_channel=",
				btohs(hci->hci_dev));
			printxval(hci_channels, hci->hci_channel,
				  "HCI_CHANNEL_???");
			break;
		}
		case sizeof(struct sockaddr_sco): {
			const struct sockaddr_sco *const sco = buf;
			tprintf("sco_bdaddr=%02x:%02x:%02x:%02x:%02x:%02x",
				sco->sco_bdaddr.b[0], sco->sco_bdaddr.b[1],
				sco->sco_bdaddr.b[2], sco->sco_bdaddr.b[3],
				sco->sco_bdaddr.b[4], sco->sco_bdaddr.b[5]);
			break;
		}
		case sizeof(struct sockaddr_rc): {
			const struct sockaddr_rc *const rc = buf;
			tprintf("rc_bdaddr=%02x:%02x:%02x:%02x:%02x:%02x"
				", rc_channel=%u",
				rc->rc_bdaddr.b[0], rc->rc_bdaddr.b[1],
				rc->rc_bdaddr.b[2], rc->rc_bdaddr.b[3],
				rc->rc_bdaddr.b[4], rc->rc_bdaddr.b[5],
				rc->rc_channel);
			break;
		}
		case sizeof(struct sockaddr_l2): {
			const struct sockaddr_l2 *const l2 = buf;
			tprintf("l2_psm=htobs(%hu)"
				", l2_bdaddr=%02x:%02x:%02x:%02x:%02x:%02x"
				", l2_cid=htobs(%hu), l2_bdaddr_type=%u",
				btohs(l2->l2_psm),
				l2->l2_bdaddr.b[0], l2->l2_bdaddr.b[1],
				l2->l2_bdaddr.b[2], l2->l2_bdaddr.b[3],
				l2->l2_bdaddr.b[4], l2->l2_bdaddr.b[5],
				btohs(l2->l2_cid), l2->l2_bdaddr_type);
			break;
		}
		default:
			print_sockaddr_data_raw(buf, addrlen);
			break;
	}
}
#endif /* HAVE_BLUETOOTH_BLUETOOTH_H */

typedef void (* const sockaddr_printer)(const void *const, const int);

static const struct {
	const sockaddr_printer printer;
	const int min_len;
} sa_printers[] = {
	[AF_UNIX] = { print_sockaddr_data_un, SIZEOF_SA_FAMILY + 1 },
	[AF_INET] = { print_sockaddr_data_in, sizeof(struct sockaddr_in) },
	[AF_IPX] = { print_sockaddr_data_ipx, sizeof(struct sockaddr_ipx) },
	[AF_INET6] = { print_sockaddr_data_in6, SIN6_MIN_LEN },
	[AF_NETLINK] = { print_sockaddr_data_nl, SIZEOF_SA_FAMILY + 1 },
	[AF_PACKET] = { print_sockaddr_data_ll, sizeof(struct sockaddr_ll) },
#ifdef HAVE_BLUETOOTH_BLUETOOTH_H
	[AF_BLUETOOTH] = { print_sockaddr_data_bt, SIZEOF_SA_FAMILY + 1 },
#endif
};

void
print_sockaddr(struct tcb *tcp, const void *const buf, const int addrlen)
{
	const struct sockaddr *const sa = buf;

	tprints("{sa_family=");
	printxval(addrfams, sa->sa_family, "AF_???");

	if (addrlen > (int) SIZEOF_SA_FAMILY) {
		tprints(", ");

		if (sa->sa_family < ARRAY_SIZE(sa_printers)
		    && sa_printers[sa->sa_family].printer
		    && addrlen >= sa_printers[sa->sa_family].min_len) {
			sa_printers[sa->sa_family].printer(buf, addrlen);
		} else {
			print_sockaddr_data_raw(buf, addrlen);
		}
	}

	tprints("}");
}

int
decode_sockaddr(struct tcb *const tcp, const kernel_ulong_t addr, int addrlen)
{
	if (addrlen < 2) {
		printaddr(addr);
		return -1;
	}

	union {
		struct sockaddr sa;
		struct sockaddr_storage storage;
		char pad[sizeof(struct sockaddr_storage) + 1];
	} addrbuf;

	if ((unsigned) addrlen > sizeof(addrbuf.storage))
		addrlen = sizeof(addrbuf.storage);

	if (umoven_or_printaddr(tcp, addr, addrlen, addrbuf.pad))
		return -1;

	memset(&addrbuf.pad[addrlen], 0, sizeof(addrbuf.pad) - addrlen);

	print_sockaddr(tcp, &addrbuf, addrlen);

	return addrbuf.sa.sa_family;
}
