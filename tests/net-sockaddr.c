/*
 * Check decoding of sockaddr structures
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "pidns.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "netlink.h"
#include <linux/ax25.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/x25.h>
#if defined HAVE_STRUCT_SOCKADDR_IPX_IN_LINUX_IPX_H
# include <linux/ipx.h>
#elif defined HAVE_STRUCT_SOCKADDR_IPX_IN_NETIPX_IPX_H
# include <netipx/ipx.h>
#endif
#ifdef HAVE_BLUETOOTH_BLUETOOTH_H
# include <bluetooth/bluetooth.h>
# include <bluetooth/hci.h>
# include <bluetooth/l2cap.h>
# include <bluetooth/rfcomm.h>
# include <bluetooth/sco.h>
#endif

static void
check_un(void)
{
	TAIL_ALLOC_OBJECT_VAR_PTR(struct sockaddr_un, un);
	un->sun_family = AF_UNIX;
	memset(un->sun_path, '0', sizeof(un->sun_path));
	unsigned int len = sizeof(*un);
	connect(-1, (void *) un, len);
	pidns_print_leader();
	printf("connect(-1, {sa_family=AF_UNIX, sun_path=\"%.*u\"}"
	       ", %u)" RVAL_EBADF,
	       (int) sizeof(un->sun_path), 0, len);

	un->sun_path[1] = 0;
	connect(-1, (void *) un, len);
	pidns_print_leader();
	printf("connect(-1, {sa_family=AF_UNIX, sun_path=\"%u\"}, %u)"
	       RVAL_EBADF, 0, len);

	un->sun_path[0] = 0;
	un->sun_path[2] = 1;
	connect(-1, (void *) un, len);
	pidns_print_leader();
	printf("connect(-1, {sa_family=AF_UNIX, sun_path=@\"\\0\\001%.*u\"}"
	       ", %u)" RVAL_EBADF,
	       (int) sizeof(un->sun_path) - 3, 0, len);

	un = ((void *) un) - 2;
	un->sun_family = AF_UNIX;
	memset(un->sun_path, '0', sizeof(un->sun_path));
	len = sizeof(*un) + 2;
	connect(-1, (void *) un, len);
	pidns_print_leader();
	printf("connect(-1, {sa_family=AF_UNIX, sun_path=\"%.*u\"}"
	       ", %u)" RVAL_EBADF,
	       (int) sizeof(un->sun_path), 0, len);

	un->sun_path[0] = 0;
	connect(-1, (void *) un, len);
	pidns_print_leader();
	printf("connect(-1, {sa_family=AF_UNIX, sun_path=@\"%.*u\"}"
	       ", %u)" RVAL_EBADF,
	       (int) sizeof(un->sun_path) - 1, 0, len);

	un = ((void *) un) + 4;
	un->sun_family = AF_UNIX;
	len = sizeof(*un) - 2;
	connect(-1, (void *) un, len);
	pidns_print_leader();
	printf("connect(-1, {sa_family=AF_UNIX, sun_path=\"%.*u\"}"
	       ", %u)" RVAL_EBADF,
	       (int) sizeof(un->sun_path) - 2, 0, len);

	un->sun_path[0] = 0;
	connect(-1, (void *) un, len);
	pidns_print_leader();
	printf("connect(-1, {sa_family=AF_UNIX, sun_path=@\"%.*u\"}"
	       ", %u)" RVAL_EBADF,
	       (int) sizeof(un->sun_path) - 3, 0, len);

	len = sizeof(*un);
	connect(-1, (void *) un, len);
	pidns_print_leader();
	printf("connect(-1, %p, %u)" RVAL_EBADF, un, len);

	un = tail_alloc(sizeof(struct sockaddr_storage));
	un->sun_family = AF_UNIX;
	memset(un->sun_path, '0', sizeof(un->sun_path));
	len = sizeof(struct sockaddr_storage) + 1;
	connect(-1, (void *) un, len);
	pidns_print_leader();
	printf("connect(-1, {sa_family=AF_UNIX, sun_path=\"%.*u\"}"
	       ", %u)" RVAL_EBADF,
	       (int) sizeof(un->sun_path), 0, len);

	un->sun_path[0] = 0;
	connect(-1, (void *) un, len);
	pidns_print_leader();
	printf("connect(-1, {sa_family=AF_UNIX, sun_path=@\"%.*u\"}"
	       ", %u)" RVAL_EBADF,
	       (int) sizeof(un->sun_path) - 1, 0, len);
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
	connect(-1, (void *) in, len);
	pidns_print_leader();
	printf("connect(-1, {sa_family=AF_INET, sin_port=htons(%hu)"
	       ", sin_addr=inet_addr(\"%s\")}, %u)" RVAL_EBADF,
	       h_port, h_addr, len);

	in = ((void *) in) - 4;
	in->sin_family = AF_INET;
	in->sin_port = htons(h_port);
	in->sin_addr.s_addr = inet_addr(h_addr);
	len = sizeof(*in) + 4;
	connect(-1, (void *) in, len);
	pidns_print_leader();
	printf("connect(-1, {sa_family=AF_INET, sin_port=htons(%hu)"
	       ", sin_addr=inet_addr(\"%s\")}, %u)" RVAL_EBADF,
	       h_port, h_addr, len);

	in = ((void *) in) + 8;
	in->sin_family = AF_INET;
	in->sin_port = 0;
	in->sin_addr.s_addr = 0;
	len = sizeof(*in) - 4;
	connect(-1, (void *) in, len);
	pidns_print_leader();
	printf("connect(-1, {sa_family=AF_INET, sa_data=\"%s\"}, %u)"
	       RVAL_EBADF,
	       "\\0\\0\\0\\0\\0\\0\\377\\377\\377\\377",
	       len);

	len = sizeof(*in);
	connect(-1, (void *) in, len);
	pidns_print_leader();
	printf("connect(-1, %p, %u)" RVAL_EBADF, in, len);
}

static void
check_in6_linklocal(struct sockaddr_in6 *const in6, const char *const h_addr)
{
	inet_pton(AF_INET6, h_addr, &in6->sin6_addr);

	in6->sin6_scope_id = 0xfacefeed;
	unsigned int len = sizeof(*in6);
	connect(-1, (void *) in6, len);
	pidns_print_leader();
	printf("connect(-1, {sa_family=AF_INET6, sin6_port=htons(%hu)"
	       ", sin6_flowinfo=htonl(%u)"
	       ", inet_pton(AF_INET6, \"%s\", &sin6_addr)"
	       ", sin6_scope_id=%u}, %u)"
	       RVAL_EBADF,
	       ntohs(in6->sin6_port), ntohl(in6->sin6_flowinfo),
	       h_addr, in6->sin6_scope_id, len);

	in6->sin6_scope_id = ifindex_lo();
	if (in6->sin6_scope_id) {
		connect(-1, (void *) in6, len);
		pidns_print_leader();
	printf("connect(-1, {sa_family=AF_INET6, sin6_port=htons(%hu)"
		       ", sin6_flowinfo=htonl(%u)"
		       ", inet_pton(AF_INET6, \"%s\", &sin6_addr)"
		       ", sin6_scope_id=%s}, %u)"
		       RVAL_EBADF,
		       ntohs(in6->sin6_port), ntohl(in6->sin6_flowinfo), h_addr,
		       IFINDEX_LO_STR, len);
	}
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
	connect(-1, (void *) in6, len);
	pidns_print_leader();
	printf("connect(-1, {sa_family=AF_INET6, sin6_port=htons(%hu)"
	       ", sin6_flowinfo=htonl(%u)"
	       ", inet_pton(AF_INET6, \"%s\", &sin6_addr)"
	       ", sin6_scope_id=%u}, %u)"
	       RVAL_EBADF,
	       h_port, h_flowinfo, h_addr, in6->sin6_scope_id, len);

	check_in6_linklocal(in6, "fe80::");
	check_in6_linklocal(in6, "ff42::");

	in6 = ((void *) in6) - 4;
	in6->sin6_family = AF_INET6;
	in6->sin6_port = htons(h_port);
	in6->sin6_flowinfo = htonl(h_flowinfo);
	inet_pton(AF_INET6, h_addr, &in6->sin6_addr);
	in6->sin6_scope_id = 0xfacefeed;
	len = sizeof(*in6) + 4;
	connect(-1, (void *) in6, len);
	pidns_print_leader();
	printf("connect(-1, {sa_family=AF_INET6, sin6_port=htons(%hu)"
	       ", sin6_flowinfo=htonl(%u)"
	       ", inet_pton(AF_INET6, \"%s\", &sin6_addr)"
	       ", sin6_scope_id=%u}, %u)"
	       RVAL_EBADF,
	       h_port, h_flowinfo, h_addr, in6->sin6_scope_id, len);

	in6 = ((void *) in6) + 4 + sizeof(in6->sin6_scope_id);
	in6->sin6_family = AF_INET6;
	in6->sin6_port = htons(h_port);
	in6->sin6_flowinfo = htonl(h_flowinfo);
	inet_pton(AF_INET6, h_addr, &in6->sin6_addr);
	len = sizeof(*in6) - sizeof(in6->sin6_scope_id);
	connect(-1, (void *) in6, len);
	pidns_print_leader();
	printf("connect(-1, {sa_family=AF_INET6, sin6_port=htons(%hu)"
	       ", sin6_flowinfo=htonl(%u)"
	       ", inet_pton(AF_INET6, \"%s\", &sin6_addr)}, %u)"
	       RVAL_EBADF,
	       h_port, h_flowinfo, h_addr, len);

	in6 = ((void *) in6) + 4;
	in6->sin6_family = AF_INET6;
	in6->sin6_port = 0;
	in6->sin6_flowinfo = 0;
	memset(&in6->sin6_addr, '0', sizeof(in6->sin6_addr) - 4);
	len = sizeof(*in6) - sizeof(in6->sin6_scope_id) - 4;
	connect(-1, (void *) in6, len);
	pidns_print_leader();
	printf("connect(-1, {sa_family=AF_INET6"
	       ", sa_data=\"\\0\\0\\0\\0\\0\\000%.*u\"}, %u)"
	       RVAL_EBADF,
	       (int) (len - offsetof(struct sockaddr_in6, sin6_addr)), 0,
	       len);

	len = sizeof(*in6) - sizeof(in6->sin6_scope_id);
	connect(-1, (void *) in6, len);
	pidns_print_leader();
	printf("connect(-1, %p, %u)" RVAL_EBADF, in6, len);
}

#ifdef HAVE_STRUCT_SOCKADDR_IPX
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
	struct sockaddr_ipx *ipx = tail_memdup(&c_ipx, sizeof(c_ipx));
	unsigned int len = sizeof(c_ipx);

	for (size_t i = 0; i < 2; i++) {
		ipx->sipx_zero = i ? 0x42 : 0;
		connect(-1, (void *) ipx, len);
		pidns_print_leader();
		printf("connect(-1, {sa_family=AF_IPX, sipx_port=htons(%u)"
		       ", sipx_network=htonl(%#x)"
		       ", sipx_node=[%#02x, %#02x, %#02x, %#02x, %#02x, %#02x]"
		       ", sipx_type=%#02x%s}, %u)" RVAL_EBADF,
		       h_port, h_network,
		       c_ipx.sipx_node[0], c_ipx.sipx_node[1],
		       c_ipx.sipx_node[2], c_ipx.sipx_node[3],
		       c_ipx.sipx_node[4], c_ipx.sipx_node[5],
		       c_ipx.sipx_type, i ? ", sipx_zero=0x42" : "",
		       len);
	}
}
#endif /* HAVE_STRUCT_SOCKADDR_IPX */

/* for a bit more compact AX.25 address definitions */
#define AX25_ADDR(c_, s_) \
	{ { (c_)[0] << 1, (c_)[1] << 1, (c_)[2] << 1, \
	    (c_)[3] << 1, (c_)[4] << 1, (c_)[5] << 1, (s_) << 1 } } \
	/* End of AX25_ADDR definition */

static void
check_ax25(void)
{
	const struct full_sockaddr_ax25 ax25 = {
		.fsa_ax25 = {
			.sax25_family = AF_AX25,
			.sax25_call = AX25_ADDR("VALID ", 13),
			.sax25_ndigis = 8,
		},
		.fsa_digipeater = {
			AX25_ADDR("SPA CE", 0),
			AX25_ADDR("SSID  ", 16),
			AX25_ADDR("      ", 0),
			AX25_ADDR("NULL\0", 3),
			AX25_ADDR("A-B-C", 4),
			AX25_ADDR(",}]\"\\'", 5),
			AX25_ADDR("DASH-0", 6),
			AX25_ADDR("\n\tABCD", 7),
		},
	};
	const ax25_address aux_addrs[] = {
		AX25_ADDR("VALID2", 7),
		AX25_ADDR("OK    ", 15),
		AX25_ADDR("FINE  ", 2),
		AX25_ADDR("smalls", 9),
	};

	enum { AX25_ALIGN = ALIGNOF(struct full_sockaddr_ax25), };
	size_t size = sizeof(ax25);
	size_t surplus = ROUNDUP(sizeof(ax25_address), AX25_ALIGN);
	void *sax_void = midtail_alloc(size, surplus);
	struct full_sockaddr_ax25 *sax = sax_void;
	long rc;

	fill_memory(sax, size);
	sax->fsa_ax25.sax25_family = AF_AX25;
	rc = connect(-1, sax_void, sizeof(struct sockaddr_ax25) - 1);
	pidns_print_leader();
	printf("connect(-1, {sa_family=AF_AX25, sa_data=\"\\202\\203\\204\\205"
	       "\\206\\207\\210\\211\\212\\213\\214\\215\\216\"}, %zu) = %s\n",
	       sizeof(struct sockaddr_ax25) - 1, sprintrc(rc));

	memcpy(sax, &ax25, sizeof(ax25));
	rc = connect(-1, sax_void, sizeof(struct sockaddr_ax25));
	pidns_print_leader();
	printf("connect(-1, {sa_family=AF_AX25, fsa_ax25={sax25_call=VALID-13"
	       ", sax25_ndigis=8}, fsa_digipeater=[...]}, %zu) = %s\n",
	       sizeof(struct sockaddr_ax25), sprintrc(rc));

	sax->fsa_ax25.sax25_ndigis = 0;
	rc = connect(-1, sax_void, sizeof(struct sockaddr_ax25));
	pidns_print_leader();
	printf("connect(-1, {sa_family=AF_AX25, sax25_call=VALID-13"
	       ", sax25_ndigis=0}, %zu) = %s\n",
	       sizeof(struct sockaddr_ax25), sprintrc(rc));

	sax->fsa_ax25.sax25_ndigis = 8;
	size = sizeof(struct sockaddr_ax25) + sizeof(ax25_address) * 3 + 1;
	rc = connect(-1, sax_void, size);
	pidns_print_leader();
	printf("connect(-1, {sa_family=AF_AX25, fsa_ax25={sax25_call=VALID-13"
	       ", sax25_ndigis=8}, fsa_digipeater"
	       "=[{ax25_call=\"\\xa6\\xa0\\x82\\x40\\x86\\x8a\\x00\""
		   "} /* SPA CE-0 */"
	       ", {ax25_call=\"\\xa6\\xa6\\x92\\x88\\x40\\x40\\x20\""
	           "} /* SSID-0 */"
	       ", *"
	       ", ...], ...}, %zu) = %s\n",
	       size, sprintrc(rc));

	sax->fsa_digipeater[2].ax25_call[6] = 0x4;
	size = sizeof(struct sockaddr_ax25) + sizeof(ax25_address) * 4;
	rc = connect(-1, sax_void, size);
	pidns_print_leader();
	printf("connect(-1, {sa_family=AF_AX25, fsa_ax25={sax25_call=VALID-13"
	       ", sax25_ndigis=8}, fsa_digipeater"
	       "=[{ax25_call=\"\\xa6\\xa0\\x82\\x40\\x86\\x8a\\x00\""
		   "} /* SPA CE-0 */"
	       ", {ax25_call=\"\\xa6\\xa6\\x92\\x88\\x40\\x40\\x20\""
	           "} /* SSID-0 */"
	       ", {ax25_call=\"\\x40\\x40\\x40\\x40\\x40\\x40\\x04\"} /* -2 */"
	       ", {ax25_call=\"\\x9c\\xaa\\x98\\x98\\x00\\x00\\x06\"}"
	       ", ...]}, %zu) = %s\n",
	       size, sprintrc(rc));

	memcpy(sax->fsa_digipeater, aux_addrs, sizeof(aux_addrs));
	sax->fsa_digipeater[2].ax25_call[6] = 0xa5;
	sax->fsa_digipeater[4].ax25_call[5] = 0x40;
	for (size_t i = 0; i < 3; i++) {
		size = sizeof(ax25) + sizeof(ax25_address) * (i / 2);
		rc = connect(-1, sax_void, size);
		pidns_print_leader();
		printf("connect(-1, {sa_family=AF_AX25"
		       ", fsa_ax25={sax25_call=VALID-13, sax25_ndigis=%d}"
		       ", fsa_digipeater=[VALID2-7, OK-15, %s /* FINE-2 */"
		       ", {ax25_call=\"\\xe6\\xda\\xc2\\xd8\\xd8\\xe6\\x12\""
		           "} /* smalls-9 */"
		       ", {ax25_call=\"\\x%s\\x%s\\x84\\x5a\\x86\\x40\\x08\""
		           "} /* %sB-C-4 */"
		       ", {ax25_call=\"\\x58\\xfa\\xba\\x44\\x%s\\x%s\\x0a\""
		           "}%s"
		       ", {ax25_call=\"\\x88\\x82\\xa6\\x90\\x5a\\x%s\\x0c\""
		           "}%s"
		       "%s]%s}, %zu) = %s\n"
		       , sax->fsa_ax25.sax25_ndigis
		       , i
		       ? "{ax25_call=\"\\x8c\\x92\\x9c\\x8a\\x40\\x41\\x04\"}"
		       : "{ax25_call=\"\\x8c\\x92\\x9c\\x8a\\x40\\x40\\xa5\"}"
		       , i ? "40" : "82"
		       , i ? "40" : "5a"
		       , i ? "  " : "A-"
		       , i ? "54" : "b8"
		       , i ? "5e" : "4e"
		       , i ? "" : " /* ,}]\"\\'-5 */"
		       , i ? "fe" : "60"
		       , i ? "" : " /* DASH-0-6 */"
		       , i == 1
		       ? ""
		       : ", {ax25_call=\"\\x14\\x12\\x82\\x84\\x86\\x88\\x0e\"}"
		       , i > 1 ? ", ..." : ""
		       , size, sprintrc(rc));

		if (i == 1) {
			sax_void = (char *) sax_void - surplus;
			memmove(sax_void, sax, sizeof(ax25));
			sax = sax_void;
		}

		sax->fsa_ax25.sax25_ndigis = 7 + 2 * i;

		sax->fsa_digipeater[2].ax25_call[5] = 0x41;
		sax->fsa_digipeater[2].ax25_call[6] = 0x4;

		sax->fsa_digipeater[4].ax25_call[0] = 0x40;
		sax->fsa_digipeater[4].ax25_call[1] = 0x40;

		sax->fsa_digipeater[5].ax25_call[4] = '*' << 1;
		sax->fsa_digipeater[5].ax25_call[5] = '/' << 1;

		sax->fsa_digipeater[6].ax25_call[5] = 0xfe;
	}
}

static void
check_x25(void)
{
	static const struct sockaddr_x25 c_x25 = {
		.sx25_family = AF_X25,
		.sx25_addr = { "0123456789abcdef" },
	};
	void *const x25_void = tail_memdup(&c_x25, sizeof(c_x25) + 1);
	long rc;

	rc = connect(-1, x25_void, sizeof(c_x25) - 1);
	pidns_print_leader();
	printf("connect(-1, {sa_family=AF_X25"
	       ", sa_data=\"0123456789abcde\"}, %zu) = %s\n",
	       sizeof(c_x25) - 1, sprintrc(rc));

	for (size_t i = 0; i < 2; i++) {
		rc = connect(-1, x25_void, sizeof(c_x25) + i);
		pidns_print_leader();
		printf("connect(-1, {sa_family=AF_X25"
		       ", sx25_addr={x25_addr=\"0123456789abcde\"...}"
		       "}, %zu) = %s\n",
		       sizeof(c_x25) + i, sprintrc(rc));
	}

	struct sockaddr_x25 *const x25 = x25_void;
	x25->sx25_addr.x25_addr[10] = '\0';
	rc = connect(-1, x25_void, sizeof(c_x25));
	pidns_print_leader();
	printf("connect(-1, {sa_family=AF_X25"
	       ", sx25_addr={x25_addr=\"0123456789\"}"
	       "}, %zu) = %s\n",
	       sizeof(c_x25), sprintrc(rc));
}

static void
check_nl(void)
{
	TAIL_ALLOC_OBJECT_VAR_PTR(struct sockaddr_nl, nl);
	nl->nl_family = AF_NETLINK;
	nl->nl_pid = 1234567890;
	nl->nl_groups = 0xfacefeed;
	unsigned int len = sizeof(*nl);

	connect(-1, (void *) nl, len - 1);
	pidns_print_leader();
	printf("connect(-1, {sa_family=AF_NETLINK, sa_data=\"\\377\\377"
	       BE_LE("I\\226\\2\\322", "\\322\\2\\226I")
	       BE_LE("\\372\\316\\376", "\\355\\376\\316")
	       "\"}, %u)" RVAL_EBADF,
	       len - 1);

	connect(-1, (void *) nl, len);
	pidns_print_leader();
	printf("connect(-1, {sa_family=AF_NETLINK, nl_pad=%#x, nl_pid=%d"
	       ", nl_groups=%#08x}, %u)" RVAL_EBADF,
	       nl->nl_pad, nl->nl_pid, nl->nl_groups, len);

	nl->nl_pad = 0;
	connect(-1, (void *) nl, len);
	pidns_print_leader();
	printf("connect(-1, {sa_family=AF_NETLINK, nl_pid=%d"
	       ", nl_groups=%#08x}, %u)" RVAL_EBADF,
	       nl->nl_pid, nl->nl_groups, len);

	nl = ((void *) nl) - 4;
	nl->nl_family = AF_NETLINK;
	nl->nl_pad = 0;
	nl->nl_pid = getpid();
	nl->nl_groups = 0xfacefeed;
	len = sizeof(*nl) + 4;
	connect(-1, (void *) nl, len);
	pidns_print_leader();
	printf("connect(-1, {sa_family=AF_NETLINK, nl_pid=%d%s"
	       ", nl_groups=%#08x}, %u)" RVAL_EBADF,
	       nl->nl_pid, pidns_pid2str(PT_TGID), nl->nl_groups, len);
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
	connect(-1, ll, len);
	pidns_print_leader();
	printf("connect(-1, {sa_family=AF_PACKET"
	       ", sll_protocol=htons(ETH_P_ALL)"
	       ", sll_ifindex=%u, sll_hatype=ARPHRD_ETHER"
	       ", sll_pkttype=PACKET_HOST, sll_halen=%u, sll_addr="
	       "[%#02x, %#02x, %#02x, %#02x, %#02x, %#02x, %#02x, %#02x]"
	       "}, %u)" RVAL_EBADF,
	       c_ll.sll_ifindex, c_ll.sll_halen,
	       c_ll.sll_addr[0], c_ll.sll_addr[1],
	       c_ll.sll_addr[2], c_ll.sll_addr[3],
	       c_ll.sll_addr[4], c_ll.sll_addr[5],
	       c_ll.sll_addr[6], c_ll.sll_addr[7],
	       len);

	((struct sockaddr_ll *) ll)->sll_halen++;
	connect(-1, ll, len);
	pidns_print_leader();
	printf("connect(-1, {sa_family=AF_PACKET"
	       ", sll_protocol=htons(ETH_P_ALL)"
	       ", sll_ifindex=%u, sll_hatype=ARPHRD_ETHER"
	       ", sll_pkttype=PACKET_HOST, sll_halen=%u, sll_addr="
	       "[%#02x, %#02x, %#02x, %#02x, %#02x, %#02x, %#02x, %#02x, ...]"
	       "}, %u)" RVAL_EBADF,
	       c_ll.sll_ifindex, c_ll.sll_halen + 1,
	       c_ll.sll_addr[0], c_ll.sll_addr[1],
	       c_ll.sll_addr[2], c_ll.sll_addr[3],
	       c_ll.sll_addr[4], c_ll.sll_addr[5],
	       c_ll.sll_addr[6], c_ll.sll_addr[7],
	       len);

	((struct sockaddr_ll *) ll)->sll_halen = 0;
	connect(-1, ll, len);
	pidns_print_leader();
	printf("connect(-1, {sa_family=AF_PACKET"
	       ", sll_protocol=htons(ETH_P_ALL)"
	       ", sll_ifindex=%u, sll_hatype=ARPHRD_ETHER"
	       ", sll_pkttype=PACKET_HOST, sll_halen=0}, %u)"
	       RVAL_EBADF, c_ll.sll_ifindex, len);

	((struct sockaddr_ll *) ll)->sll_ifindex = ifindex_lo();
	if (((struct sockaddr_ll *) ll)->sll_ifindex) {
		connect(-1, ll, len);
	pidns_print_leader();
		printf("connect(-1, {sa_family=AF_PACKET"
		       ", sll_protocol=htons(ETH_P_ALL)"
		       ", sll_ifindex=%s"
		       ", sll_hatype=ARPHRD_ETHER"
		       ", sll_pkttype=PACKET_HOST, sll_halen=0}, %u)"
		       RVAL_EBADF, IFINDEX_LO_STR, len);
	}
}

#ifdef HAVE_BLUETOOTH_BLUETOOTH_H
static void
check_hci(void)
{
	const unsigned short h_port = 12345;
	TAIL_ALLOC_OBJECT_VAR_PTR(struct sockaddr_hci, hci);
	hci->hci_family = AF_BLUETOOTH;
	hci->hci_dev = htobs(h_port);
# ifdef HAVE_STRUCT_SOCKADDR_HCI_HCI_CHANNEL
	hci->hci_channel = HCI_CHANNEL_RAW;
# endif
	unsigned int len = sizeof(*hci);

	connect(-1, (void *) hci, 4);
	pidns_print_leader();
	printf("connect(-1, {sa_family=AF_BLUETOOTH, hci_dev=htobs(%hu)"
	       "}, 4)" RVAL_EBADF,
	       h_port);

	connect(-1, (void *) hci, len);
	pidns_print_leader();
	printf("connect(-1, {sa_family=AF_BLUETOOTH, hci_dev=htobs(%hu)"
# ifdef HAVE_STRUCT_SOCKADDR_HCI_HCI_CHANNEL
	       ", hci_channel=HCI_CHANNEL_RAW"
# endif
	       "}, %u)" RVAL_EBADF,
	       h_port, len);
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
	connect(-1, sco, len);
	pidns_print_leader();
	printf("connect(-1, {sa_family=AF_BLUETOOTH"
	       ", sco_bdaddr=%02x:%02x:%02x:%02x:%02x:%02x"
	       "}, %u)" RVAL_EBADF,
	       c_sco.sco_bdaddr.b[0], c_sco.sco_bdaddr.b[1],
	       c_sco.sco_bdaddr.b[2], c_sco.sco_bdaddr.b[3],
	       c_sco.sco_bdaddr.b[4], c_sco.sco_bdaddr.b[5],
	       len);
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
	connect(-1, rc, len);
	pidns_print_leader();
	printf("connect(-1, {sa_family=AF_BLUETOOTH"
	       ", rc_bdaddr=%02x:%02x:%02x:%02x:%02x:%02x"
	       ", rc_channel=%u}, %u)" RVAL_EBADF,
	       c_rc.rc_bdaddr.b[0], c_rc.rc_bdaddr.b[1],
	       c_rc.rc_bdaddr.b[2], c_rc.rc_bdaddr.b[3],
	       c_rc.rc_bdaddr.b[4], c_rc.rc_bdaddr.b[5],
	       c_rc.rc_channel, len);
}

static void
check_l2(void)
{
	const unsigned short h_psm = 12345;
	const unsigned short h_cid = 13579;
	struct sockaddr_l2 c_l2 = {
		.l2_family = AF_BLUETOOTH,
		.l2_psm = htobs(h_psm),
		.l2_bdaddr.b = "abcdef",
		.l2_cid = htobs(h_cid),
# ifdef HAVE_STRUCT_SOCKADDR_L2_L2_BDADDR_TYPE
		.l2_bdaddr_type = 0xce,
# endif
	};
	void *l2 = tail_memdup(&c_l2, sizeof(c_l2));
	unsigned int len = sizeof(c_l2);

	connect(-1, l2, len);
	pidns_print_leader();
	printf("connect(-1, {sa_family=AF_BLUETOOTH"
	       ", l2_psm=htobs(L2CAP_PSM_DYN_START+%hu)"
	       ", l2_bdaddr=%02x:%02x:%02x:%02x:%02x:%02x"
	       ", l2_cid=htobs(L2CAP_CID_DYN_START+%hu)"
# ifdef HAVE_STRUCT_SOCKADDR_L2_L2_BDADDR_TYPE
	       ", l2_bdaddr_type=0xce /* BDADDR_??? */"
# endif
	       "}, %u)" RVAL_EBADF,
	       (short) (h_psm - 0x1001),
	       c_l2.l2_bdaddr.b[0], c_l2.l2_bdaddr.b[1],
	       c_l2.l2_bdaddr.b[2], c_l2.l2_bdaddr.b[3],
	       c_l2.l2_bdaddr.b[4], c_l2.l2_bdaddr.b[5],
	       (short) (h_cid - 0x40), len);

	c_l2.l2_psm = htobs(1);
	c_l2.l2_cid = htobs(1);
# ifdef HAVE_STRUCT_SOCKADDR_L2_L2_BDADDR_TYPE
	c_l2.l2_bdaddr_type = BDADDR_LE_RANDOM;
# endif
	memcpy(l2, &c_l2, sizeof(c_l2));
	connect(-1, l2, len);
	pidns_print_leader();
	printf("connect(-1, {sa_family=AF_BLUETOOTH"
	       ", l2_psm=htobs(L2CAP_PSM_SDP)"
	       ", l2_bdaddr=%02x:%02x:%02x:%02x:%02x:%02x"
	       ", l2_cid=htobs(L2CAP_CID_SIGNALING)"
# ifdef HAVE_STRUCT_SOCKADDR_L2_L2_BDADDR_TYPE
	       ", l2_bdaddr_type=BDADDR_LE_RANDOM"
# endif
	       "}, %u)" RVAL_EBADF,
	       c_l2.l2_bdaddr.b[0], c_l2.l2_bdaddr.b[1],
	       c_l2.l2_bdaddr.b[2], c_l2.l2_bdaddr.b[3],
	       c_l2.l2_bdaddr.b[4], c_l2.l2_bdaddr.b[5],
	       len);

	c_l2.l2_psm = htobs(0xbad);
	c_l2.l2_cid = htobs(8);
# ifdef HAVE_STRUCT_SOCKADDR_L2_L2_BDADDR_TYPE
	c_l2.l2_bdaddr_type = 3;
# endif
	memcpy(l2, &c_l2, sizeof(c_l2));
	connect(-1, l2, len);
	pidns_print_leader();
	printf("connect(-1, {sa_family=AF_BLUETOOTH"
	       ", l2_psm=htobs(0xbad /* L2CAP_PSM_??? */)"
	       ", l2_bdaddr=%02x:%02x:%02x:%02x:%02x:%02x"
	       ", l2_cid=htobs(0x8 /* L2CAP_CID_??? */)"
# ifdef HAVE_STRUCT_SOCKADDR_L2_L2_BDADDR_TYPE
	       ", l2_bdaddr_type=0x3 /* BDADDR_??? */"
# endif
	       "}, %u)" RVAL_EBADF,
	       c_l2.l2_bdaddr.b[0], c_l2.l2_bdaddr.b[1],
	       c_l2.l2_bdaddr.b[2], c_l2.l2_bdaddr.b[3],
	       c_l2.l2_bdaddr.b[4], c_l2.l2_bdaddr.b[5],
	       len);

	c_l2.l2_psm = htobs(0x10ff);
	c_l2.l2_cid = htobs(0xffff);
	memcpy(l2, &c_l2, 12);
	connect(-1, l2, 12);
	pidns_print_leader();
	printf("connect(-1, {sa_family=AF_BLUETOOTH"
	       ", l2_psm=htobs(L2CAP_PSM_AUTO_END)"
	       ", l2_bdaddr=%02x:%02x:%02x:%02x:%02x:%02x"
	       ", l2_cid=htobs(L2CAP_CID_DYN_END)"
	       "}, 12)" RVAL_EBADF,
	       c_l2.l2_bdaddr.b[0], c_l2.l2_bdaddr.b[1],
	       c_l2.l2_bdaddr.b[2], c_l2.l2_bdaddr.b[3],
	       c_l2.l2_bdaddr.b[4], c_l2.l2_bdaddr.b[5]);
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
	connect(-1, (void *) u.st, len);
	pidns_print_leader();
	printf("connect(-1, {sa_family=%#x /* AF_??? */, sa_data=\"%.*u\"}"
	       ", %u)" RVAL_EBADF, u.sa->sa_family,
	       (int) (sizeof(*u.st) - sizeof(u.sa->sa_family)), 0, len);

	u.sa->sa_family = 0;
	len = sizeof(u.sa->sa_family) + 1;
	connect(-1, (void *) u.st, len);
	pidns_print_leader();
	printf("connect(-1, {sa_family=AF_UNSPEC, sa_data=\"0\"}, %u)"
	       RVAL_EBADF, len);

	u.sa->sa_family = AF_BLUETOOTH;
	connect(-1, (void *) u.st, len);
	pidns_print_leader();
	printf("connect(-1, {sa_family=AF_BLUETOOTH, sa_data=\"0\"}, %u)"
	       RVAL_EBADF, len);
}

int
main(void)
{
	PIDNS_TEST_INIT;

	check_un();
	check_in();
	check_in6();
#ifdef HAVE_STRUCT_SOCKADDR_IPX
	check_ipx();
#endif
	check_ax25();
	check_x25();
	check_nl();
	check_ll();
#ifdef HAVE_BLUETOOTH_BLUETOOTH_H
	check_hci();
	check_sco();
	check_rc();
	check_l2();
#endif
	check_raw();

	pidns_print_leader();
	puts("+++ exited with 0 +++");
	return 0;
}
