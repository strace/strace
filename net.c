/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
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
 *
 *	$Id$
 */

#include "defs.h"

#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#if defined(LINUX)
#include <asm/types.h>
#if defined(__GLIBC__) && (__GLIBC__ >= 2) && (__GLIBC__ + __GLIBC_MINOR__ >= 3)
#  include <netipx/ipx.h>
#else
#  include <linux/ipx.h>
#endif
#endif /* LINUX */

#if defined(LINUX) && defined(MIPS)
#if defined(HAVE_LINUX_IN6_H)
#include <linux/in6.h>
#endif
#endif

#if defined(HAVE_SYS_UIO_H)
#include <sys/uio.h>
#endif

#if defined(HAVE_LINUX_NETLINK_H)
#include <linux/netlink.h>
#endif

#if defined(HAVE_LINUX_IF_PACKET_H)
#include <linux/if_packet.h>
#endif

#ifndef PF_UNSPEC
#define PF_UNSPEC AF_UNSPEC
#endif

#ifdef LINUX
/* Under Linux these are enums so we can't test for them with ifdef. */
#define IPPROTO_EGP IPPROTO_EGP
#define IPPROTO_PUP IPPROTO_PUP
#define IPPROTO_IDP IPPROTO_IDP
#define IPPROTO_IGMP IPPROTO_IGMP
#define IPPROTO_RAW IPPROTO_RAW
#define IPPROTO_MAX IPPROTO_MAX
#endif

static struct xlat domains[] = {
	{ PF_UNSPEC,	"PF_UNSPEC"	},
	{ PF_UNIX,	"PF_UNIX"	},
	{ PF_INET,	"PF_INET"	},
#ifdef PF_NETLINK
	{ PF_NETLINK,	"PF_NETLINK"	},
#endif
#ifdef PF_PACKET
	{ PF_PACKET,	"PF_PACKET"	},
#endif
#ifdef PF_INET6
	{ PF_INET6,	"PF_INET6"	},
#endif
#ifdef PF_ATMSVC
	{ PF_ATMSVC,	"PF_INET6"	},
#endif
#ifdef PF_INET6
	{ PF_INET6,	"PF_INET6"	},
#endif
#ifdef PF_LOCAL
	{ PF_LOCAL,	"PS_LOCAL"	},
#endif
#ifdef PF_ISO
	{ PF_ISO,	"PF_ISO"	},
#endif
#ifdef PF_AX25
	{ PF_AX25,	"PF_AX25"	},
#endif
#ifdef PF_IPX
	{ PF_IPX,	"PF_IPX"	},
#endif
#ifdef PF_APPLETALK
	{ PF_APPLETALK,	"PF_APPLETALK"	},
#endif
#ifdef PF_NETROM
	{ PF_NETROM,	"PF_NETROM"	},
#endif
#ifdef PF_BRIDGE
	{ PF_BRIDGE,	"PF_BRIDGE"	},
#endif
#ifdef PF_AAL5
	{ PF_AAL5,	"PF_AAL5"	},
#endif
#ifdef PF_X25
	{ PF_X25,	"PF_X25"	},
#endif
#ifdef PF_ROSE
	{ PF_ROSE,	"PF_ROSE"	},
#endif
#ifdef PF_DECNET
	{ PF_DECNET,	"PF_DECNET"	},
#endif
#ifdef PF_NETBEUI
	{ PF_NETBEUI,	"PF_NETBEUI"	},
#endif
#ifdef PF_IMPLINK
	{ PF_IMPLINK,	"PF_IMPLINK"	},
#endif
	{ 0,		NULL		},
};
static struct xlat addrfams[] = {
	{ AF_UNSPEC,	"AF_UNSPEC"	},
	{ AF_UNIX,	"AF_UNIX"	},
	{ AF_INET,	"AF_INET"	},
#ifdef AF_INET6
	{ AF_INET6,	"AF_INET6"	},
#endif
	{ AF_DECnet,	"AF_DECnet"	},
#ifdef PF_ATMSVC
	{ AF_ATMSVC,	"AF_ATMSVC"	},
#endif
#ifdef AF_PACKET
	{ AF_PACKET,	"AF_PACKET"	},
#endif
#ifdef AF_NETLINK
	{ AF_NETLINK,	"AF_NETLINK"	},
#endif
#ifdef AF_ISO
	{ AF_ISO,	"AF_ISO"	},
#endif
#ifdef AF_IMPLINK
	{ AF_IMPLINK,	"AF_IMPLINK"	},
#endif
	{ 0,		NULL		},
};
static struct xlat socktypes[] = {
	{ SOCK_STREAM,	"SOCK_STREAM"	},
	{ SOCK_DGRAM,	"SOCK_DGRAM"	},
#ifdef SOCK_RAW
	{ SOCK_RAW,	"SOCK_RAW"	},
#endif
#ifdef SOCK_SEQPACKET
	{ SOCK_SEQPACKET,"SOCK_SEQPACKET"},
#endif
#ifdef SOCK_RDM
	{ SOCK_RDM,	"SOCK_RDM"	},
#endif
#ifdef SOCK_PACKET
	{ SOCK_PACKET,	"SOCK_PACKET"	},
#endif
	{ 0,		NULL		},
};
static struct xlat protocols[] = {
	{ IPPROTO_IP,	"IPPROTO_IP"	},
	{ IPPROTO_ICMP,	"IPPROTO_ICMP"	},
	{ IPPROTO_TCP,	"IPPROTO_TCP"	},
	{ IPPROTO_UDP,	"IPPROTO_UDP"	},
#ifdef IPPROTO_GGP
	{ IPPROTO_GGP,	"IPPROTO_GGP"	},
#endif
#ifdef IPPROTO_EGP
	{ IPPROTO_EGP,	"IPPROTO_EGP"	},
#endif
#ifdef IPPROTO_PUP
	{ IPPROTO_PUP,	"IPPROTO_PUP"	},
#endif
#ifdef IPPROTO_IDP
	{ IPPROTO_IDP,	"IPPROTO_IDP"	},
#endif
#ifdef IPPROTO_IPV6
	{ IPPROTO_IPV6,	"IPPROTO_IPV6"	},
#endif
#ifdef IPPROTO_ICMPV6
	{ IPPROTO_ICMPV6,"IPPROTO_ICMPV6"},
#endif
#ifdef IPPROTO_IGMP
	{ IPPROTO_IGMP,	"IPPROTO_IGMP"	},
#endif
#ifdef IPPROTO_HELLO
	{ IPPROTO_HELLO,"IPPROTO_HELLO"	},
#endif
#ifdef IPPROTO_ND
	{ IPPROTO_ND,	"IPPROTO_ND"	},
#endif
#ifdef IPPROTO_RAW
	{ IPPROTO_RAW,	"IPPROTO_RAW"	},
#endif
#ifdef IPPROTO_MAX
	{ IPPROTO_MAX,	"IPPROTO_MAX"	},
#endif
#ifdef IPPROTO_IPIP
	{ IPPROTO_IPIP,	"IPPROTO_IPIP"	},
#endif
	{ 0,		NULL		},
};
static struct xlat msg_flags[] = {
	{ MSG_OOB,	"MSG_OOB"	},
#ifdef MSG_DONTROUTE
	{ MSG_DONTROUTE,"MSG_DONTROUTE"	},
#endif
#ifdef MSG_PEEK
	{ MSG_PEEK,	"MSG_PEEK"	},
#endif
#ifdef MSG_CTRUNC
	{ MSG_CTRUNC,	"MSG_CTRUNC"	},
#endif
#ifdef MSG_PROXY
	{ MSG_PROXY,	"MSG_PROXY"	},
#endif
#ifdef MSG_EOR
	{ MSG_EOR,	"MSG_EOR"	},
#endif
#ifdef MSG_WAITALL
	{ MSG_WAITALL,	"MSG_WAITALL"	},
#endif
#ifdef MSG_TRUNC
	{ MSG_TRUNC,	"MSG_TRUNC"	},
#endif
#ifdef MSG_CTRUNC
	{ MSG_CTRUNC,	"MSG_CTRUNC"	},
#endif
#ifdef MSG_ERRQUEUE
	{ MSG_ERRQUEUE,	"MSG_ERRQUEUE"	},
#endif
#ifdef MSG_DONTWAIT
	{ MSG_DONTWAIT,	"MSG_DONTWAIT"	},
#endif
#ifdef MSG_CONFIRM
	{ MSG_CONFIRM,	"MSG_CONFIRM"	},
#endif
#ifdef MSG_PROBE
	{ MSG_PROBE,	"MSG_PROBE"	},
#endif
	{ 0,		NULL		},
};

static struct xlat sockoptions[] = {
#ifdef SO_PEERCRED
	{ SO_PEERCRED,	"SO_PEERCRED"	},
#endif
#ifdef SO_PASSCRED
	{ SO_PASSCRED,	"SO_PASSCRED"	},
#endif
#ifdef SO_DEBUG
	{ SO_DEBUG,	"SO_DEBUG"	},
#endif
#ifdef SO_REUSEADDR
	{ SO_REUSEADDR,	"SO_REUSEADDR"	},
#endif
#ifdef SO_KEEPALIVE
	{ SO_KEEPALIVE,	"SO_KEEPALIVE"	},
#endif
#ifdef SO_DONTROUTE
	{ SO_DONTROUTE,	"SO_DONTROUTE"	},
#endif
#ifdef SO_BROADCAST
	{ SO_BROADCAST,	"SO_BROADCAST"	},
#endif
#ifdef SO_LINGER
	{ SO_LINGER,	"SO_LINGER"	},
#endif
#ifdef SO_OOBINLINE
	{ SO_OOBINLINE,	"SO_OOBINLINE"	},
#endif
#ifdef SO_TYPE
	{ SO_TYPE,	"SO_TYPE"	},
#endif
#ifdef SO_ERROR
	{ SO_ERROR,	"SO_ERROR"	},
#endif
#ifdef SO_SNDBUF
	{ SO_SNDBUF,	"SO_SNDBUF"	},
#endif
#ifdef SO_RCVBUF
	{ SO_RCVBUF,	"SO_RCVBUF"	},
#endif
#ifdef SO_NO_CHECK
	{ SO_NO_CHECK,	"SO_NO_CHECK"	},
#endif
#ifdef SO_PRIORITY
	{ SO_PRIORITY,	"SO_PRIORITY"	},
#endif
#ifdef SO_ACCEPTCONN
	{ SO_ACCEPTCONN,"SO_ACCEPTCONN"	},
#endif
#ifdef SO_USELOOPBACK
	{ SO_USELOOPBACK,"SO_USELOOPBACK"},
#endif
#ifdef SO_SNDLOWAT
	{ SO_SNDLOWAT,	"SO_SNDLOWAT"	},
#endif
#ifdef SO_RCVLOWAT
	{ SO_RCVLOWAT,	"SO_RCVLOWAT"	},
#endif
#ifdef SO_SNDTIMEO
	{ SO_SNDTIMEO,	"SO_SNDTIMEO"	},
#endif
#ifdef SO_RCVTIMEO
	{ SO_RCVTIMEO,	"SO_RCVTIMEO"	},
#endif
#ifdef SO_BSDCOMPAT
	{ SO_BSDCOMPAT,	"SO_BSDCOMPAT"	},
#endif
#ifdef SO_REUSEPORT
	{ SO_REUSEPORT,	"SO_REUSEPORT"	},
#endif
#ifdef SO_RCVLOWAT
	{ SO_RCVLOWAT, "SO_RCVLOWAT"	},
#endif
#ifdef SO_SNDLOWAT
	{ SO_SNDLOWAT, "SO_SNDLOWAT"	},
#endif
#ifdef SO_RCVTIMEO
	{ SO_RCVTIMEO, "SO_RCVTIMEO"	},
#endif
#ifdef SO_SNDTIMEO
	{ SO_SNDTIMEO, "SO_SNDTIMEO"	},
#endif
	{ 0,		NULL		},
};

#ifdef SOL_IP
static struct xlat sockipoptions[] = {
	{ IP_TOS,       "IP_TOS"        },
	{ IP_TTL,       "IP_TTL"        },
#if defined(IP_HDRINCL)
	{ IP_HDRINCL,   "IP_HDRINCL"    },
#endif
#if defined(IP_OPTIONS)
	{ IP_OPTIONS,   "IP_OPTIONS"    },
#endif
	{ IP_MULTICAST_IF,      "IP_MULTICAST_IF"       },
	{ IP_MULTICAST_TTL,     "IP_MULTICAST_TTL"      },
	{ IP_MULTICAST_LOOP,    "IP_MULTICAST_LOOP"     },
	{ IP_ADD_MEMBERSHIP,    "IP_ADD_MEMBERSHIP"     },
	{ IP_DROP_MEMBERSHIP,   "IP_DROP_MEMBERSHIP"    },
	{ 0,            NULL            },
};
#endif /* SOL_IP */

#ifdef SOL_IPX
static struct xlat sockipxoptions[] = {
	{ IPX_TYPE,     "IPX_TYPE"      },
	{ 0,            NULL            },
};
#endif /* SOL_IPX */

#ifdef SOL_TCP
static struct xlat socktcpoptions[] = {
	{ TCP_NODELAY,  "TCP_NODELAY"   },
	{ TCP_MAXSEG,   "TCP_MAXSEG"    },
	{ 0,            NULL            },
};
#endif /* SOL_TCP */

void
printsock(tcp, addr, addrlen)
struct tcb *tcp;
long addr;
int addrlen;
{
	union {
		char pad[128];
		struct sockaddr sa;
		struct sockaddr_in sin;
		struct sockaddr_un sau;
#ifdef HAVE_INET_NTOP
		struct sockaddr_in6 sa6;
#endif
#if defined(LINUX) && defined(AF_IPX)
		struct sockaddr_ipx sipx;
#endif
#ifdef AF_PACKET
		struct sockaddr_ll ll;
#endif
#ifdef AF_NETLINK
		struct sockaddr_nl nl;
#endif
	} addrbuf;
	char string_addr[100];

	if (addr == 0) {
		tprintf("NULL");
		return;
	}
	if (!verbose(tcp)) {
		tprintf("%#lx", addr);
		return;
	}
	if ((addrlen<2) || (addrlen>sizeof(addrbuf)))
		addrlen=sizeof(addrbuf);

	if (umoven(tcp, addr, addrlen, (char*)&addrbuf) < 0) {
		tprintf("{...}");
		return;
	}

	tprintf("{sin_family=");
	printxval(addrfams, addrbuf.sa.sa_family, "AF_???");
	tprintf(", ");

	switch (addrbuf.sa.sa_family) {
	case AF_UNIX:
		if (addrlen==2) {
			tprintf("<nil>");
		} else if (addrbuf.sau.sun_path[0]) {
			tprintf("path=\"%*.*s\"", addrlen-2, addrlen-2, addrbuf.sau.sun_path);
		} else {
			tprintf("path=@%*.*s", addrlen-3, addrlen-3, addrbuf.sau.sun_path+1);
		}
		break;
	case AF_INET:
		tprintf("sin_port=htons(%u), sin_addr=inet_addr(\"%s\")}",
			ntohs(addrbuf.sin.sin_port), inet_ntoa(addrbuf.sin.sin_addr));
		break;
#ifdef HAVE_INET_NTOP
	case AF_INET6:
		inet_ntop(AF_INET6, &addrbuf.sa6.sin6_addr, string_addr, sizeof(string_addr));
		tprintf("sin6_port=htons(%u), inet_pton(AF_INET6, \"%s\", &sin6_addr), sin6_flowinfo=htonl(%u)}",
			ntohs(addrbuf.sa6.sin6_port), string_addr, ntohl(addrbuf.sa6.sin6_flowinfo));
		break;	
#endif
#if defined(AF_IPX) && defined(linux)
	case AF_IPX:
		{
			int i;
			tprintf("{sipx_port=htons(%u), ",
					ntohs(addrbuf.sipx.sipx_port));
			/* Yes, I know, this does not look too
			 * strace-ish, but otherwise the IPX
			 * addresses just look monstrous...
			 * Anyways, feel free if you don't like
			 * this way.. :) 
			 */
			tprintf("%08lx:", (unsigned long)ntohl(addrbuf.sipx.sipx_network));
			for (i = 0; i<IPX_NODE_LEN; i++)
				tprintf("%02x", addrbuf.sipx.sipx_node[i]);
			tprintf("/[%02x]", addrbuf.sipx.sipx_type);
		}
		break;
#endif /* AF_IPX && linux */
#ifdef AF_PACKET
	case AF_PACKET:
		{
			int i;
			tprintf("proto=%#04x, if%d, pkttype=%d, addr(%d)={%d, ",
					ntohs(addrbuf.ll.sll_protocol),
					addrbuf.ll.sll_ifindex,
					addrbuf.ll.sll_pkttype,
					addrbuf.ll.sll_halen,
					addrbuf.ll.sll_hatype);
			for (i=0; i<addrbuf.ll.sll_addr[i]; i++) 
				tprintf("%02x", addrbuf.ll.sll_addr[i]);
		}
		break;

#endif /* AF_APACKET */
#ifdef AF_NETLINLK
	case AF_NETLINK:
		tprintf("pid=%d, groups=%08x", addrbuf.nl.nl_pid, addrbuf.nl.nl_groups);
		break;
#endif /* AF_NETLINK */
	/* AF_AX25 AF_APPLETALK AF_NETROM AF_BRIDGE AF_AAL5
	AF_X25 AF_ROSE etc. still need to be done */

	default:
		tprintf("{sa_family=%u, sa_data=", addrbuf.sa.sa_family);
		printstr(tcp, (long) &((struct sockaddr *) addr)->sa_data,
			sizeof addrbuf.sa.sa_data);
		break;
	}
	tprintf("}");
}

#if HAVE_SENDMSG

static void
printiovec(tcp, iovec, len)
struct tcb *tcp;
struct iovec *iovec;
long   len;
{
	struct iovec *iov;
	int i;

	iov = (struct iovec *) malloc(len * sizeof *iov);
	if (iov == NULL) {
		fprintf(stderr, "No memory");
		return;
	}
	if (umoven(tcp, (long)iovec,
				len * sizeof *iov, (char *) iov) < 0) {
		tprintf("%#lx", (unsigned long)iovec);
	} else {
		tprintf("[");
		for (i = 0; i < len; i++) {
			if (i)
				tprintf(", ");
			tprintf("{");
			printstr(tcp, (long) iov[i].iov_base,
					iov[i].iov_len);
			tprintf(", %lu}", (unsigned long)iov[i].iov_len);
		}
		tprintf("]");
	}
	free((char *) iov);
}

static void
printmsghdr(tcp, addr)
struct tcb *tcp;
long addr;
{
	struct msghdr msg;

	if (umove(tcp, addr, &msg) < 0) {
		tprintf("%#lx", addr);
		return;
	}
	tprintf("{msg_name(%d)=", msg.msg_namelen);
	printsock(tcp, (long)msg.msg_name, msg.msg_namelen);

	tprintf(", msg_iov(%lu)=", (unsigned long)msg.msg_iovlen);
	printiovec(tcp, msg.msg_iov, msg.msg_iovlen);

#ifdef HAVE_MSG_CONTROL
	tprintf(", msg_controllen=%lu", (unsigned long)msg.msg_controllen);
	if (msg.msg_controllen) 
		tprintf(", msg_control=%#lx, ", (unsigned long) msg.msg_control);
	tprintf(", msg_flags=");
	if (printflags(msg_flags, msg.msg_flags)==0)
		tprintf("0");
#else /* !HAVE_MSG_CONTROL */
	tprintf("msg_accrights=%#lx, msg_accrightslen=%u",
		(unsigned long) msg.msg_accrights, msg.msg_accrightslen);
#endif /* !HAVE_MSG_CONTROL */
	tprintf("}");
}

#endif /* HAVE_SENDMSG */

int
sys_socket(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printxval(domains, tcp->u_arg[0], "PF_???");
		tprintf(", ");
		printxval(socktypes, tcp->u_arg[1], "SOCK_???");
		tprintf(", ");
		switch (tcp->u_arg[0]) {
		case PF_INET:
			printxval(protocols, tcp->u_arg[2], "IPPROTO_???");
			break;
#ifdef PF_IPX
		case PF_IPX:
			/* BTW: I don't believe this.. */
			tprintf("[");
			printxval(domains, tcp->u_arg[2], "PF_???");
			tprintf("]");
			break;
#endif /* PF_IPX */
		default:
			tprintf("%lu", tcp->u_arg[2]);
			break;
		}
	}
	return 0;
}

int
sys_bind(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
		printsock(tcp, tcp->u_arg[1], tcp->u_arg[2]);
		tprintf(", %lu", tcp->u_arg[2]);
	}
	return 0;
}

int
sys_connect(tcp)
struct tcb *tcp;
{
	return sys_bind(tcp);
}

int
sys_listen(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld, %lu", tcp->u_arg[0], tcp->u_arg[1]);
	}
	return 0;
}

int
sys_accept(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
	} else if (!tcp->u_arg[2])
		tprintf("%#lx, NULL", tcp->u_arg[1]);
	else {
		if (tcp->u_arg[1] == 0 || syserror(tcp)) {
			tprintf("%#lx", tcp->u_arg[1]);
		} else {
			printsock(tcp, tcp->u_arg[1], tcp->u_arg[2]);
		}
		tprintf(", ");
		printnum(tcp, tcp->u_arg[2], "%lu");
	}
	return 0;
}

int
sys_send(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
		printstr(tcp, tcp->u_arg[1], tcp->u_arg[2]);
		tprintf(", %lu, ", tcp->u_arg[2]);
		/* flags */
		if (printflags(msg_flags, tcp->u_arg[3]) == 0)
			tprintf("0");
	}
	return 0;
}

int
sys_sendto(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
		printstr(tcp, tcp->u_arg[1], tcp->u_arg[2]);
		tprintf(", %lu, ", tcp->u_arg[2]);
		/* flags */
		if (printflags(msg_flags, tcp->u_arg[3]) == 0)
			tprintf("0");
		/* to address */
		tprintf(", ");
		printsock(tcp, tcp->u_arg[4], tcp->u_arg[5]);
		/* to length */
		tprintf(", %lu", tcp->u_arg[5]);
	}
	return 0;
}

#ifdef HAVE_SENDMSG

int
sys_sendmsg(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
		printmsghdr(tcp, tcp->u_arg[1]);
		/* flags */
		tprintf(", ");
		if (printflags(msg_flags, tcp->u_arg[2]) == 0)
			tprintf("0");
	}
	return 0;
}

#endif /* HAVE_SENDMSG */

int
sys_recv(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
	} else {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[1]);
		else
			printstr(tcp, tcp->u_arg[1], tcp->u_rval);

		tprintf(", %lu, ", tcp->u_arg[2]);
		if (printflags(msg_flags, tcp->u_arg[3]) == 0)
			tprintf("0");
	}
	return 0;
}

int
sys_recvfrom(tcp)
struct tcb *tcp;
{
	int fromlen;

	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
	} else {
		if (syserror(tcp)) {
			tprintf("%#lx, %lu, %lu, %#lx, %#lx",
				tcp->u_arg[1], tcp->u_arg[2], tcp->u_arg[3],
				tcp->u_arg[4], tcp->u_arg[5]);
			return 0;
		}
		/* buf */
		printstr(tcp, tcp->u_arg[1], tcp->u_rval);
		/* len */
		tprintf(", %lu, ", tcp->u_arg[2]);
		/* flags */
		if (printflags(msg_flags, tcp->u_arg[3]) == 0)
			tprintf("0");
		/* from address, len */
		if (!tcp->u_arg[4] || !tcp->u_arg[5]) {
			if (tcp->u_arg[4] == 0)
				tprintf(", NULL");
			else
				tprintf(", %#lx", tcp->u_arg[4]);
			if (tcp->u_arg[5] == 0)
				tprintf(", NULL");
			else
				tprintf(", %#lx", tcp->u_arg[5]);
			return 0;
		}
		if (umove(tcp, tcp->u_arg[5], &fromlen) < 0) {
			tprintf(", {...}, [?]");
			return 0;
		}
		tprintf(", ");
		printsock(tcp, tcp->u_arg[4], tcp->u_arg[5]);
		/* from length */
		tprintf(", [%u]", fromlen);
	}
	return 0;
}

#ifdef HAVE_SENDMSG

int
sys_recvmsg(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
	} else {
		if (syserror(tcp) || !verbose(tcp))
			tprintf("%#lx", tcp->u_arg[1]);
		else
			printmsghdr(tcp, tcp->u_arg[1]);
		/* flags */
		tprintf(", ");
		if (printflags(msg_flags, tcp->u_arg[2]) == 0)
			tprintf("0");
	}
	return 0;
}

#endif /* HAVE_SENDMSG */

int
sys_shutdown(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld, %ld", tcp->u_arg[0], tcp->u_arg[1]);
		switch (tcp->u_arg[1]) {
		case 0:
			tprintf("%s", " /* receive */");
			break;
		case 1:
			tprintf("%s", " /* send */");
			break;
		case 2:
			tprintf("%s", " /* send and receive */");
			break;
		}
	}
	return 0;
}

int
sys_getsockname(tcp)
struct tcb *tcp;
{
	return sys_accept(tcp);
}

int
sys_getpeername(tcp)
struct tcb *tcp;
{
	return sys_accept(tcp);
}

int
sys_pipe(tcp)
struct tcb *tcp;
{

#if defined(LINUX) && !defined(SPARC)
	int fds[2];

	if (exiting(tcp)) {
		if (syserror(tcp)) {
			tprintf("%#lx", tcp->u_arg[0]);
			return 0;
		}
		if (umoven(tcp, tcp->u_arg[0], sizeof fds, (char *) fds) < 0)
			tprintf("[...]");
		else
			tprintf("[%u, %u]", fds[0], fds[1]);
	}
#elif defined(SPARC) || defined(SVR4)
	if (exiting(tcp))
		tprintf("[%lu, %lu]", tcp->u_rval, getrval2(tcp));
#endif
	return 0;
}

int
sys_socketpair(tcp)
struct tcb *tcp;
{
#ifdef LINUX
	int fds[2];
#endif

	if (entering(tcp)) {
		printxval(domains, tcp->u_arg[0], "PF_???");
		tprintf(", ");
		printxval(socktypes, tcp->u_arg[1], "SOCK_???");
		tprintf(", ");
		switch (tcp->u_arg[0]) {
		case PF_INET:
			printxval(protocols, tcp->u_arg[2], "IPPROTO_???");
			break;
#ifdef PF_IPX
		case PF_IPX:
			/* BTW: I don't believe this.. */
			tprintf("[");
			printxval(domains, tcp->u_arg[2], "PF_???");
			tprintf("]");
			break;
#endif /* PF_IPX */
		default:	
			tprintf("%lu", tcp->u_arg[2]);
			break;
		}
	} else {
		if (syserror(tcp)) {
			tprintf(", %#lx", tcp->u_arg[3]);
			return 0;
		}
#ifdef LINUX
		if (umoven(tcp, tcp->u_arg[3], sizeof fds, (char *) fds) < 0)
			tprintf(", [...]");
		else
			tprintf(", [%u, %u]", fds[0], fds[1]);
#endif /* LINUX */
#ifdef SUNOS4
		tprintf(", [%lu, %lu]", tcp->u_rval, getrval2(tcp));
#endif /* SUNOS4 */
#ifdef SVR4
		tprintf(", [%lu, %lu]", tcp->u_rval, getrval2(tcp));
#endif /* SVR4 */
	}
	return 0;
}

int
sys_getsockopt(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
		switch (tcp->u_arg[1]) {
		case SOL_SOCKET:
			tprintf("SOL_SOCKET, ");
			printxval(sockoptions, tcp->u_arg[2], "SO_???");
			tprintf(", ");
			break;
#ifdef SOL_IP
		case SOL_IP:
			tprintf("SOL_IP, ");
			printxval(sockipoptions, tcp->u_arg[2], "IP_???");
			tprintf(", ");
			break;
#endif
#ifdef SOL_IPX
		case SOL_IPX:
			tprintf("SOL_IPX, ");
			printxval(sockipxoptions, tcp->u_arg[2], "IPX_???");
			tprintf(", ");
			break;
#endif
#ifdef SOL_TCP
		case SOL_TCP:
			tprintf("SOL_TCP, ");
			printxval(socktcpoptions, tcp->u_arg[2], "TCP_???");
			tprintf(", ");
			break;
#endif

		/* SOL_AX25 SOL_ROSE SOL_ATALK SOL_NETROM SOL_UDP SOL_DECNET SOL_X25
		 * etc. still need work */
		default: 
			/* XXX - should know socket family here */
			printxval(protocols, tcp->u_arg[1], "IPPROTO_???");
			tprintf(", %lu, ", tcp->u_arg[2]);
			break;
		}
	} else {
		if (syserror(tcp)) {
			tprintf("%#lx, %#lx",
				tcp->u_arg[3], tcp->u_arg[4]);
			return 0;
		}
		printnum(tcp, tcp->u_arg[3], "%ld");
		tprintf(", ");
		printnum(tcp, tcp->u_arg[4], "%ld");
	}
	return 0;
}

int
sys_setsockopt(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
		switch (tcp->u_arg[1]) {
		case SOL_SOCKET:
			tprintf("SOL_SOCKET, ");
			printxval(sockoptions, tcp->u_arg[2], "SO_???");
			tprintf(", ");
			break;
#ifdef SOL_IP
		case SOL_IP:
			tprintf("SOL_IP, ");
			printxval(sockipoptions, tcp->u_arg[2], "IP_???");
			tprintf(", ");
			break;
#endif
#ifdef SOL_IPX
		case SOL_IPX:
			tprintf("SOL_IPX, ");
			printxval(sockipxoptions, tcp->u_arg[2], "IPX_???");
			tprintf(", ");
			break;
#endif
#ifdef SOL_TCP
		case SOL_TCP:
			tprintf("SOL_TCP, ");
			printxval(socktcpoptions, tcp->u_arg[2], "TCP_???");
			tprintf(", ");
			break;
#endif

		/* SOL_AX25 SOL_ATALK SOL_NETROM SOL_UDP SOL_DECNET SOL_X25 
		 * etc. still need work  */
		default:
			/* XXX - should know socket family here */
			printxval(protocols, tcp->u_arg[1], "IPPROTO_???");
			tprintf("%lu, ", tcp->u_arg[2]);
			break;
		}
		printnum(tcp, tcp->u_arg[3], "%ld");
		tprintf(", %lu", tcp->u_arg[4]);
	}
	return 0;
}
