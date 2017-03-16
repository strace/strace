/*
 * Check decoding of sockaddr structures
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
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

#include "tests.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/ipx.h>
#include <linux/netlink.h>
#ifdef HAVE_BLUETOOTH_BLUETOOTH_H
# include <bluetooth/bluetooth.h>
# include <bluetooth/hci.h>
# include <bluetooth/l2cap.h>
# include <bluetooth/rfcomm.h>
# include <bluetooth/sco.h>
#endif

#ifdef HAVE_IF_INDEXTONAME
/* <linux/if.h> used to conflict with <net/if.h> */
extern unsigned int if_nametoindex(const char *);
#endif

static void
check_un(void)
{
	TAIL_ALLOC_OBJECT_VAR_PTR(struct sockaddr_un, un);
	un->sun_family = AF_UNIX;
	memset(un->sun_path, '0', sizeof(un->sun_path));
	unsigned int len = sizeof(*un);
	int ret = connect(-1, (void *) un, len);
	printf("connect(-1, {sa_family=AF_UNIX, sun_path=\"%.*u\"}"
	       ", %u) = %d EBADF (%m)\n",
	       (int) sizeof(un->sun_path), 0, len, ret);

	un->sun_path[1] = 0;
	ret = connect(-1, (void *) un, len);
	printf("connect(-1, {sa_family=AF_UNIX, sun_path=\"%u\"}, %u)"
	       " = %d EBADF (%m)\n", 0, len, ret);

	un->sun_path[0] = 0;
	un->sun_path[2] = 1;
	ret = connect(-1, (void *) un, len);
	printf("connect(-1, {sa_family=AF_UNIX, sun_path=@\"\\0\\001%.*u\"}"
	       ", %u) = %d EBADF (%m)\n",
	       (int) sizeof(un->sun_path) - 3, 0, len, ret);

	un = ((void *) un) - 2;
	un->sun_family = AF_UNIX;
	memset(un->sun_path, '0', sizeof(un->sun_path));
	len = sizeof(*un) + 2;
	ret = connect(-1, (void *) un, len);
	printf("connect(-1, {sa_family=AF_UNIX, sun_path=\"%.*u\"}"
	       ", %u) = %d EBADF (%m)\n",
	       (int) sizeof(un->sun_path), 0, len, ret);

	un->sun_path[0] = 0;
	ret = connect(-1, (void *) un, len);
	printf("connect(-1, {sa_family=AF_UNIX, sun_path=@\"%.*u\"}"
	       ", %u) = %d EBADF (%m)\n",
	       (int) sizeof(un->sun_path) - 1, 0, len, ret);

	un = ((void *) un) + 4;
	un->sun_family = AF_UNIX;
	len = sizeof(*un) - 2;
	ret = connect(-1, (void *) un, len);
	printf("connect(-1, {sa_family=AF_UNIX, sun_path=\"%.*u\"}"
	       ", %u) = %d EBADF (%m)\n",
	       (int) sizeof(un->sun_path) - 2, 0, len, ret);

	un->sun_path[0] = 0;
	ret = connect(-1, (void *) un, len);
	printf("connect(-1, {sa_family=AF_UNIX, sun_path=@\"%.*u\"}"
	       ", %u) = %d EBADF (%m)\n",
	       (int) sizeof(un->sun_path) - 3, 0, len, ret);

	len = sizeof(*un);
	ret = connect(-1, (void *) un, len);
	printf("connect(-1, %p, %u) = %d EBADF (%m)\n", un, len, ret);

	un = tail_alloc(sizeof(struct sockaddr_storage));
	un->sun_family = AF_UNIX;
	memset(un->sun_path, '0', sizeof(un->sun_path));
	len = sizeof(struct sockaddr_storage) + 1;
	ret = connect(-1, (void *) un, len);
	printf("connect(-1, {sa_family=AF_UNIX, sun_path=\"%.*u\"}"
	       ", %u) = %d EBADF (%m)\n",
	       (int) sizeof(un->sun_path), 0, len, ret);

	un->sun_path[0] = 0;
	ret = connect(-1, (void *) un, len);
	printf("connect(-1, {sa_family=AF_UNIX, sun_path=@\"%.*u\"}"
	       ", %u) = %d EBADF (%m)\n",
	       (int) sizeof(un->sun_path) - 1, 0, len, ret);
}

static void
check_in(void)
{
	const unsigned short h_port = 12345;
	static const char h_addr[] = "12.34.56.78";

	TAIL_ALLOC_OBJECT_VAR_PTR(struct sockaddr_in, in);
	in->sin_family = AF_INET;
	in->sin_port = htons(h_port);
	in->sin_addr.s_addr = inet_addr(h_addr);
	unsigned int len = sizeof(*in);
	int ret = connect(-1, (void *) in, len);
	printf("connect(-1, {sa_family=AF_INET, sin_port=htons(%hu)"
	       ", sin_addr=inet_addr(\"%s\")}, %u) = %d EBADF (%m)\n",
	       h_port, h_addr, len, ret);

	in = ((void *) in) - 4;
	in->sin_family = AF_INET;
	in->sin_port = htons(h_port);
	in->sin_addr.s_addr = inet_addr(h_addr);
	len = sizeof(*in) + 4;
	ret = connect(-1, (void *) in, len);
	printf("connect(-1, {sa_family=AF_INET, sin_port=htons(%hu)"
	       ", sin_addr=inet_addr(\"%s\")}, %u) = %d EBADF (%m)\n",
	       h_port, h_addr, len, ret);

	in = ((void *) in) + 8;
	in->sin_family = AF_INET;
	in->sin_port = 0;
	in->sin_addr.s_addr = 0;
	len = sizeof(*in) - 4;
	ret = connect(-1, (void *) in, len);
	printf("connect(-1, {sa_family=AF_INET, sa_data=\"%s\"}, %u)"
	       " = %d EBADF (%m)\n",
	       "\\0\\0\\0\\0\\0\\0\\377\\377\\377\\377",
	       len, ret);

	len = sizeof(*in);
	ret = connect(-1, (void *) in, len);
	printf("connect(-1, %p, %u) = %d EBADF (%m)\n", in, len, ret);
}

static void
check_in6_linklocal(struct sockaddr_in6 *const in6, const char *const h_addr)
{
	inet_pton(AF_INET6, h_addr, &in6->sin6_addr);

	in6->sin6_scope_id = 0xfacefeed;
	unsigned int len = sizeof(*in6);
	int ret = connect(-1, (void *) in6, len);
	printf("connect(-1, {sa_family=AF_INET6, sin6_port=htons(%hu)"
	       ", inet_pton(AF_INET6, \"%s\", &sin6_addr)"
	       ", sin6_flowinfo=htonl(%u)"
	       ", sin6_scope_id=%u}, %u)"
	       " = %d EBADF (%m)\n",
	       ntohs(in6->sin6_port), h_addr,
	       ntohl(in6->sin6_flowinfo), in6->sin6_scope_id, len, ret);

#ifdef HAVE_IF_INDEXTONAME
	in6->sin6_scope_id = if_nametoindex("lo");
	if (in6->sin6_scope_id) {
		ret = connect(-1, (void *) in6, len);
		printf("connect(-1, {sa_family=AF_INET6, sin6_port=htons(%hu)"
		       ", inet_pton(AF_INET6, \"%s\", &sin6_addr)"
		       ", sin6_flowinfo=htonl(%u)"
		       ", sin6_scope_id=if_nametoindex(\"lo\")}, %u)"
		       " = %d EBADF (%m)\n",
		       ntohs(in6->sin6_port), h_addr,
		       ntohl(in6->sin6_flowinfo), len, ret);
	}
#endif
}

static void
check_in6(void)
{
	const unsigned short h_port = 12345;
	const unsigned int h_flowinfo = 1234567890;
	static const char h_addr[] = "12:34:56:78:90:ab:cd:ef";

	TAIL_ALLOC_OBJECT_VAR_PTR(struct sockaddr_in6, in6);
	in6->sin6_family = AF_INET6;
	in6->sin6_port = htons(h_port);
	in6->sin6_flowinfo = htonl(h_flowinfo);
	inet_pton(AF_INET6, h_addr, &in6->sin6_addr);
	in6->sin6_scope_id = 0xfacefeed;
	unsigned int len = sizeof(*in6);
	int ret = connect(-1, (void *) in6, len);
	printf("connect(-1, {sa_family=AF_INET6, sin6_port=htons(%hu)"
	       ", inet_pton(AF_INET6, \"%s\", &sin6_addr)"
	       ", sin6_flowinfo=htonl(%u), sin6_scope_id=%u}, %u)"
	       " = %d EBADF (%m)\n",
	       h_port, h_addr, h_flowinfo, in6->sin6_scope_id, len, ret);

	check_in6_linklocal(in6, "fe80::");
	check_in6_linklocal(in6, "ff42::");

	in6 = ((void *) in6) - 4;
	in6->sin6_family = AF_INET6;
	in6->sin6_port = htons(h_port);
	in6->sin6_flowinfo = htonl(h_flowinfo);
	inet_pton(AF_INET6, h_addr, &in6->sin6_addr);
	in6->sin6_scope_id = 0xfacefeed;
	len = sizeof(*in6) + 4;
	ret = connect(-1, (void *) in6, len);
	printf("connect(-1, {sa_family=AF_INET6, sin6_port=htons(%hu)"
	       ", inet_pton(AF_INET6, \"%s\", &sin6_addr)"
	       ", sin6_flowinfo=htonl(%u), sin6_scope_id=%u}, %u)"
	       " = %d EBADF (%m)\n",
	       h_port, h_addr, h_flowinfo, in6->sin6_scope_id, len, ret);

	in6 = ((void *) in6) + 4 + sizeof(in6->sin6_scope_id);
	in6->sin6_family = AF_INET6;
	in6->sin6_port = htons(h_port);
	in6->sin6_flowinfo = htonl(h_flowinfo);
	inet_pton(AF_INET6, h_addr, &in6->sin6_addr);
	len = sizeof(*in6) - sizeof(in6->sin6_scope_id);
	ret = connect(-1, (void *) in6, len);
	printf("connect(-1, {sa_family=AF_INET6, sin6_port=htons(%hu)"
	       ", inet_pton(AF_INET6, \"%s\", &sin6_addr)"
	       ", sin6_flowinfo=htonl(%u)}, %u)"
	       " = %d EBADF (%m)\n",
	       h_port, h_addr, h_flowinfo, len, ret);

	in6 = ((void *) in6) + 4;
	in6->sin6_family = AF_INET6;
	in6->sin6_port = 0;
	in6->sin6_flowinfo = 0;
	memset(&in6->sin6_addr, '0', sizeof(in6->sin6_addr) - 4);
	len = sizeof(*in6) - sizeof(in6->sin6_scope_id) - 4;
	ret = connect(-1, (void *) in6, len);
	printf("connect(-1, {sa_family=AF_INET6"
	       ", sa_data=\"\\0\\0\\0\\0\\0\\000%.*u\"}, %u)"
	       " = %d EBADF (%m)\n",
	       (int) (len - offsetof(struct sockaddr_in6, sin6_addr)), 0,
	       len, ret);

	len = sizeof(*in6) - sizeof(in6->sin6_scope_id);
	ret = connect(-1, (void *) in6, len);
	printf("connect(-1, %p, %u) = %d EBADF (%m)\n", in6, len, ret);
}

static void
check_ipx(void)
{
	const unsigned short h_port = 12345;
	const unsigned int h_network = 0xfacefeed;
	struct sockaddr_ipx c_ipx = {
		.sipx_family = AF_IPX,
		.sipx_port = htons(h_port),
		.sipx_network = htonl(h_network),
		.sipx_node = "ABCDEF",
		.sipx_type = -1
	};
	void *ipx = tail_memdup(&c_ipx, sizeof(c_ipx));
	unsigned int len = sizeof(c_ipx);
	int ret = connect(-1, ipx, len);
	printf("connect(-1, {sa_family=AF_IPX, sipx_port=htons(%u)"
	       ", sipx_network=htonl(%#x)"
	       ", sipx_node=[%#02x, %#02x, %#02x, %#02x, %#02x, %#02x]"
	       ", sipx_type=%#02x}, %u) = %d EBADF (%m)\n",
	       h_port, h_network,
	       c_ipx.sipx_node[0], c_ipx.sipx_node[1],
	       c_ipx.sipx_node[2], c_ipx.sipx_node[3],
	       c_ipx.sipx_node[4], c_ipx.sipx_node[5],
	       c_ipx.sipx_type, len, ret);
}

static void
check_nl(void)
{
	TAIL_ALLOC_OBJECT_VAR_PTR(struct sockaddr_nl, nl);
	nl->nl_family = AF_NETLINK;
	nl->nl_pid = 1234567890;
	nl->nl_groups = 0xfacefeed;
	unsigned int len = sizeof(*nl);
	int ret = connect(-1, (void *) nl, len);
	printf("connect(-1, {sa_family=AF_NETLINK, nl_pid=%d"
	       ", nl_groups=%#08x}, %u) = %d EBADF (%m)\n",
	       nl->nl_pid, nl->nl_groups, len, ret);

	nl = ((void *) nl) - 4;
	nl->nl_family = AF_NETLINK;
	nl->nl_pid = 1234567890;
	nl->nl_groups = 0xfacefeed;
	len = sizeof(*nl) + 4;
	ret = connect(-1, (void *) nl, len);
	printf("connect(-1, {sa_family=AF_NETLINK, nl_pid=%d"
	       ", nl_groups=%#08x}, %u) = %d EBADF (%m)\n",
	       nl->nl_pid, nl->nl_groups, len, ret);
}

static void
check_ll(void)
{
	struct sockaddr_ll c_ll = {
		.sll_family = AF_PACKET,
		.sll_protocol = htons(ETH_P_ALL),
		.sll_ifindex = 0xfacefeed,
		.sll_hatype = ARPHRD_ETHER,
		.sll_pkttype = PACKET_HOST,
		.sll_halen = sizeof(c_ll.sll_addr),
		.sll_addr = "abcdefgh"
	};
	void *ll = tail_memdup(&c_ll, sizeof(c_ll));
	unsigned int len = sizeof(c_ll);
	int ret = connect(-1, ll, len);
	printf("connect(-1, {sa_family=AF_PACKET"
	       ", sll_protocol=htons(ETH_P_ALL)"
	       ", sll_ifindex=%u, sll_hatype=ARPHRD_ETHER"
	       ", sll_pkttype=PACKET_HOST, sll_halen=%u, sll_addr="
	       "[%#02x, %#02x, %#02x, %#02x, %#02x, %#02x, %#02x, %#02x]"
	       "}, %u) = %d EBADF (%m)\n",
	       c_ll.sll_ifindex, c_ll.sll_halen,
	       c_ll.sll_addr[0], c_ll.sll_addr[1],
	       c_ll.sll_addr[2], c_ll.sll_addr[3],
	       c_ll.sll_addr[4], c_ll.sll_addr[5],
	       c_ll.sll_addr[6], c_ll.sll_addr[7],
	       len, ret);

	((struct sockaddr_ll *) ll)->sll_halen++;
	ret = connect(-1, ll, len);
	printf("connect(-1, {sa_family=AF_PACKET"
	       ", sll_protocol=htons(ETH_P_ALL)"
	       ", sll_ifindex=%u, sll_hatype=ARPHRD_ETHER"
	       ", sll_pkttype=PACKET_HOST, sll_halen=%u, sll_addr="
	       "[%#02x, %#02x, %#02x, %#02x, %#02x, %#02x, %#02x, %#02x, ...]"
	       "}, %u) = %d EBADF (%m)\n",
	       c_ll.sll_ifindex, c_ll.sll_halen + 1,
	       c_ll.sll_addr[0], c_ll.sll_addr[1],
	       c_ll.sll_addr[2], c_ll.sll_addr[3],
	       c_ll.sll_addr[4], c_ll.sll_addr[5],
	       c_ll.sll_addr[6], c_ll.sll_addr[7],
	       len, ret);

	((struct sockaddr_ll *) ll)->sll_halen = 0;
	ret = connect(-1, ll, len);
	printf("connect(-1, {sa_family=AF_PACKET"
	       ", sll_protocol=htons(ETH_P_ALL)"
	       ", sll_ifindex=%u, sll_hatype=ARPHRD_ETHER"
	       ", sll_pkttype=PACKET_HOST, sll_halen=0}, %u)"
	       " = %d EBADF (%m)\n", c_ll.sll_ifindex, len, ret);

#ifdef HAVE_IF_INDEXTONAME
	const int id = if_nametoindex("lo");
	if (id) {
		((struct sockaddr_ll *) ll)->sll_ifindex = id;
		ret = connect(-1, ll, len);
		printf("connect(-1, {sa_family=AF_PACKET"
		       ", sll_protocol=htons(ETH_P_ALL)"
		       ", sll_ifindex=if_nametoindex(\"lo\")"
		       ", sll_hatype=ARPHRD_ETHER"
		       ", sll_pkttype=PACKET_HOST, sll_halen=0}, %u)"
		       " = %d EBADF (%m)\n", len, ret);
	}
#endif
}

#ifdef HAVE_BLUETOOTH_BLUETOOTH_H
static void
check_hci(void)
{
	const unsigned short h_port = 12345;
	TAIL_ALLOC_OBJECT_VAR_PTR(struct sockaddr_hci, hci);
	hci->hci_family = AF_BLUETOOTH;
	hci->hci_dev = htobs(h_port);
	hci->hci_channel = HCI_CHANNEL_RAW;
	unsigned int len = sizeof(*hci);
	int ret = connect(-1, (void *) hci, len);
	printf("connect(-1, {sa_family=AF_BLUETOOTH, hci_dev=htobs(%hu)"
	       ", hci_channel=HCI_CHANNEL_RAW}, %u) = %d EBADF (%m)\n",
	       h_port, len, ret);
}

static void
check_sco(void)
{
	const struct sockaddr_sco c_sco = {
		.sco_family = AF_BLUETOOTH,
		.sco_bdaddr.b = "abcdef"
	};
	void *sco = tail_memdup(&c_sco, sizeof(c_sco));
	unsigned int len = sizeof(c_sco);
	int ret = connect(-1, sco, len);
	printf("connect(-1, {sa_family=AF_BLUETOOTH"
	       ", sco_bdaddr=%02x:%02x:%02x:%02x:%02x:%02x"
	       "}, %u) = %d EBADF (%m)\n",
	       c_sco.sco_bdaddr.b[0], c_sco.sco_bdaddr.b[1],
	       c_sco.sco_bdaddr.b[2], c_sco.sco_bdaddr.b[3],
	       c_sco.sco_bdaddr.b[4], c_sco.sco_bdaddr.b[5],
	       len, ret);
}

static void
check_rc(void)
{
	const struct sockaddr_rc c_rc = {
		.rc_family = AF_BLUETOOTH,
		.rc_bdaddr.b = "abcdef",
		.rc_channel = 42
	};
	void *rc = tail_memdup(&c_rc, sizeof(c_rc));
	unsigned int len = sizeof(c_rc);
	int ret = connect(-1, rc, len);
	printf("connect(-1, {sa_family=AF_BLUETOOTH"
	       ", rc_bdaddr=%02x:%02x:%02x:%02x:%02x:%02x"
	       ", rc_channel=%u}, %u) = %d EBADF (%m)\n",
	       c_rc.rc_bdaddr.b[0], c_rc.rc_bdaddr.b[1],
	       c_rc.rc_bdaddr.b[2], c_rc.rc_bdaddr.b[3],
	       c_rc.rc_bdaddr.b[4], c_rc.rc_bdaddr.b[5],
	       c_rc.rc_channel, len, ret);
}

static void
check_l2(void)
{
	const unsigned short h_psm = 12345;
	const unsigned short h_cid = 13579;
	const struct sockaddr_l2 c_l2 = {
		.l2_family = AF_BLUETOOTH,
		.l2_psm = htobs(h_psm),
		.l2_bdaddr.b = "abcdef",
		.l2_cid = htobs(h_cid),
		.l2_bdaddr_type = 42
	};
	void *l2 = tail_memdup(&c_l2, sizeof(c_l2));
	unsigned int len = sizeof(c_l2);
	int ret = connect(-1, l2, len);
	printf("connect(-1, {sa_family=AF_BLUETOOTH"
	       ", l2_psm=htobs(%hu)"
	       ", l2_bdaddr=%02x:%02x:%02x:%02x:%02x:%02x"
	       ", l2_cid=htobs(%hu), l2_bdaddr_type=%u}"
	       ", %u) = %d EBADF (%m)\n", h_psm,
	       c_l2.l2_bdaddr.b[0], c_l2.l2_bdaddr.b[1],
	       c_l2.l2_bdaddr.b[2], c_l2.l2_bdaddr.b[3],
	       c_l2.l2_bdaddr.b[4], c_l2.l2_bdaddr.b[5],
	       h_cid, c_l2.l2_bdaddr_type, len, ret);
}
#endif

static void
check_raw(void)
{
	union {
		struct sockaddr *sa;
		struct sockaddr_storage *st;
	} u = { .st = tail_alloc(sizeof(*u.st)) };
	memset(u.st, '0', sizeof(*u.st));
	u.sa->sa_family = 0xff;
	unsigned int len = sizeof(*u.st) + 8;
	int ret = connect(-1, (void *) u.st, len);
	printf("connect(-1, {sa_family=%#x /* AF_??? */, sa_data=\"%.*u\"}"
	       ", %u) = %d EBADF (%m)\n", u.sa->sa_family,
	       (int) (sizeof(*u.st) - sizeof(u.sa->sa_family)), 0, len, ret);

	u.sa->sa_family = 0;
	len = sizeof(u.sa->sa_family) + 1;
	ret = connect(-1, (void *) u.st, len);
	printf("connect(-1, {sa_family=AF_UNSPEC, sa_data=\"0\"}, %u)"
	       " = %d EBADF (%m)\n", len, ret);

	u.sa->sa_family = AF_BLUETOOTH;
	++len;
	ret = connect(-1, (void *) u.st, len);
	printf("connect(-1, {sa_family=AF_BLUETOOTH, sa_data=\"00\"}, %u)"
	       " = %d EBADF (%m)\n", len, ret);
}

int
main(void)
{
	check_un();
	check_in();
	check_in6();
	check_ipx();
	check_nl();
	check_ll();
#ifdef HAVE_BLUETOOTH_BLUETOOTH_H
	check_hci();
	check_sco();
	check_rc();
	check_l2();
#endif
	check_raw();

	puts("+++ exited with 0 +++");
	return 0;
}
