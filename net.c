/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-2000 Wichert Akkerman <wichert@cistron.nl>
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

#if defined(HAVE_SIN6_SCOPE_ID_LINUX)
#define in6_addr in6_addr_libc
#define ipv6_mreq ipv6_mreq_libc
#define sockaddr_in6 sockaddr_in6_libc
#endif

#include <netinet/in.h>
#ifdef HAVE_NETINET_TCP_H
#include <netinet/tcp.h>
#endif
#ifdef HAVE_NETINET_UDP_H
#include <netinet/udp.h>
#endif
#include <arpa/inet.h>
#include <net/if.h>
#if defined(LINUX)
#include <asm/types.h>
#if defined(__GLIBC__) && (__GLIBC__ >= 2) && (__GLIBC__ + __GLIBC_MINOR__ >= 3)
#  include <netipx/ipx.h>
#else
#  include <linux/ipx.h>
#endif
#endif /* LINUX */

#if defined (__GLIBC__) && (((__GLIBC__ < 2) || (__GLIBC__ == 2 && __GLIBC_MINOR__ < 1)) || defined(HAVE_SIN6_SCOPE_ID_LINUX))
#if defined(HAVE_LINUX_IN6_H)
#if defined(HAVE_SIN6_SCOPE_ID_LINUX)
#undef in6_addr
#undef ipv6_mreq
#undef sockaddr_in6
#define in6_addr in6_addr_kernel
#define ipv6_mreq ipv6_mreq_kernel
#define sockaddr_in6 sockaddr_in6_kernel
#endif
#include <linux/in6.h>
#if defined(HAVE_SIN6_SCOPE_ID_LINUX)
#undef in6_addr
#undef ipv6_mreq
#undef sockaddr_in6
#define in6_addr in6_addr_libc
#define ipv6_mreq ipv6_mreq_libc
#define sockaddr_in6 sockaddr_in6_kernel
#endif
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

#if defined(HAVE_LINUX_ICMP_H)
#include <linux/icmp.h>
#endif

#ifndef PF_UNSPEC
#define PF_UNSPEC AF_UNSPEC
#endif

#if UNIXWARE >= 7
#define HAVE_SENDMSG		1		/* HACK - *FIXME* */
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
static struct xlat socketlayers[] = {
	{ SOL_IP,	"SOL_IP"	},
#if defined(SOL_ICMP)
	{ SOL_ICMP,	"SOL_ICMP"	},
#endif
	{ SOL_TCP,	"SOL_TCP"	},
	{ SOL_UDP,	"SOL_UDP"	},
#if defined(SOL_IPV6)
	{ SOL_IPV6,	"SOL_IPV6"	},
#endif
#if defined(SOL_ICMPV6)
	{ SOL_ICMPV6,	"SOL_ICMPV6"	},
#endif
#if defined(SOL_RAW)
	{ SOL_RAW,	"SOL_RAW"	},
#endif
#if defined(SOL_IPX)
	{ SOL_IPX,	"SOL_IPX"	},
#endif
#if defined(SOL_IPX)
	{ SOL_IPX,	"SOL_IPX"	},
#endif
#if defined(SOL_AX25)
	{ SOL_AX25,	"SOL_AX25"	},
#endif
#if defined(SOL_ATALK)
	{ SOL_ATALK,	"SOL_ATALK"	},
#endif
#if defined(SOL_NETROM)
	{ SOL_NETROM,	"SOL_NETROM"	},
#endif
#if defined(SOL_ROSE)
	{ SOL_ROSE,	"SOL_ROSE"	},
#endif
#if defined(SOL_DECNET)
	{ SOL_DECNET,	"SOL_DECNET"	},
#endif
#if defined(SOL_X25)
	{ SOL_X25,	"SOL_X25"	},
#endif
#if defined(SOL_PACKET)
	{ SOL_PACKET,	"SOL_PACKET"	},
#endif
#if defined(SOL_ATM)
	{ SOL_ATM,	"SOL_ATM"	},
#endif
#if defined(SOL_AAL)
	{ SOL_AAL,	"SOL_AAL"	},
#endif
#if defined(SOL_IRDA)
	{ SOL_IRDA,	"SOL_IRDA"	},
#endif
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
	{ IP_TOS,		"IP_TOS"		},
	{ IP_TTL,		"IP_TTL"		},
#if defined(IP_HDRINCL)
	{ IP_HDRINCL,		"IP_HDRINCL"		},
#endif
#if defined(IP_OPTIONS)
	{ IP_OPTIONS,		"IP_OPTIONS"		},
#endif
	{ IP_ROUTER_ALERT,	"IP_ROUTER_ALERT"	},
#if defined(IP_RECVOPTIONS)
	{ IP_RECVOPTIONS,	"IP_RECVOPTIONS"	},
#endif
	{ IP_RETOPTS,		"IP_RETOPTS"		},
	{ IP_PKTINFO,		"IP_PKTINFO"		},
	{ IP_PKTOPTIONS,	"IP_PKTOPTIONS"	},
	{ IP_MTU_DISCOVER,	"IP_MTU_DISCOVER"	},
	{ IP_MTU_DISCOVER,	"IP_MTU_DISCOVER"	},
	{ IP_RECVERR,		"IP_RECVERR"		},
	{ IP_RECVTTL,		"IP_RECRECVTTL"		},
	{ IP_RECVTOS,		"IP_RECRECVTOS"		},
#if defined(IP_MTU)
	{ IP_MTU,		"IP_MTU"		},
#endif
	{ IP_MULTICAST_IF,	"IP_MULTICAST_IF"	},
	{ IP_MULTICAST_TTL,	"IP_MULTICAST_TTL"	},
	{ IP_MULTICAST_LOOP,	"IP_MULTICAST_LOOP"	},
	{ IP_ADD_MEMBERSHIP,	"IP_ADD_MEMBERSHIP"	},
	{ IP_DROP_MEMBERSHIP,	"IP_DROP_MEMBERSHIP"	},
	{ 0,			NULL			},
};
#endif /* SOL_IP */

#ifdef SOL_IPX
static struct xlat sockipxoptions[] = {
	{ IPX_TYPE,     "IPX_TYPE"      },
	{ 0,            NULL            },
};
#endif /* SOL_IPX */

#ifdef SOL_RAW
static struct xlat sockrawoptions[] = {
#if defined(ICMP_FILTER)
	{ ICMP_FILTER,		"ICMP_FILTER"	},
#endif
	{ 0,			NULL		},
};
#endif /* SOL_RAW */

#ifdef SOL_PACKET
static struct xlat sockpacketoptions[] = {
	{ PACKET_ADD_MEMBERSHIP,	"PACKET_ADD_MEMBERSHIP"	},
	{ PACKET_DROP_MEMBERSHIP,	"PACKET_DROP_MEMBERSHIP"},
#if defined(PACKET_RECV_OUTPUT)
	{ PACKET_RECV_OUTPUT,		"PACKET_RECV_OUTPUT"	},
#endif
#if defined(PACKET_RX_RING)
	{ PACKET_RX_RING,		"PACKET_RX_RING"	},
#endif
#if defined(PACKET_STATISTICS)
	{ PACKET_STATISTICS,		"PACKET_STATISTICS"	},
#endif
	{ 0,				NULL			},
};
#endif /* SOL_PACKET */

#ifdef SOL_TCP
static struct xlat socktcpoptions[] = {
	{ TCP_NODELAY,	"TCP_NODELAY"	},
	{ TCP_MAXSEG,	"TCP_MAXSEG"	},
#if defined(TCP_CORK)
	{ TCP_CORK,	"TCP_CORK"	},
#endif
	{ 0,		NULL		},
};
#endif /* SOL_TCP */

#ifdef SOL_RAW
static struct xlat icmpfilterflags[] = {
#if defined(ICMP_ECHOREPLY)
	{ (1<<ICMP_ECHOREPLY),		"ICMP_ECHOREPLY"	},
#endif
#if defined(ICMP_DEST_UNREACH)
	{ (1<<ICMP_DEST_UNREACH),	"ICMP_DEST_UNREACH"	},
#endif
#if defined(ICMP_SOURCE_QUENCH)
	{ (1<<ICMP_SOURCE_QUENCH),	"ICMP_SOURCE_QUENCH"	},
#endif
#if defined(ICMP_REDIRECT)
	{ (1<<ICMP_REDIRECT),		"ICMP_REDIRECT"		},
#endif
#if defined(ICMP_ECHO)
	{ (1<<ICMP_ECHO),		"ICMP_ECHO"		},
#endif
#if defined(ICMP_TIME_EXCEEDED)
	{ (1<<ICMP_TIME_EXCEEDED),	"ICMP_TIME_EXCEEDED"	},
#endif
#if defined(ICMP_PARAMETERPROB)
	{ (1<<ICMP_PARAMETERPROB),	"ICMP_PARAMETERPROB"	},
#endif
#if defined(ICMP_TIMESTAMP)
	{ (1<<ICMP_TIMESTAMP),		"ICMP_TIMESTAMP"	},
#endif
#if defined(ICMP_TIMESTAMPREPLY)
	{ (1<<ICMP_TIMESTAMPREPLY),	"ICMP_TIMESTAMPREPLY"	},
#endif
#if defined(ICMP_INFO_REQUEST)
	{ (1<<ICMP_INFO_REQUEST),	"ICMP_INFO_REQUEST"	},
#endif
#if defined(ICMP_INFO_REPLY)
	{ (1<<ICMP_INFO_REPLY),		"ICMP_INFO_REPLY"	},
#endif
#if defined(ICMP_ADDRESS)
	{ (1<<ICMP_ADDRESS),		"ICMP_ADDRESS"		},
#endif
#if defined(ICMP_ADDRESSREPLY)
	{ (1<<ICMP_ADDRESSREPLY),	"ICMP_ADDRESSREPLY"	},
#endif
	{ 0,				NULL			},
};
#endif /* SOL_RAW */


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
		tprintf("sin6_port=htons(%u), inet_pton(AF_INET6, \"%s\", &sin6_addr), sin6_flowinfo=%u",
				ntohs(addrbuf.sa6.sin6_port), string_addr,
				addrbuf.sa6.sin6_flowinfo);
#ifdef HAVE_SIN6_SCOPE_ID
		{
#if defined(HAVE_IF_INDEXTONAME) && defined(IN6_IS_ADDR_LINKLOCAL) && defined(IN6_IS_ADDR_MC_LINKLOCAL)
		    int numericscope = 0;
		    if (IN6_IS_ADDR_LINKLOCAL (&addrbuf.sa6.sin6_addr)
			    || IN6_IS_ADDR_MC_LINKLOCAL (&addrbuf.sa6.sin6_addr)) {
			char scopebuf[IFNAMSIZ + 1];
			
			if (if_indextoname (addrbuf.sa6.sin6_scope_id, scopebuf) == NULL)
			    numericscope++;
			else
			    tprintf(", sin6_scope_id=if_nametoindex(\"%s\")", scopebuf);
		    } else
			numericscope++;
		    
		    if (numericscope)
#endif
			tprintf(", sin6_scope_id=%u", addrbuf.sa6.sin6_scope_id);
		}
#endif
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
#elif defined(SPARC) || defined(SVR4) || defined(FREEBSD)
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
#if defined(SUNOS4) || defined(SVR4) || defined(FREEBSD)
		tprintf(", [%lu, %lu]", tcp->u_rval, getrval2(tcp));
#endif /* SUNOS4 || SVR4 || FREEBSD */
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
#ifdef SOL_PACKET
		case SOL_PACKET:
			tprintf("SOL_PACKET, ");
			printxval(sockpacketoptions, tcp->u_arg[2], "PACKET_???");
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
			printxval(socketlayers, tcp->u_arg[1], "SOL_???");
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

#if defined(ICMP_FILTER)
static void printicmpfilter(tcp, addr)
struct tcb *tcp;
long addr;
{
	struct icmp_filter	filter;

	if (!addr) {
		tprintf("NULL");
		return;
	}
	if (syserror(tcp) || !verbose(tcp)) {
		tprintf("%#lx", addr);
		return;
	}
	if (umove(tcp, addr, &filter) < 0) {
		tprintf("{...}");
		return;
	}

	tprintf("~(");
	if (printflags(icmpfilterflags, ~filter.data) == 0)
		tprintf("0");
	tprintf(")");
}
#endif /* ICMP_FILTER */

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
			printnum(tcp, tcp->u_arg[3], "%ld");
			tprintf(", %lu", tcp->u_arg[4]);
			break;
#ifdef SOL_IP
		case SOL_IP:
			tprintf("SOL_IP, ");
			printxval(sockipoptions, tcp->u_arg[2], "IP_???");
			tprintf(", ");
			printnum(tcp, tcp->u_arg[3], "%ld");
			tprintf(", %lu", tcp->u_arg[4]);
			break;
#endif
#ifdef SOL_IPX
		case SOL_IPX:
			tprintf("SOL_IPX, ");
			printxval(sockipxoptions, tcp->u_arg[2], "IPX_???");
			tprintf(", ");
			printnum(tcp, tcp->u_arg[3], "%ld");
			tprintf(", %lu", tcp->u_arg[4]);
			break;
#endif
#ifdef SOL_PACKET
		case SOL_PACKET:
			tprintf("SOL_PACKET, ");
			printxval(sockpacketoptions, tcp->u_arg[2], "PACKET_???");
			tprintf(", ");
			/* TODO: decode packate_mreq for PACKET_*_MEMBERSHIP */
			printnum(tcp, tcp->u_arg[3], "%ld");
			tprintf(", %lu", tcp->u_arg[4]);
			break;
#endif
#ifdef SOL_TCP
		case SOL_TCP:
			tprintf("SOL_TCP, ");
			printxval(socktcpoptions, tcp->u_arg[2], "TCP_???");
			tprintf(", ");
			printnum(tcp, tcp->u_arg[3], "%ld");
			tprintf(", %lu", tcp->u_arg[4]);
			break;
#endif
#ifdef SOL_RAW
		case SOL_RAW:
			tprintf("SOL_RAW, ");
			printxval(sockrawoptions, tcp->u_arg[2], "RAW_???");
			tprintf(", ");
			switch (tcp->u_arg[2]) {
#if defined(ICMP_FILTER)
				case ICMP_FILTER:
					printicmpfilter(tcp, tcp->u_arg[3]);
					break;
#endif
				default:
					printnum(tcp, tcp->u_arg[3], "%ld");
					break;
			}
			tprintf(", %lu", tcp->u_arg[4]);
			break;
#endif

		/* SOL_AX25 SOL_ATALK SOL_NETROM SOL_UDP SOL_DECNET SOL_X25 
		 * etc. still need work  */
		default:
			/* XXX - should know socket family here */
			printxval(socketlayers, tcp->u_arg[1], "IPPROTO_???");
			tprintf(", %lu, ", tcp->u_arg[2]);
			printnum(tcp, tcp->u_arg[3], "%ld");
			tprintf(", %lu", tcp->u_arg[4]);
			break;
		}
	}
	return 0;
}

#if UNIXWARE >= 7

static struct xlat sock_version[] = {
	{ __NETLIB_UW211_SVR4,	"UW211_SVR4" },
	{ __NETLIB_UW211_XPG4,	"UW211_XPG4" },
	{ __NETLIB_GEMINI_SVR4,	"GEMINI_SVR4" },
	{ __NETLIB_GEMINI_XPG4,	"GEMINI_XPG4" },
	{ __NETLIB_FP1_SVR4,	"FP1_SVR4" },
	{ __NETLIB_FP1_XPG4,	"FP1_XPG4" },
	{ 0,            NULL            },
};


int
netlib_call(tcp, func)
struct tcb *tcp;
int (*func) ();
{
	if (entering(tcp)) {
		int i;
		printxval (sock_version, tcp->u_arg[0], "__NETLIB_???");
		tprintf(", ");
		--tcp->u_nargs;
		for (i = 0; i < tcp->u_nargs; i++)
			tcp->u_arg[i] = tcp->u_arg[i + 1];
		return func (tcp);
		
	}

	return func (tcp);
}

int
sys_xsocket(tcp)
struct tcb *tcp;
{
	return netlib_call (tcp, sys_socket);
}

int
sys_xsocketpair(tcp)
struct tcb *tcp;
{
	return netlib_call (tcp, sys_socketpair);
}

int
sys_xbind(tcp)
struct tcb *tcp;
{
	return netlib_call (tcp, sys_bind);
}

int
sys_xconnect(tcp)
struct tcb *tcp;
{
	return netlib_call (tcp, sys_connect);
}

int
sys_xlisten(tcp)
struct tcb *tcp;
{
	return netlib_call (tcp, sys_listen);
}

int
sys_xaccept(tcp)
struct tcb *tcp;
{
	return netlib_call (tcp, sys_accept);
}

int
sys_xsendmsg(tcp)
struct tcb *tcp;
{
	return netlib_call (tcp, sys_sendmsg);
}

int
sys_xrecvmsg(tcp)
struct tcb *tcp;
{
	return netlib_call (tcp, sys_recvmsg);
}

int
sys_xgetsockaddr(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printxval (sock_version, tcp->u_arg[0], "__NETLIB_???");
		tprintf(", ");
		if (tcp->u_arg[1] == 0) {
			tprintf ("LOCALNAME, ");
		}
		else if (tcp->u_arg[1] == 1) {
			tprintf ("REMOTENAME, ");
		}
		else {
			tprintf ("%ld, ", tcp->u_arg [1]);
		}
		tprintf ("%ld, ", tcp->u_arg [2]);
	} 
	else {
		if (tcp->u_arg[3] == 0 || syserror(tcp)) {
			tprintf("%#lx", tcp->u_arg[3]);
		} else {
			printsock(tcp, tcp->u_arg[3], tcp->u_arg[4]);
		}
		tprintf(", ");
		printnum(tcp, tcp->u_arg[4], "%lu");
	}

	return 0;

}

#if 0

int
sys_xsetsockaddr(tcp)
struct tcb *tcp;
{
	return netlib_call (tcp, sys_setsockaddr);
}

#endif

int
sys_xgetsockopt(tcp)
struct tcb *tcp;
{
	return netlib_call (tcp, sys_getsockopt);
}

int
sys_xsetsockopt(tcp)
struct tcb *tcp;
{
	return netlib_call (tcp, sys_setsockopt);
}

int
sys_xshutdown(tcp)
struct tcb *tcp;
{
	return netlib_call (tcp, sys_shutdown);
}

#endif
