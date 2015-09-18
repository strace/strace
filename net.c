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
#if defined(__GLIBC__)
# include <netipx/ipx.h>
#else
# include <linux/ipx.h>
#endif

#if defined(HAVE_LINUX_IP_VS_H)
# include <linux/ip_vs.h>
#endif
#if defined(HAVE_LINUX_NETLINK_H)
# include <linux/netlink.h>
#endif
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
#if defined(HAVE_LINUX_IF_PACKET_H)
# include <linux/if_packet.h>
#endif
#if defined(HAVE_LINUX_ICMP_H)
# include <linux/icmp.h>
#endif
#ifdef HAVE_BLUETOOTH_BLUETOOTH_H
# include <bluetooth/bluetooth.h>
# include <bluetooth/hci.h>
# include <bluetooth/l2cap.h>
# include <bluetooth/rfcomm.h>
# include <bluetooth/sco.h>
#endif
#ifndef PF_UNSPEC
# define PF_UNSPEC AF_UNSPEC
#endif

#include "xlat/domains.h"
#include "xlat/addrfams.h"
#include "xlat/socktypes.h"
#include "xlat/sock_type_flags.h"
#ifndef SOCK_TYPE_MASK
# define SOCK_TYPE_MASK 0xf
#endif

#include "xlat/socketlayers.h"

#include "xlat/inet_protocols.h"

#ifdef PF_NETLINK
# if !defined NETLINK_SOCK_DIAG && defined NETLINK_INET_DIAG
#  define NETLINK_SOCK_DIAG NETLINK_INET_DIAG
# endif
# include "xlat/netlink_protocols.h"
#endif

#if defined(HAVE_BLUETOOTH_BLUETOOTH_H)
# include "xlat/bt_protocols.h"
#endif

#include "xlat/msg_flags.h"

#if defined(AF_PACKET) /* from e.g. linux/if_packet.h */
# include "xlat/af_packet_types.h"
#endif

static void
print_ifindex(unsigned int ifindex)
{
#ifdef HAVE_IF_INDEXTONAME
	char buf[IFNAMSIZ + 1];

	if (if_indextoname(ifindex, buf)) {
		tprints("if_nametoindex(");
		print_quoted_string(buf, sizeof(buf), QUOTE_0_TERMINATED);
		tprints(")");
		return;
	}
#endif
	tprintf("%u", ifindex);
}

void
printsock(struct tcb *tcp, long addr, int addrlen)
{
	union {
		char pad[128];
		struct sockaddr sa;
		struct sockaddr_in sin;
		struct sockaddr_un sau;
#ifdef HAVE_INET_NTOP
		struct sockaddr_in6 sa6;
#endif
#if defined(AF_IPX)
		struct sockaddr_ipx sipx;
#endif
#ifdef AF_PACKET
		struct sockaddr_ll ll;
#endif
#ifdef AF_NETLINK
		struct sockaddr_nl nl;
#endif
#ifdef HAVE_BLUETOOTH_BLUETOOTH_H
		struct sockaddr_hci hci;
		struct sockaddr_l2 l2;
		struct sockaddr_rc rc;
		struct sockaddr_sco sco;
#endif
	} addrbuf;
	char string_addr[100];

	if (addrlen < 2) {
		printaddr(addr);
		return;
	}

	if (addrlen > (int) sizeof(addrbuf))
		addrlen = sizeof(addrbuf);

	memset(&addrbuf, 0, sizeof(addrbuf));
	if (umoven_or_printaddr(tcp, addr, addrlen, addrbuf.pad))
		return;
	addrbuf.pad[sizeof(addrbuf.pad) - 1] = '\0';

	tprints("{sa_family=");
	printxval(addrfams, addrbuf.sa.sa_family, "AF_???");
	tprints(", ");

	switch (addrbuf.sa.sa_family) {
	case AF_UNIX:
		if (addrlen == 2) {
			tprints("NULL");
		} else if (addrbuf.sau.sun_path[0]) {
			tprints("sun_path=");
			print_quoted_string(addrbuf.sau.sun_path,
					    sizeof(addrbuf.sau.sun_path) + 1,
					    QUOTE_0_TERMINATED);
		} else {
			tprints("sun_path=@");
			print_quoted_string(addrbuf.sau.sun_path + 1,
					    sizeof(addrbuf.sau.sun_path),
					    QUOTE_0_TERMINATED);
		}
		break;
	case AF_INET:
		tprintf("sin_port=htons(%u), sin_addr=inet_addr(\"%s\")",
			ntohs(addrbuf.sin.sin_port), inet_ntoa(addrbuf.sin.sin_addr));
		break;
#ifdef HAVE_INET_NTOP
	case AF_INET6:
		inet_ntop(AF_INET6, &addrbuf.sa6.sin6_addr, string_addr, sizeof(string_addr));
		tprintf("sin6_port=htons(%u), inet_pton(AF_INET6, \"%s\", &sin6_addr), sin6_flowinfo=%u",
				ntohs(addrbuf.sa6.sin6_port), string_addr,
				addrbuf.sa6.sin6_flowinfo);
#ifdef HAVE_STRUCT_SOCKADDR_IN6_SIN6_SCOPE_ID
		tprints(", sin6_scope_id=");
#if defined IN6_IS_ADDR_LINKLOCAL && defined IN6_IS_ADDR_MC_LINKLOCAL
		if (IN6_IS_ADDR_LINKLOCAL(&addrbuf.sa6.sin6_addr)
		    || IN6_IS_ADDR_MC_LINKLOCAL(&addrbuf.sa6.sin6_addr))
			print_ifindex(addrbuf.sa6.sin6_scope_id);
		else
#endif
			tprintf("%u", addrbuf.sa6.sin6_scope_id);
#endif /* HAVE_STRUCT_SOCKADDR_IN6_SIN6_SCOPE_ID */
		break;
#endif
#if defined(AF_IPX)
	case AF_IPX:
		{
			int i;
			tprintf("sipx_port=htons(%u), ",
					ntohs(addrbuf.sipx.sipx_port));
			/* Yes, I know, this does not look too
			 * strace-ish, but otherwise the IPX
			 * addresses just look monstrous...
			 * Anyways, feel free if you don't like
			 * this way.. :)
			 */
			tprintf("%08lx:", (unsigned long)ntohl(addrbuf.sipx.sipx_network));
			for (i = 0; i < IPX_NODE_LEN; i++)
				tprintf("%02x", addrbuf.sipx.sipx_node[i]);
			tprintf("/[%02x]", addrbuf.sipx.sipx_type);
		}
		break;
#endif /* AF_IPX */
#ifdef AF_PACKET
	case AF_PACKET:
		{
			int i;
			tprintf("proto=%#04x, if%d, pkttype=",
					ntohs(addrbuf.ll.sll_protocol),
					addrbuf.ll.sll_ifindex);
			printxval(af_packet_types, addrbuf.ll.sll_pkttype, "PACKET_???");
			tprintf(", addr(%d)={%d, ",
					addrbuf.ll.sll_halen,
					addrbuf.ll.sll_hatype);
			for (i = 0; i < addrbuf.ll.sll_halen; i++)
				tprintf("%02x", addrbuf.ll.sll_addr[i]);
		}
		break;

#endif /* AF_PACKET */
#ifdef AF_NETLINK
	case AF_NETLINK:
		tprintf("pid=%d, groups=%08x", addrbuf.nl.nl_pid, addrbuf.nl.nl_groups);
		break;
#endif /* AF_NETLINK */
#if defined(AF_BLUETOOTH) && defined(HAVE_BLUETOOTH_BLUETOOTH_H)
	case AF_BLUETOOTH:
		tprintf("{sco_bdaddr=%02X:%02X:%02X:%02X:%02X:%02X} or "
			"{rc_bdaddr=%02X:%02X:%02X:%02X:%02X:%02X, rc_channel=%d} or "
			"{l2_psm=htobs(%d), l2_bdaddr=%02X:%02X:%02X:%02X:%02X:%02X, l2_cid=htobs(%d)} or "
			"{hci_dev=htobs(%d)}",
			addrbuf.sco.sco_bdaddr.b[0], addrbuf.sco.sco_bdaddr.b[1],
			addrbuf.sco.sco_bdaddr.b[2], addrbuf.sco.sco_bdaddr.b[3],
			addrbuf.sco.sco_bdaddr.b[4], addrbuf.sco.sco_bdaddr.b[5],
			addrbuf.rc.rc_bdaddr.b[0], addrbuf.rc.rc_bdaddr.b[1],
			addrbuf.rc.rc_bdaddr.b[2], addrbuf.rc.rc_bdaddr.b[3],
			addrbuf.rc.rc_bdaddr.b[4], addrbuf.rc.rc_bdaddr.b[5],
			addrbuf.rc.rc_channel,
			btohs(addrbuf.l2.l2_psm), addrbuf.l2.l2_bdaddr.b[0],
			addrbuf.l2.l2_bdaddr.b[1], addrbuf.l2.l2_bdaddr.b[2],
			addrbuf.l2.l2_bdaddr.b[3], addrbuf.l2.l2_bdaddr.b[4],
			addrbuf.l2.l2_bdaddr.b[5], btohs(addrbuf.l2.l2_cid),
			btohs(addrbuf.hci.hci_dev));
		break;
#endif /* AF_BLUETOOTH && HAVE_BLUETOOTH_BLUETOOTH_H */
	/* AF_AX25 AF_APPLETALK AF_NETROM AF_BRIDGE AF_AAL5
	AF_X25 AF_ROSE etc. still need to be done */

	default:
		tprints("sa_data=");
		print_quoted_string(addrbuf.sa.sa_data,
				    sizeof(addrbuf.sa.sa_data), 0);
		break;
	}
	tprints("}");
}

#ifdef HAVE_SENDMSG
# ifndef SCM_SECURITY
#  define SCM_SECURITY 0x03
# endif
# include "xlat/scmvals.h"

#if SUPPORTED_PERSONALITIES > 1 && SIZEOF_LONG > 4
struct cmsghdr32 {
	uint32_t cmsg_len;
	int cmsg_level;
	int cmsg_type;
};
#endif

typedef union {
	char *ptr;
	struct cmsghdr *cmsg;
#if SUPPORTED_PERSONALITIES > 1 && SIZEOF_LONG > 4
	struct cmsghdr32 *cmsg32;
#endif
} union_cmsghdr;

static bool
print_scm_rights(struct tcb *tcp, size_t cmsg_size, char *ptr, size_t cmsg_len)
{
	if (cmsg_size + sizeof(int) > cmsg_len)
		return false;

	int *fds = (int *) (ptr + cmsg_size);
	bool seen = false;

	tprints(", [");
	while ((char *) fds < (ptr + cmsg_len)) {
		if (seen)
			tprints(", ");
		else
			seen = true;
		printfd(tcp, *fds++);
	}
	tprints("]}");
	return true;
}

static bool
print_scm_creds(struct tcb *tcp, size_t cmsg_size, char *ptr, size_t cmsg_len)
{
	if (cmsg_size + sizeof(struct ucred) > cmsg_len)
		return false;

	const struct ucred *uc = (void *) (ptr + cmsg_size);

	tprintf(", {pid=%u, uid=%u, gid=%u}}",
		(unsigned) uc->pid, (unsigned) uc->uid, (unsigned) uc->gid);
	return true;
}

static bool
print_scm_security(struct tcb *tcp, size_t cmsg_size, char *ptr, size_t cmsg_len)
{
	if (cmsg_size + sizeof(char) > cmsg_len)
		return false;

	const char *label = (const char *) (ptr + cmsg_size);
	const size_t label_len = cmsg_len - cmsg_size;

	tprints(", ");
	print_quoted_string(label, label_len, 0);
	tprints("}");

	return true;
}

static void
printcmsghdr(struct tcb *tcp, unsigned long addr, size_t len)
{
	const size_t cmsg_size =
#if SUPPORTED_PERSONALITIES > 1 && SIZEOF_LONG > 4
		(current_wordsize < sizeof(long)) ? sizeof(struct cmsghdr32) :
#endif
			sizeof(struct cmsghdr);

	char *buf = len < cmsg_size ? NULL : malloc(len);
	if (!buf || umoven(tcp, addr, len, buf) < 0) {
		tprints(", msg_control=");
		printaddr(addr);
		free(buf);
		return;
	}

	union_cmsghdr u = { .ptr = buf };

	tprints(", [");
	while (len >= cmsg_size) {
		size_t cmsg_len =
#if SUPPORTED_PERSONALITIES > 1 && SIZEOF_LONG > 4
			(current_wordsize < sizeof(long)) ? u.cmsg32->cmsg_len :
#endif
				u.cmsg->cmsg_len;
		int cmsg_level =
#if SUPPORTED_PERSONALITIES > 1 && SIZEOF_LONG > 4
			(current_wordsize < sizeof(long)) ? u.cmsg32->cmsg_level :
#endif
				u.cmsg->cmsg_level;
		int cmsg_type =
#if SUPPORTED_PERSONALITIES > 1 && SIZEOF_LONG > 4
			(current_wordsize < sizeof(long)) ? u.cmsg32->cmsg_type :
#endif
				u.cmsg->cmsg_type;

		if (u.ptr != buf)
			tprints(", ");
		tprintf("{cmsg_len=%lu, cmsg_level=", (unsigned long) cmsg_len);
		printxval(socketlayers, cmsg_level, "SOL_???");
		tprints(", cmsg_type=");

		if (cmsg_len > len)
			cmsg_len = len;

		if (cmsg_level == SOL_SOCKET) {
			printxval(scmvals, cmsg_type, "SCM_???");
			switch (cmsg_type) {
			case SCM_RIGHTS:
				if (print_scm_rights(tcp, cmsg_size, u.ptr, cmsg_len))
					goto next_cmsg;
				break;
			case SCM_CREDENTIALS:
				if (print_scm_creds(tcp, cmsg_size, u.ptr, cmsg_len))
					goto next_cmsg;
				break;
			case SCM_SECURITY:
				if (print_scm_security(tcp, cmsg_size, u.ptr, cmsg_len))
					goto next_cmsg;
				break;
			}
		} else {
			tprintf("%u", cmsg_type);
		}
		tprints(", ...}");
next_cmsg:
		if (cmsg_len < cmsg_size) {
			len -= cmsg_size;
			break;
		}
		cmsg_len = (cmsg_len + current_wordsize - 1) &
			(size_t) ~(current_wordsize - 1);
		if (cmsg_len >= len) {
			len = 0;
			break;
		}
		u.ptr += cmsg_len;
		len -= cmsg_len;
	}
	if (len)
		tprints(", ...");
	tprints("]");
	free(buf);
}

static void
do_msghdr(struct tcb *tcp, struct msghdr *msg, unsigned long data_size)
{
	tprintf("{msg_name(%d)=", msg->msg_namelen);
	printsock(tcp, (long)msg->msg_name, msg->msg_namelen);

	tprintf(", msg_iov(%lu)=", (unsigned long)msg->msg_iovlen);
	tprint_iov_upto(tcp, (unsigned long)msg->msg_iovlen,
		   (unsigned long)msg->msg_iov, 1, data_size);

#ifdef HAVE_STRUCT_MSGHDR_MSG_CONTROL
	tprintf(", msg_controllen=%lu", (unsigned long)msg->msg_controllen);
	if (msg->msg_controllen)
		printcmsghdr(tcp, (unsigned long) msg->msg_control,
			     msg->msg_controllen);
	tprints(", msg_flags=");
	printflags(msg_flags, msg->msg_flags, "MSG_???");
#else /* !HAVE_STRUCT_MSGHDR_MSG_CONTROL */
	tprintf("msg_accrights=%#lx, msg_accrightslen=%u",
		(unsigned long) msg->msg_accrights, msg->msg_accrightslen);
#endif /* !HAVE_STRUCT_MSGHDR_MSG_CONTROL */
	tprints("}");
}

struct msghdr32 {
	uint32_t /* void* */    msg_name;
	uint32_t /* socklen_t */msg_namelen;
	uint32_t /* iovec* */   msg_iov;
	uint32_t /* size_t */   msg_iovlen;
	uint32_t /* void* */    msg_control;
	uint32_t /* size_t */   msg_controllen;
	uint32_t /* int */      msg_flags;
};
struct mmsghdr32 {
	struct msghdr32         msg_hdr;
	uint32_t /* unsigned */ msg_len;
};

#ifndef HAVE_STRUCT_MMSGHDR
struct mmsghdr {
	struct msghdr msg_hdr;
	unsigned msg_len;
};
#endif

#if SUPPORTED_PERSONALITIES > 1 && SIZEOF_LONG > 4
static void
copy_from_msghdr32(struct msghdr *to_msg, struct msghdr32 *from_msg32)
{
	to_msg->msg_name       = (void*)(long)from_msg32->msg_name;
	to_msg->msg_namelen    =              from_msg32->msg_namelen;
	to_msg->msg_iov        = (void*)(long)from_msg32->msg_iov;
	to_msg->msg_iovlen     =              from_msg32->msg_iovlen;
	to_msg->msg_control    = (void*)(long)from_msg32->msg_control;
	to_msg->msg_controllen =              from_msg32->msg_controllen;
	to_msg->msg_flags      =              from_msg32->msg_flags;
}
#endif

static bool
extractmsghdr(struct tcb *tcp, long addr, struct msghdr *msg)
{
#if SUPPORTED_PERSONALITIES > 1 && SIZEOF_LONG > 4
	if (current_wordsize == 4) {
		struct msghdr32 msg32;

		if (umove(tcp, addr, &msg32) < 0)
			return false;
		copy_from_msghdr32(msg, &msg32);
	} else
#endif
	if (umove(tcp, addr, msg) < 0)
		return false;
	return true;
}

static bool
extractmmsghdr(struct tcb *tcp, long addr, unsigned int idx, struct mmsghdr *mmsg)
{
#if SUPPORTED_PERSONALITIES > 1 && SIZEOF_LONG > 4
	if (current_wordsize == 4) {
		struct mmsghdr32 mmsg32;

		addr += sizeof(struct mmsghdr32) * idx;
		if (umove(tcp, addr, &mmsg32) < 0)
			return false;

		copy_from_msghdr32(&mmsg->msg_hdr, &mmsg32.msg_hdr);
		mmsg->msg_len = mmsg32.msg_len;
	} else
#endif
	{
		addr += sizeof(*mmsg) * idx;
		if (umove(tcp, addr, mmsg) < 0)
			return false;
	}
	return true;
}

static void
printmsghdr(struct tcb *tcp, long addr, unsigned long data_size)
{
	struct msghdr msg;

	if (verbose(tcp) && extractmsghdr(tcp, addr, &msg))
		do_msghdr(tcp, &msg, data_size);
	else
		printaddr(addr);
}

void
dumpiov_in_msghdr(struct tcb *tcp, long addr)
{
	struct msghdr msg;

	if (extractmsghdr(tcp, addr, &msg))
		dumpiov(tcp, msg.msg_iovlen, (long)msg.msg_iov);
}

static void
printmmsghdr(struct tcb *tcp, long addr, unsigned int idx, unsigned long msg_len)
{
	struct mmsghdr mmsg;

	if (extractmmsghdr(tcp, addr, idx, &mmsg)) {
		tprints("{");
		do_msghdr(tcp, &mmsg.msg_hdr, msg_len ? msg_len : mmsg.msg_len);
		tprintf(", %u}", mmsg.msg_len);
	}
	else
		printaddr(addr);
}

static void
decode_mmsg(struct tcb *tcp, unsigned long msg_len)
{
	/* mmsgvec */
	if (syserror(tcp)) {
		printaddr(tcp->u_arg[1]);
	} else {
		unsigned int len = tcp->u_rval;
		unsigned int i;

		tprints("{");
		for (i = 0; i < len; ++i) {
			if (i)
				tprints(", ");
			printmmsghdr(tcp, tcp->u_arg[1], i, msg_len);
		}
		tprints("}");
	}
	/* vlen */
	tprintf(", %u, ", (unsigned int) tcp->u_arg[2]);
	/* flags */
	printflags(msg_flags, tcp->u_arg[3], "MSG_???");
}

void
dumpiov_in_mmsghdr(struct tcb *tcp, long addr)
{
	unsigned int len = tcp->u_rval;
	unsigned int i;
	struct mmsghdr mmsg;

	for (i = 0; i < len; ++i) {
		if (extractmmsghdr(tcp, addr, i, &mmsg)) {
			tprintf(" = %lu buffers in vector %u\n",
				(unsigned long)mmsg.msg_hdr.msg_iovlen, i);
			dumpiov(tcp, mmsg.msg_hdr.msg_iovlen,
				(long)mmsg.msg_hdr.msg_iov);
		}
	}
}
#endif /* HAVE_SENDMSG */

/*
 * low bits of the socket type define real socket type,
 * other bits are socket type flags.
 */
static void
tprint_sock_type(int flags)
{
	const char *str = xlookup(socktypes, flags & SOCK_TYPE_MASK);

	if (str) {
		tprints(str);
		flags &= ~SOCK_TYPE_MASK;
		if (!flags)
			return;
		tprints("|");
	}
	printflags(sock_type_flags, flags, "SOCK_???");
}

SYS_FUNC(socket)
{
	printxval(domains, tcp->u_arg[0], "PF_???");
	tprints(", ");
	tprint_sock_type(tcp->u_arg[1]);
	tprints(", ");
	switch (tcp->u_arg[0]) {
	case PF_INET:
#ifdef PF_INET6
	case PF_INET6:
#endif
		printxval(inet_protocols, tcp->u_arg[2], "IPPROTO_???");
		break;
#ifdef PF_IPX
	case PF_IPX:
		/* BTW: I don't believe this.. */
		tprints("[");
		printxval(domains, tcp->u_arg[2], "PF_???");
		tprints("]");
		break;
#endif /* PF_IPX */
#ifdef PF_NETLINK
	case PF_NETLINK:
		printxval(netlink_protocols, tcp->u_arg[2], "NETLINK_???");
		break;
#endif
#if defined(PF_BLUETOOTH) && defined(HAVE_BLUETOOTH_BLUETOOTH_H)
	case PF_BLUETOOTH:
		printxval(bt_protocols, tcp->u_arg[2], "BTPROTO_???");
		break;
#endif
	default:
		tprintf("%lu", tcp->u_arg[2]);
		break;
	}

	return RVAL_DECODED | RVAL_FD;
}

SYS_FUNC(bind)
{
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	printsock(tcp, tcp->u_arg[1], tcp->u_arg[2]);
	tprintf(", %lu", tcp->u_arg[2]);

	return RVAL_DECODED;
}

SYS_FUNC(listen)
{
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	tprintf("%lu", tcp->u_arg[1]);

	return RVAL_DECODED;
}

static int
do_sockname(struct tcb *tcp, int flags_arg)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		return 0;
	}

	int len;
	if (!tcp->u_arg[2] || !verbose(tcp) || syserror(tcp) ||
	    umove(tcp, tcp->u_arg[2], &len) < 0) {
		printaddr(tcp->u_arg[1]);
		tprints(", ");
		printaddr(tcp->u_arg[2]);
	} else {
		printsock(tcp, tcp->u_arg[1], len);
		tprintf(", [%d]", len);
	}

	if (flags_arg >= 0) {
		tprints(", ");
		printflags(sock_type_flags, tcp->u_arg[flags_arg],
			   "SOCK_???");
	}
	return 0;
}

SYS_FUNC(accept)
{
	do_sockname(tcp, -1);
	return RVAL_FD;
}

SYS_FUNC(accept4)
{
	do_sockname(tcp, 3);
	return RVAL_FD;
}

SYS_FUNC(send)
{
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	printstr(tcp, tcp->u_arg[1], tcp->u_arg[2]);
	tprintf(", %lu, ", tcp->u_arg[2]);
	/* flags */
	printflags(msg_flags, tcp->u_arg[3], "MSG_???");

	return RVAL_DECODED;
}

SYS_FUNC(sendto)
{
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	printstr(tcp, tcp->u_arg[1], tcp->u_arg[2]);
	tprintf(", %lu, ", tcp->u_arg[2]);
	/* flags */
	printflags(msg_flags, tcp->u_arg[3], "MSG_???");
	/* to address */
	tprints(", ");
	printsock(tcp, tcp->u_arg[4], tcp->u_arg[5]);
	/* to length */
	tprintf(", %lu", tcp->u_arg[5]);

	return RVAL_DECODED;
}

#ifdef HAVE_SENDMSG

SYS_FUNC(sendmsg)
{
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	printmsghdr(tcp, tcp->u_arg[1], (unsigned long) -1L);
	/* flags */
	tprints(", ");
	printflags(msg_flags, tcp->u_arg[2], "MSG_???");

	return RVAL_DECODED;
}

SYS_FUNC(sendmmsg)
{
	if (entering(tcp)) {
		/* sockfd */
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		if (!verbose(tcp)) {
			tprintf("%#lx, %u, ",
				tcp->u_arg[1], (unsigned int) tcp->u_arg[2]);
			printflags(msg_flags, tcp->u_arg[3], "MSG_???");
		}
	} else {
		if (verbose(tcp))
			decode_mmsg(tcp, (unsigned long) -1L);
	}
	return 0;
}

#endif /* HAVE_SENDMSG */

SYS_FUNC(recv)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		if (syserror(tcp))
			printaddr(tcp->u_arg[1]);
		else
			printstr(tcp, tcp->u_arg[1], tcp->u_rval);

		tprintf(", %lu, ", tcp->u_arg[2]);
		printflags(msg_flags, tcp->u_arg[3], "MSG_???");
	}
	return 0;
}

SYS_FUNC(recvfrom)
{
	int fromlen;

	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		/* buf */
		if (syserror(tcp)) {
			printaddr(tcp->u_arg[1]);
		} else {
			printstr(tcp, tcp->u_arg[1], tcp->u_rval);
		}
		/* len */
		tprintf(", %lu, ", tcp->u_arg[2]);
		/* flags */
		printflags(msg_flags, tcp->u_arg[3], "MSG_???");
		tprints(", ");
		if (syserror(tcp) || !tcp->u_arg[4] || !tcp->u_arg[5] ||
		    umove(tcp, tcp->u_arg[5], &fromlen) < 0) {
			/* from address, len */
			printaddr(tcp->u_arg[4]);
			tprints(", ");
			printaddr(tcp->u_arg[5]);
			return 0;
		}
		/* from address */
		printsock(tcp, tcp->u_arg[4], fromlen);
		/* from length */
		tprintf(", [%u]", fromlen);
	}
	return 0;
}

#ifdef HAVE_SENDMSG

SYS_FUNC(recvmsg)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		if (syserror(tcp))
			printaddr(tcp->u_arg[1]);
		else
			printmsghdr(tcp, tcp->u_arg[1], tcp->u_rval);
		/* flags */
		tprints(", ");
		printflags(msg_flags, tcp->u_arg[2], "MSG_???");
	}
	return 0;
}

SYS_FUNC(recvmmsg)
{
	static char str[sizeof("left") + TIMESPEC_TEXT_BUFSIZE];

	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		if (verbose(tcp)) {
			/* Abusing tcp->auxstr as temp storage.
			 * Will be used and cleared on syscall exit.
			 */
			tcp->auxstr = sprint_timespec(tcp, tcp->u_arg[4]);
		} else {
			tprintf("%#lx, %ld, ", tcp->u_arg[1], tcp->u_arg[2]);
			printflags(msg_flags, tcp->u_arg[3], "MSG_???");
			tprints(", ");
			print_timespec(tcp, tcp->u_arg[4]);
		}
		return 0;
	} else {
		if (verbose(tcp)) {
			decode_mmsg(tcp, 0);
			tprints(", ");
			/* timeout on entrance */
			tprints(tcp->auxstr);
			tcp->auxstr = NULL;
		}
		if (syserror(tcp))
			return 0;
		if (tcp->u_rval == 0) {
			tcp->auxstr = "Timeout";
			return RVAL_STR;
		}
		if (!verbose(tcp))
			return 0;
		/* timeout on exit */
		snprintf(str, sizeof(str), "left %s",
			 sprint_timespec(tcp, tcp->u_arg[4]));
		tcp->auxstr = str;
		return RVAL_STR;
	}
}

#endif /* HAVE_SENDMSG */

#include "xlat/shutdown_modes.h"

SYS_FUNC(shutdown)
{
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	printxval(shutdown_modes, tcp->u_arg[1], "SHUT_???");

	return RVAL_DECODED;
}

SYS_FUNC(getsockname)
{
	return do_sockname(tcp, -1);
}

static void
printpair_fd(struct tcb *tcp, const int i0, const int i1)
{
	tprints("[");
	printfd(tcp, i0);
	tprints(", ");
	printfd(tcp, i1);
	tprints("]");
}

static void
decode_pair_fd(struct tcb *tcp, const long addr)
{
	int pair[2];

	if (umove_or_printaddr(tcp, addr, &pair))
		return;

	printpair_fd(tcp, pair[0], pair[1]);
}

static int
do_pipe(struct tcb *tcp, int flags_arg)
{
	if (exiting(tcp)) {
		if (syserror(tcp)) {
			printaddr(tcp->u_arg[0]);
		} else {
#ifdef HAVE_GETRVAL2
			if (flags_arg < 0) {
				printpair_fd(tcp, tcp->u_rval, getrval2(tcp));
			} else
#endif
				decode_pair_fd(tcp, tcp->u_arg[0]);
		}
		if (flags_arg >= 0) {
			tprints(", ");
			printflags(open_mode_flags, tcp->u_arg[flags_arg], "O_???");
		}
	}
	return 0;
}

SYS_FUNC(pipe)
{
	return do_pipe(tcp, -1);
}

SYS_FUNC(pipe2)
{
	return do_pipe(tcp, 1);
}

SYS_FUNC(socketpair)
{
	if (entering(tcp)) {
		printxval(domains, tcp->u_arg[0], "PF_???");
		tprints(", ");
		tprint_sock_type(tcp->u_arg[1]);
		tprintf(", %lu", tcp->u_arg[2]);
	} else {
		tprints(", ");
		decode_pair_fd(tcp, tcp->u_arg[3]);
	}
	return 0;
}

#include "xlat/sockoptions.h"
#include "xlat/sockipoptions.h"
#include "xlat/getsockipoptions.h"
#include "xlat/setsockipoptions.h"
#include "xlat/sockipv6options.h"
#include "xlat/getsockipv6options.h"
#include "xlat/setsockipv6options.h"
#include "xlat/sockipxoptions.h"
#include "xlat/sockrawoptions.h"
#include "xlat/sockpacketoptions.h"
#include "xlat/socksctpoptions.h"
#include "xlat/socktcpoptions.h"

static void
print_sockopt_fd_level_name(struct tcb *tcp, int fd, int level, int name, bool is_getsockopt)
{
	printfd(tcp, fd);
	tprints(", ");
	printxval(socketlayers, level, "SOL_??");
	tprints(", ");

	switch (level) {
	case SOL_SOCKET:
		printxval(sockoptions, name, "SO_???");
		break;
	case SOL_IP:
		printxvals(name, "IP_???", sockipoptions,
			is_getsockopt ? getsockipoptions : setsockipoptions, NULL);
		break;
	case SOL_IPV6:
		printxvals(name, "IPV6_???", sockipv6options,
			is_getsockopt ? getsockipv6options : setsockipv6options, NULL);
		break;
	case SOL_IPX:
		printxval(sockipxoptions, name, "IPX_???");
		break;
	case SOL_PACKET:
		printxval(sockpacketoptions, name, "PACKET_???");
		break;
	case SOL_TCP:
		printxval(socktcpoptions, name, "TCP_???");
		break;
	case SOL_SCTP:
		printxval(socksctpoptions, name, "SCTP_???");
		break;
	case SOL_RAW:
		printxval(sockrawoptions, name, "RAW_???");
		break;

		/* Other SOL_* protocol levels still need work. */

	default:
		tprintf("%u", name);
	}

	tprints(", ");
}

#ifdef SO_LINGER
static void
print_linger(struct tcb *tcp, long addr, int len)
{
	struct linger linger;

	if (len != sizeof(linger) ||
	    umove(tcp, addr, &linger) < 0) {
		printaddr(addr);
		return;
	}

	tprintf("{onoff=%d, linger=%d}",
		linger.l_onoff,
		linger.l_linger);
}
#endif /* SO_LINGER */

#ifdef SO_PEERCRED
static void
print_ucred(struct tcb *tcp, long addr, int len)
{
	struct ucred uc;

	if (len != sizeof(uc) ||
	    umove(tcp, addr, &uc) < 0) {
		printaddr(addr);
	} else {
		tprintf("{pid=%u, uid=%u, gid=%u}",
			(unsigned) uc.pid,
			(unsigned) uc.uid,
			(unsigned) uc.gid);
	}
}
#endif /* SO_PEERCRED */

#ifdef PACKET_STATISTICS
static void
print_tpacket_stats(struct tcb *tcp, long addr, int len)
{
	struct tpacket_stats stats;

	if (len != sizeof(stats) ||
	    umove(tcp, addr, &stats) < 0) {
		printaddr(addr);
	} else {
		tprintf("{packets=%u, drops=%u}",
			stats.tp_packets,
			stats.tp_drops);
	}
}
#endif /* PACKET_STATISTICS */

#ifdef ICMP_FILTER
# include "xlat/icmpfilterflags.h"

static void
print_icmp_filter(struct tcb *tcp, long addr, int len)
{
	struct icmp_filter	filter;

	if (len != sizeof(filter) ||
	    umove(tcp, addr, &filter) < 0) {
		printaddr(addr);
		return;
	}

	tprints("~(");
	printflags(icmpfilterflags, ~filter.data, "ICMP_???");
	tprints(")");
}
#endif /* ICMP_FILTER */

static void
print_getsockopt(struct tcb *tcp, int level, int name, long addr, int len)
{
	if (addr && verbose(tcp))
	switch (level) {
	case SOL_SOCKET:
		switch (name) {
#ifdef SO_LINGER
		case SO_LINGER:
			print_linger(tcp, addr, len);
			goto done;
#endif
#ifdef SO_PEERCRED
		case SO_PEERCRED:
			print_ucred(tcp, addr, len);
			goto done;
#endif
		}
		break;

	case SOL_PACKET:
		switch (name) {
#ifdef PACKET_STATISTICS
		case PACKET_STATISTICS:
			print_tpacket_stats(tcp, addr, len);
			goto done;
#endif
		}
		break;

	case SOL_RAW:
		switch (name) {
#ifdef ICMP_FILTER
		case ICMP_FILTER:
			print_icmp_filter(tcp, addr, len);
			goto done;
#endif
		}
		break;
	}

	/* default arg printing */

	if (verbose(tcp)) {
		if (len == sizeof(int)) {
			printnum_int(tcp, addr, "%d");
		} else {
			printstr(tcp, addr, len);
		}
	} else {
		printaddr(addr);
	}
done:
	tprintf(", [%d]", len);
}

SYS_FUNC(getsockopt)
{
	if (entering(tcp)) {
		print_sockopt_fd_level_name(tcp, tcp->u_arg[0],
					    tcp->u_arg[1], tcp->u_arg[2], true);
	} else {
		int len;

		if (syserror(tcp) || umove(tcp, tcp->u_arg[4], &len) < 0) {
			tprintf("%#lx, %#lx",
				tcp->u_arg[3], tcp->u_arg[4]);
		} else {
			print_getsockopt(tcp, tcp->u_arg[1], tcp->u_arg[2],
					 tcp->u_arg[3], len);
		}
	}
	return 0;
}

#ifdef IP_ADD_MEMBERSHIP
static void
print_mreq(struct tcb *tcp, long addr, unsigned int len)
{
	struct ip_mreq mreq;

	if (len < sizeof(mreq)) {
		printstr(tcp, addr, len);
		return;
	}
	if (umove_or_printaddr(tcp, addr, &mreq))
		return;

	tprints("{imr_multiaddr=inet_addr(");
	print_quoted_string(inet_ntoa(mreq.imr_multiaddr),
			    16, QUOTE_0_TERMINATED);
	tprints("), imr_interface=inet_addr(");
	print_quoted_string(inet_ntoa(mreq.imr_interface),
			    16, QUOTE_0_TERMINATED);
	tprints(")}");
}
#endif /* IP_ADD_MEMBERSHIP */

#ifdef IPV6_ADD_MEMBERSHIP
static void
print_mreq6(struct tcb *tcp, long addr, unsigned int len)
{
	struct ipv6_mreq mreq;

	if (len < sizeof(mreq))
		goto fail;

	if (umove_or_printaddr(tcp, addr, &mreq))
		return;

#ifdef HAVE_INET_NTOP
	const struct in6_addr *in6 = &mreq.ipv6mr_multiaddr;
	char address[INET6_ADDRSTRLEN];

	if (!inet_ntop(AF_INET6, in6, address, sizeof(address)))
		goto fail;

	tprints("{ipv6mr_multiaddr=inet_pton(");
	print_quoted_string(address, sizeof(address), QUOTE_0_TERMINATED);
	tprints("), ipv6mr_interface=");
	print_ifindex(mreq.ipv6mr_interface);
	tprints("}");
	return;
#endif /* HAVE_INET_NTOP */

fail:
	printstr(tcp, addr, len);
}
#endif /* IPV6_ADD_MEMBERSHIP */

#ifdef MCAST_JOIN_GROUP
static void
print_group_req(struct tcb *tcp, long addr, int len)
{
	struct group_req greq;

	if (len != sizeof(greq) ||
	    umove(tcp, addr, &greq) < 0) {
		printaddr(addr);
		return;
	}

	union {
		struct sockaddr *sa;
		struct sockaddr_in *sin;
#ifdef HAVE_INET_NTOP
		struct sockaddr_in6 *sin6;
#endif
	} a = { .sa = (struct sockaddr *) &greq.gr_group };
#ifdef HAVE_INET_NTOP
	char str[INET6_ADDRSTRLEN];
#endif

	tprintf("{gr_interface=%u, gr_group={sa_family=", greq.gr_interface);
	printxval(addrfams, a.sa->sa_family, "AF_???");

	switch (a.sa->sa_family) {
	case AF_INET:
		tprintf(", sin_port=htons(%u), sin_addr=inet_addr(\"%s\")}}",
			ntohs(a.sin->sin_port),
			inet_ntoa(a.sin->sin_addr));
		return;
#ifdef HAVE_INET_NTOP
	case AF_INET6:
		if (!inet_ntop(AF_INET6, &a.sin6->sin6_addr, str, sizeof(str)))
			break;
		tprintf(", sin6_port=htons(%u)"
			", inet_pton(AF_INET6, \"%s\", &sin6_addr)}}",
			ntohs(a.sin6->sin6_port), str);
		return;
#endif /* HAVE_INET_NTOP */
	}

	tprints(", sa_data=");
	print_quoted_string(a.sa->sa_data, sizeof(a.sa->sa_data), 0);
	tprintf("}}");

}
#endif /* MCAST_JOIN_GROUP */

#ifdef PACKET_RX_RING
static void
print_tpacket_req(struct tcb *tcp, long addr, int len)
{
	struct tpacket_req req;

	if (len != sizeof(req) ||
	    umove(tcp, addr, &req) < 0) {
		printaddr(addr);
	} else {
		tprintf("{block_size=%u, block_nr=%u, "
			"frame_size=%u, frame_nr=%u}",
			req.tp_block_size,
			req.tp_block_nr,
			req.tp_frame_size,
			req.tp_frame_nr);
	}
}
#endif /* PACKET_RX_RING */

#ifdef PACKET_ADD_MEMBERSHIP
# include "xlat/packet_mreq_type.h"

static void
print_packet_mreq(struct tcb *tcp, long addr, int len)
{
	struct packet_mreq mreq;

	if (len != sizeof(mreq) ||
	    umove(tcp, addr, &mreq) < 0) {
		printaddr(addr);
	} else {
		unsigned int i;

		tprintf("{mr_ifindex=%u, mr_type=", mreq.mr_ifindex);
		printxval(packet_mreq_type, mreq.mr_type, "PACKET_MR_???");
		tprintf(", mr_alen=%u, mr_address=", mreq.mr_alen);
		if (mreq.mr_alen > ARRAY_SIZE(mreq.mr_address))
			mreq.mr_alen = ARRAY_SIZE(mreq.mr_address);
		for (i = 0; i < mreq.mr_alen; ++i)
			tprintf("%02x", mreq.mr_address[i]);
		tprints("}");
	}
}
#endif /* PACKET_ADD_MEMBERSHIP */

static void
print_setsockopt(struct tcb *tcp, int level, int name, long addr, int len)
{
	if (addr && verbose(tcp))
	switch (level) {
	case SOL_SOCKET:
		switch (name) {
#ifdef SO_LINGER
		case SO_LINGER:
			print_linger(tcp, addr, len);
			goto done;
#endif
		}
		break;

	case SOL_IP:
		switch (name) {
#ifdef IP_ADD_MEMBERSHIP
		case IP_ADD_MEMBERSHIP:
		case IP_DROP_MEMBERSHIP:
			print_mreq(tcp, addr, len);
			goto done;
#endif /* IP_ADD_MEMBERSHIP */
#ifdef MCAST_JOIN_GROUP
		case MCAST_JOIN_GROUP:
		case MCAST_LEAVE_GROUP:
			print_group_req(tcp, addr, len);
			goto done;
#endif /* MCAST_JOIN_GROUP */
		}
		break;

	case SOL_IPV6:
		switch (name) {
#ifdef IPV6_ADD_MEMBERSHIP
		case IPV6_ADD_MEMBERSHIP:
		case IPV6_DROP_MEMBERSHIP:
# ifdef IPV6_JOIN_ANYCAST
		case IPV6_JOIN_ANYCAST:
# endif
# ifdef IPV6_LEAVE_ANYCAST
		case IPV6_LEAVE_ANYCAST:
# endif
			print_mreq6(tcp, addr, len);
			goto done;
#endif /* IPV6_ADD_MEMBERSHIP */
		}
		break;

	case SOL_PACKET:
		switch (name) {
#ifdef PACKET_RX_RING
		case PACKET_RX_RING:
# ifdef PACKET_TX_RING
		case PACKET_TX_RING:
# endif
			print_tpacket_req(tcp, addr, len);
			goto done;
#endif /* PACKET_RX_RING */
#ifdef PACKET_ADD_MEMBERSHIP
		case PACKET_ADD_MEMBERSHIP:
		case PACKET_DROP_MEMBERSHIP:
			print_packet_mreq(tcp, addr, len);
			goto done;
#endif /* PACKET_ADD_MEMBERSHIP */
		}
		break;

	case SOL_RAW:
		switch (name) {
#ifdef ICMP_FILTER
		case ICMP_FILTER:
			print_icmp_filter(tcp, addr, len);
			goto done;
#endif
		}
		break;
	}

	/* default arg printing */

	if (verbose(tcp)) {
		if (len == sizeof(int)) {
			printnum_int(tcp, addr, "%d");
		} else {
			printstr(tcp, addr, len);
		}
	} else {
		printaddr(addr);
	}
done:
	tprintf(", %d", len);
}

SYS_FUNC(setsockopt)
{
	print_sockopt_fd_level_name(tcp, tcp->u_arg[0],
				    tcp->u_arg[1], tcp->u_arg[2], false);
	print_setsockopt(tcp, tcp->u_arg[1], tcp->u_arg[2],
			 tcp->u_arg[3], tcp->u_arg[4]);

	return RVAL_DECODED;
}
