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
#include <sys/un.h>
#if defined(HAVE_SIN6_SCOPE_ID_LINUX)
# define in6_addr in6_addr_libc
# define ipv6_mreq ipv6_mreq_libc
# define sockaddr_in6 sockaddr_in6_libc
#endif
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

#if defined(__GLIBC__) && defined(HAVE_SIN6_SCOPE_ID_LINUX)
# if defined(HAVE_LINUX_IN6_H)
#  if defined(HAVE_SIN6_SCOPE_ID_LINUX)
#   undef in6_addr
#   undef ipv6_mreq
#   undef sockaddr_in6
#   define in6_addr in6_addr_kernel
#   define ipv6_mreq ipv6_mreq_kernel
#   define sockaddr_in6 sockaddr_in6_kernel
#  endif
#  include <linux/in6.h>
#  if defined(HAVE_SIN6_SCOPE_ID_LINUX)
#   undef in6_addr
#   undef ipv6_mreq
#   undef sockaddr_in6
#   define in6_addr in6_addr_libc
#   define ipv6_mreq ipv6_mreq_libc
#   define sockaddr_in6 sockaddr_in6_kernel
#  endif
# endif
#endif

#if defined(HAVE_SYS_UIO_H)
# include <sys/uio.h>
#endif
#if defined(HAVE_LINUX_NETLINK_H)
# include <linux/netlink.h>
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
/*** WARNING: DANGER WILL ROBINSON: NOTE "socketlayers" array above
     falls into "inet_protocols" array below!!!!   This is intended!!! ***/
#include "xlat/inet_protocols.h"

#ifdef PF_NETLINK
#include "xlat/netlink_protocols.h"
#endif

#if defined(HAVE_BLUETOOTH_BLUETOOTH_H)
# include "xlat/bt_protocols.h"
#endif

#include "xlat/msg_flags.h"
#include "xlat/sockoptions.h"

#if !defined(SOL_IP) && defined(IPPROTO_IP)
#define SOL_IP IPPROTO_IP
#endif

#ifdef SOL_IP
#include "xlat/sockipoptions.h"
#endif /* SOL_IP */

#ifdef SOL_IPV6
#include "xlat/sockipv6options.h"
#endif /* SOL_IPV6 */

#ifdef SOL_IPX
#include "xlat/sockipxoptions.h"
#endif /* SOL_IPX */

#ifdef SOL_RAW
#include "xlat/sockrawoptions.h"
#endif /* SOL_RAW */

#ifdef SOL_PACKET
#include "xlat/sockpacketoptions.h"
#endif /* SOL_PACKET */

#ifdef SOL_SCTP
#include "xlat/socksctpoptions.h"
#endif

#if !defined(SOL_TCP) && defined(IPPROTO_TCP)
#define SOL_TCP IPPROTO_TCP
#endif

#ifdef SOL_TCP
#include "xlat/socktcpoptions.h"
#endif /* SOL_TCP */

#ifdef SOL_RAW
#include "xlat/icmpfilterflags.h"
#endif /* SOL_RAW */

#if defined(AF_PACKET) /* from e.g. linux/if_packet.h */
#include "xlat/af_packet_types.h"
#endif /* defined(AF_PACKET) */

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

	if (addr == 0) {
		tprints("NULL");
		return;
	}
	if (!verbose(tcp)) {
		tprintf("%#lx", addr);
		return;
	}

	if (addrlen < 2 || addrlen > (int) sizeof(addrbuf))
		addrlen = sizeof(addrbuf);

	memset(&addrbuf, 0, sizeof(addrbuf));
	if (umoven(tcp, addr, addrlen, addrbuf.pad) < 0) {
		tprints("{...}");
		return;
	}
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
			printpathn(tcp, addr + 2, strlen(addrbuf.sau.sun_path));
		} else {
			tprints("sun_path=@");
			printpathn(tcp, addr + 3, strlen(addrbuf.sau.sun_path + 1));
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
		{
#if defined(HAVE_IF_INDEXTONAME) && defined(IN6_IS_ADDR_LINKLOCAL) && defined(IN6_IS_ADDR_MC_LINKLOCAL)
			int numericscope = 0;
			if (IN6_IS_ADDR_LINKLOCAL(&addrbuf.sa6.sin6_addr)
			    || IN6_IS_ADDR_MC_LINKLOCAL(&addrbuf.sa6.sin6_addr)) {
				char scopebuf[IFNAMSIZ + 1];

				if (if_indextoname(addrbuf.sa6.sin6_scope_id, scopebuf) == NULL)
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
		printstr(tcp, (long) &((struct sockaddr *) addr)->sa_data,
			sizeof addrbuf.sa.sa_data);
		break;
	}
	tprints("}");
}

#if HAVE_SENDMSG
#include "xlat/scmvals.h"

static void
printcmsghdr(struct tcb *tcp, unsigned long addr, unsigned long len)
{
	struct cmsghdr *cmsg = len < sizeof(struct cmsghdr) ?
			       NULL : malloc(len);
	if (cmsg == NULL || umoven(tcp, addr, len, (char *) cmsg) < 0) {
		tprintf(", msg_control=%#lx", addr);
		free(cmsg);
		return;
	}

	tprintf(", {cmsg_len=%u, cmsg_level=", (unsigned) cmsg->cmsg_len);
	printxval(socketlayers, cmsg->cmsg_level, "SOL_???");
	tprints(", cmsg_type=");

	if (cmsg->cmsg_level == SOL_SOCKET) {
		unsigned long cmsg_len;

		printxval(scmvals, cmsg->cmsg_type, "SCM_???");
		cmsg_len = (len < cmsg->cmsg_len) ? len : cmsg->cmsg_len;

		if (cmsg->cmsg_type == SCM_RIGHTS
		    && CMSG_LEN(sizeof(int)) <= cmsg_len) {
			int *fds = (int *) CMSG_DATA(cmsg);
			int first = 1;

			tprints(", {");
			while ((char *) fds < ((char *) cmsg + cmsg_len)) {
				if (!first)
					tprints(", ");
				printfd(tcp, *fds++);
				first = 0;
			}
			tprints("}}");
			free(cmsg);
			return;
		}
		if (cmsg->cmsg_type == SCM_CREDENTIALS
		    && CMSG_LEN(sizeof(struct ucred)) <= cmsg_len) {
			struct ucred *uc = (struct ucred *) CMSG_DATA(cmsg);

			tprintf("{pid=%ld, uid=%ld, gid=%ld}}",
				(long)uc->pid, (long)uc->uid, (long)uc->gid);
			free(cmsg);
			return;
		}
	}
	free(cmsg);
	tprints(", ...}");
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

	if (extractmsghdr(tcp, addr, &msg))
		do_msghdr(tcp, &msg, data_size);
	else
		tprintf("%#lx", addr);
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
		tprintf("%#lx", addr);
}

static void
decode_mmsg(struct tcb *tcp, unsigned long msg_len)
{
	/* mmsgvec */
	if (syserror(tcp)) {
		tprintf("%#lx", tcp->u_arg[1]);
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

int
sys_socket(struct tcb *tcp)
{
	if (entering(tcp)) {
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
	}
	return 0;
}

int
sys_bind(struct tcb *tcp)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		printsock(tcp, tcp->u_arg[1], tcp->u_arg[2]);
		tprintf(", %lu", tcp->u_arg[2]);
	}
	return 0;
}

int
sys_connect(struct tcb *tcp)
{
	return sys_bind(tcp);
}

int
sys_listen(struct tcb *tcp)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		tprintf("%lu", tcp->u_arg[1]);
	}
	return 0;
}

static int
do_sockname(struct tcb *tcp, int flags_arg)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		return 0;
	}
	if (!tcp->u_arg[2])
		tprintf("%#lx, NULL", tcp->u_arg[1]);
	else {
		int len;
		if (tcp->u_arg[1] == 0 || syserror(tcp)
		    || umove(tcp, tcp->u_arg[2], &len) < 0) {
			tprintf("%#lx", tcp->u_arg[1]);
		} else {
			printsock(tcp, tcp->u_arg[1], len);
		}
		tprints(", ");
		printnum_int(tcp, tcp->u_arg[2], "%u");
	}
	if (flags_arg >= 0) {
		tprints(", ");
		printflags(sock_type_flags, tcp->u_arg[flags_arg],
			   "SOCK_???");
	}
	return 0;
}

int
sys_accept(struct tcb *tcp)
{
	do_sockname(tcp, -1);
	return RVAL_FD;
}

int
sys_accept4(struct tcb *tcp)
{
	do_sockname(tcp, 3);
	return RVAL_FD;
}

int
sys_send(struct tcb *tcp)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		printstr(tcp, tcp->u_arg[1], tcp->u_arg[2]);
		tprintf(", %lu, ", tcp->u_arg[2]);
		/* flags */
		printflags(msg_flags, tcp->u_arg[3], "MSG_???");
	}
	return 0;
}

int
sys_sendto(struct tcb *tcp)
{
	if (entering(tcp)) {
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
	}
	return 0;
}

#ifdef HAVE_SENDMSG

int
sys_sendmsg(struct tcb *tcp)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		printmsghdr(tcp, tcp->u_arg[1], (unsigned long) -1L);
		/* flags */
		tprints(", ");
		printflags(msg_flags, tcp->u_arg[2], "MSG_???");
	}
	return 0;
}

int
sys_sendmmsg(struct tcb *tcp)
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

int
sys_recv(struct tcb *tcp)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[1]);
		else
			printstr(tcp, tcp->u_arg[1], tcp->u_rval);

		tprintf(", %lu, ", tcp->u_arg[2]);
		printflags(msg_flags, tcp->u_arg[3], "MSG_???");
	}
	return 0;
}

int
sys_recvfrom(struct tcb *tcp)
{
	int fromlen;

	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
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
		printflags(msg_flags, tcp->u_arg[3], "MSG_???");
		/* from address, len */
		if (!tcp->u_arg[4] || !tcp->u_arg[5]) {
			if (tcp->u_arg[4] == 0)
				tprints(", NULL");
			else
				tprintf(", %#lx", tcp->u_arg[4]);
			if (tcp->u_arg[5] == 0)
				tprints(", NULL");
			else
				tprintf(", %#lx", tcp->u_arg[5]);
			return 0;
		}
		if (umove(tcp, tcp->u_arg[5], &fromlen) < 0) {
			tprints(", {...}, [?]");
			return 0;
		}
		tprints(", ");
		printsock(tcp, tcp->u_arg[4], tcp->u_arg[5]);
		/* from length */
		tprintf(", [%u]", fromlen);
	}
	return 0;
}

#ifdef HAVE_SENDMSG

int
sys_recvmsg(struct tcb *tcp)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		if (syserror(tcp) || !verbose(tcp))
			tprintf("%#lx", tcp->u_arg[1]);
		else
			printmsghdr(tcp, tcp->u_arg[1], tcp->u_rval);
		/* flags */
		tprints(", ");
		printflags(msg_flags, tcp->u_arg[2], "MSG_???");
	}
	return 0;
}

int
sys_recvmmsg(struct tcb *tcp)
{
	/* +5 chars are for "left " prefix */
	static char str[5 + TIMESPEC_TEXT_BUFSIZE];

	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		if (verbose(tcp)) {
			sprint_timespec(str, tcp, tcp->u_arg[4]);
			/* Abusing tcp->auxstr as temp storage.
			 * Will be used and freed on syscall exit.
			 */
			tcp->auxstr = strdup(str);
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
			/* timeout on entrance */
			tprintf(", %s", tcp->auxstr ? tcp->auxstr : "{...}");
			free((void *) tcp->auxstr);
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
		sprint_timespec(stpcpy(str, "left "), tcp, tcp->u_arg[4]);
		tcp->auxstr = str;
		return RVAL_STR;
	}
}

#endif /* HAVE_SENDMSG */

#include "xlat/shutdown_modes.h"

int
sys_shutdown(struct tcb *tcp)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		printxval(shutdown_modes, tcp->u_arg[1], "SHUT_???");
	}
	return 0;
}

int
sys_getsockname(struct tcb *tcp)
{
	return do_sockname(tcp, -1);
}

int
sys_getpeername(struct tcb *tcp)
{
	return do_sockname(tcp, -1);
}

static int
do_pipe(struct tcb *tcp, int flags_arg)
{
	if (exiting(tcp)) {
		if (syserror(tcp)) {
			tprintf("%#lx", tcp->u_arg[0]);
		} else {
#if !defined(SPARC) && !defined(SPARC64) && !defined(SH) && !defined(IA64)
			int fds[2];

			if (umoven(tcp, tcp->u_arg[0], sizeof fds, (char *) fds) < 0)
				tprints("[...]");
			else
				tprintf("[%u, %u]", fds[0], fds[1]);
#elif defined(SPARC) || defined(SPARC64) || defined(SH) || defined(IA64)
			tprintf("[%lu, %lu]", tcp->u_rval, getrval2(tcp));
#else
			tprintf("%#lx", tcp->u_arg[0]);
#endif
		}
		if (flags_arg >= 0) {
			tprints(", ");
			printflags(open_mode_flags, tcp->u_arg[flags_arg], "O_???");
		}
	}
	return 0;
}

int
sys_pipe(struct tcb *tcp)
{
	return do_pipe(tcp, -1);
}

int
sys_pipe2(struct tcb *tcp)
{
	return do_pipe(tcp, 1);
}

int
sys_socketpair(struct tcb *tcp)
{
	int fds[2];

	if (entering(tcp)) {
		printxval(domains, tcp->u_arg[0], "PF_???");
		tprints(", ");
		tprint_sock_type(tcp->u_arg[1]);
		tprintf(", %lu", tcp->u_arg[2]);
	} else {
		if (syserror(tcp)) {
			tprintf(", %#lx", tcp->u_arg[3]);
			return 0;
		}
		if (umoven(tcp, tcp->u_arg[3], sizeof fds, (char *) fds) < 0)
			tprints(", [...]");
		else
			tprintf(", [%u, %u]", fds[0], fds[1]);
	}
	return 0;
}

int
sys_getsockopt(struct tcb *tcp)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		printxval(socketlayers, tcp->u_arg[1], "SOL_???");
		tprints(", ");
		switch (tcp->u_arg[1]) {
		case SOL_SOCKET:
			printxval(sockoptions, tcp->u_arg[2], "SO_???");
			break;
#ifdef SOL_IP
		case SOL_IP:
			printxval(sockipoptions, tcp->u_arg[2], "IP_???");
			break;
#endif
#ifdef SOL_IPV6
		case SOL_IPV6:
			printxval(sockipv6options, tcp->u_arg[2], "IPV6_???");
			break;
#endif
#ifdef SOL_IPX
		case SOL_IPX:
			printxval(sockipxoptions, tcp->u_arg[2], "IPX_???");
			break;
#endif
#ifdef SOL_PACKET
		case SOL_PACKET:
			printxval(sockpacketoptions, tcp->u_arg[2], "PACKET_???");
			break;
#endif
#ifdef SOL_TCP
		case SOL_TCP:
			printxval(socktcpoptions, tcp->u_arg[2], "TCP_???");
			break;
#endif
#ifdef SOL_SCTP
		case SOL_SCTP:
			printxval(socksctpoptions, tcp->u_arg[2], "SCTP_???");
			break;
#endif

		/* SOL_AX25 SOL_ROSE SOL_ATALK SOL_NETROM SOL_UDP SOL_DECNET SOL_X25
		 * etc. still need work */
		default:
			tprintf("%lu", tcp->u_arg[2]);
			break;
		}
		tprints(", ");
	} else {
		int len;
		if (syserror(tcp) || umove(tcp, tcp->u_arg[4], &len) < 0) {
			tprintf("%#lx, %#lx",
				tcp->u_arg[3], tcp->u_arg[4]);
			return 0;
		}

		switch (tcp->u_arg[1]) {
		case SOL_SOCKET:
			switch (tcp->u_arg[2]) {
#ifdef SO_LINGER
			case SO_LINGER:
				if (len == sizeof(struct linger)) {
					struct linger linger;
					if (umove(tcp,
						   tcp->u_arg[3],
						   &linger) < 0)
						break;
					tprintf("{onoff=%d, linger=%d}, "
						"[%d]",
						linger.l_onoff,
						linger.l_linger,
						len);
					return 0;
				}
				break;
#endif
#ifdef SO_PEERCRED
			case SO_PEERCRED:
				if (len == sizeof(struct ucred)) {
					struct ucred uc;
					if (umove(tcp,
						   tcp->u_arg[3],
						   &uc) < 0)
						break;
					tprintf("{pid=%ld, uid=%ld, gid=%ld}, "
						"[%d]",
						(long)uc.pid,
						(long)uc.uid,
						(long)uc.gid,
						len);
					return 0;
				}
				break;
#endif
			}
			break;
		case SOL_PACKET:
			switch (tcp->u_arg[2]) {
#ifdef PACKET_STATISTICS
			case PACKET_STATISTICS:
				if (len == sizeof(struct tpacket_stats)) {
					struct tpacket_stats stats;
					if (umove(tcp,
						   tcp->u_arg[3],
						   &stats) < 0)
						break;
					tprintf("{packets=%u, drops=%u}, "
						"[%d]",
						stats.tp_packets,
						stats.tp_drops,
						len);
					return 0;
				}
				break;
#endif
			}
			break;
		}

		if (len == sizeof(int)) {
			printnum_int(tcp, tcp->u_arg[3], "%d");
		}
		else {
			printstr(tcp, tcp->u_arg[3], len);
		}
		tprintf(", [%d]", len);
	}
	return 0;
}

#if defined(ICMP_FILTER)
static void printicmpfilter(struct tcb *tcp, long addr)
{
	struct icmp_filter	filter;

	if (!addr) {
		tprints("NULL");
		return;
	}
	if (syserror(tcp) || !verbose(tcp)) {
		tprintf("%#lx", addr);
		return;
	}
	if (umove(tcp, addr, &filter) < 0) {
		tprints("{...}");
		return;
	}

	tprints("~(");
	printflags(icmpfilterflags, ~filter.data, "ICMP_???");
	tprints(")");
}
#endif /* ICMP_FILTER */

static int
printsockopt(struct tcb *tcp, int level, int name, long addr, int len)
{
	printxval(socketlayers, level, "SOL_??");
	tprints(", ");
	switch (level) {
	case SOL_SOCKET:
		printxval(sockoptions, name, "SO_???");
		switch (name) {
#if defined(SO_LINGER)
		case SO_LINGER:
			if (len == sizeof(struct linger)) {
				struct linger linger;
				if (umove(tcp, addr, &linger) < 0)
					break;
				tprintf(", {onoff=%d, linger=%d}",
					linger.l_onoff,
					linger.l_linger);
				return 0;
			}
			break;
#endif
		}
		break;
#ifdef SOL_IP
	case SOL_IP:
		printxval(sockipoptions, name, "IP_???");
		break;
#endif
#ifdef SOL_IPV6
	case SOL_IPV6:
		printxval(sockipv6options, name, "IPV6_???");
		break;
#endif
#ifdef SOL_IPX
	case SOL_IPX:
		printxval(sockipxoptions, name, "IPX_???");
		break;
#endif
#ifdef SOL_PACKET
	case SOL_PACKET:
		printxval(sockpacketoptions, name, "PACKET_???");
		/* TODO: decode packate_mreq for PACKET_*_MEMBERSHIP */
		switch (name) {
#ifdef PACKET_RX_RING
		case PACKET_RX_RING:
#endif
#ifdef PACKET_TX_RING
		case PACKET_TX_RING:
#endif
#if defined(PACKET_RX_RING) || defined(PACKET_TX_RING)
			if (len == sizeof(struct tpacket_req)) {
				struct tpacket_req req;
				if (umove(tcp, addr, &req) < 0)
					break;
				tprintf(", {block_size=%u, block_nr=%u, frame_size=%u, frame_nr=%u}",
					req.tp_block_size,
					req.tp_block_nr,
					req.tp_frame_size,
					req.tp_frame_nr);
				return 0;
			}
			break;
#endif /* PACKET_RX_RING || PACKET_TX_RING */
		}
		break;
#endif
#ifdef SOL_TCP
	case SOL_TCP:
		printxval(socktcpoptions, name, "TCP_???");
		break;
#endif
#ifdef SOL_SCTP
	case SOL_SCTP:
		printxval(socksctpoptions, name, "SCTP_???");
		break;
#endif
#ifdef SOL_RAW
	case SOL_RAW:
		printxval(sockrawoptions, name, "RAW_???");
		switch (name) {
#if defined(ICMP_FILTER)
			case ICMP_FILTER:
				tprints(", ");
				printicmpfilter(tcp, addr);
				return 0;
#endif
		}
		break;
#endif

		/* SOL_AX25 SOL_ATALK SOL_NETROM SOL_UDP SOL_DECNET SOL_X25
		 * etc. still need work  */

	default:
		tprintf("%u", name);
	}

	/* default arg printing */

	tprints(", ");

	if (len == sizeof(int)) {
		printnum_int(tcp, addr, "%d");
	}
	else {
		printstr(tcp, addr, len);
	}
	return 0;
}

int
sys_setsockopt(struct tcb *tcp)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		printsockopt(tcp, tcp->u_arg[1], tcp->u_arg[2],
			      tcp->u_arg[3], tcp->u_arg[4]);
		tprintf(", %lu", tcp->u_arg[4]);
	}
	return 0;
}
