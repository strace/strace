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
#ifndef PF_UNSPEC
# define PF_UNSPEC AF_UNSPEC
#endif

static const struct xlat domains[] = {
#ifdef PF_UNSPEC
	XLAT(PF_UNSPEC),
#endif
#ifdef PF_LOCAL
	XLAT(PF_LOCAL),
#endif
#ifdef PF_UNIX
	XLAT(PF_UNIX),
#endif
#ifdef PF_INET
	XLAT(PF_INET),
#endif
#ifdef PF_AX25
	XLAT(PF_AX25),
#endif
#ifdef PF_IPX
	XLAT(PF_IPX),
#endif
#ifdef PF_APPLETALK
	XLAT(PF_APPLETALK),
#endif
#ifdef PF_NETROM
	XLAT(PF_NETROM),
#endif
#ifdef PF_BRIDGE
	XLAT(PF_BRIDGE),
#endif
#ifdef PF_ATMPVC
	XLAT(PF_ATMPVC),
#endif
#ifdef PF_X25
	XLAT(PF_X25),
#endif
#ifdef PF_INET6
	XLAT(PF_INET6),
#endif
#ifdef PF_ROSE
	XLAT(PF_ROSE),
#endif
#ifdef PF_DECnet
	XLAT(PF_DECnet),
#endif
#ifdef PF_NETBEUI
	XLAT(PF_NETBEUI),
#endif
#ifdef PF_SECURITY
	XLAT(PF_SECURITY),
#endif
#ifdef PF_KEY
	XLAT(PF_KEY),
#endif
#ifdef PF_NETLINK
	XLAT(PF_NETLINK),
#endif
#ifdef PF_ROUTE
	XLAT(PF_ROUTE),
#endif
#ifdef PF_PACKET
	XLAT(PF_PACKET),
#endif
#ifdef PF_ASH
	XLAT(PF_ASH),
#endif
#ifdef PF_ECONET
	XLAT(PF_ECONET),
#endif
#ifdef PF_ATMSVC
	XLAT(PF_ATMSVC),
#endif
#ifdef PF_RDS
	XLAT(PF_RDS),
#endif
#ifdef PF_SNA
	XLAT(PF_SNA),
#endif
#ifdef PF_IRDA
	XLAT(PF_IRDA),
#endif
#ifdef PF_PPPOX
	XLAT(PF_PPPOX),
#endif
#ifdef PF_WANPIPE
	XLAT(PF_WANPIPE),
#endif
#ifdef PF_LLC
	XLAT(PF_LLC),
#endif
#ifdef PF_CAN
	XLAT(PF_CAN),
#endif
#ifdef PF_TIPC
	XLAT(PF_TIPC),
#endif
#ifdef PF_BLUETOOTH
	XLAT(PF_BLUETOOTH),
#endif
#ifdef PF_IUCV
	XLAT(PF_IUCV),
#endif
#ifdef PF_RXRPC
	XLAT(PF_RXRPC),
#endif
#ifdef PF_ISDN
	XLAT(PF_ISDN),
#endif
#ifdef PF_PHONET
	XLAT(PF_PHONET),
#endif
#ifdef PF_IEEE802154
	XLAT(PF_IEEE802154),
#endif
#ifdef PF_CAIF
	XLAT(PF_CAIF),
#endif
#ifdef PF_ALG
	XLAT(PF_ALG),
#endif
#ifdef PF_NFC
	XLAT(PF_NFC),
#endif
#ifdef PF_VSOCK
	XLAT(PF_VSOCK),
#endif
	XLAT_END
};
const struct xlat addrfams[] = {
#ifdef AF_UNSPEC
	XLAT(AF_UNSPEC),
#endif
#ifdef AF_LOCAL
	XLAT(AF_LOCAL),
#endif
#ifdef AF_UNIX
	XLAT(AF_UNIX),
#endif
#ifdef AF_INET
	XLAT(AF_INET),
#endif
#ifdef AF_AX25
	XLAT(AF_AX25),
#endif
#ifdef AF_IPX
	XLAT(AF_IPX),
#endif
#ifdef AF_APPLETALK
	XLAT(AF_APPLETALK),
#endif
#ifdef AF_NETROM
	XLAT(AF_NETROM),
#endif
#ifdef AF_BRIDGE
	XLAT(AF_BRIDGE),
#endif
#ifdef AF_ATMPVC
	XLAT(AF_ATMPVC),
#endif
#ifdef AF_X25
	XLAT(AF_X25),
#endif
#ifdef AF_INET6
	XLAT(AF_INET6),
#endif
#ifdef AF_ROSE
	XLAT(AF_ROSE),
#endif
#ifdef AF_DECnet
	XLAT(AF_DECnet),
#endif
#ifdef AF_NETBEUI
	XLAT(AF_NETBEUI),
#endif
#ifdef AF_SECURITY
	XLAT(AF_SECURITY),
#endif
#ifdef AF_KEY
	XLAT(AF_KEY),
#endif
#ifdef AF_NETLINK
	XLAT(AF_NETLINK),
#endif
#ifdef AF_ROUTE
	XLAT(AF_ROUTE),
#endif
#ifdef AF_PACKET
	XLAT(AF_PACKET),
#endif
#ifdef AF_ASH
	XLAT(AF_ASH),
#endif
#ifdef AF_ECONET
	XLAT(AF_ECONET),
#endif
#ifdef AF_ATMSVC
	XLAT(AF_ATMSVC),
#endif
#ifdef AF_RDS
	XLAT(AF_RDS),
#endif
#ifdef AF_SNA
	XLAT(AF_SNA),
#endif
#ifdef AF_IRDA
	XLAT(AF_IRDA),
#endif
#ifdef AF_PPPOX
	XLAT(AF_PPPOX),
#endif
#ifdef AF_WANPIPE
	XLAT(AF_WANPIPE),
#endif
#ifdef AF_LLC
	XLAT(AF_LLC),
#endif
#ifdef AF_CAN
	XLAT(AF_CAN),
#endif
#ifdef AF_TIPC
	XLAT(AF_TIPC),
#endif
#ifdef AF_BLUETOOTH
	XLAT(AF_BLUETOOTH),
#endif
#ifdef AF_IUCV
	XLAT(AF_IUCV),
#endif
#ifdef AF_RXRPC
	XLAT(AF_RXRPC),
#endif
#ifdef AF_ISDN
	XLAT(AF_ISDN),
#endif
#ifdef AF_PHONET
	XLAT(AF_PHONET),
#endif
#ifdef AF_IEEE802154
	XLAT(AF_IEEE802154),
#endif
#ifdef AF_CAIF
	XLAT(AF_CAIF),
#endif
#ifdef AF_ALG
	XLAT(AF_ALG),
#endif
#ifdef AF_NFC
	XLAT(AF_NFC),
#endif
#ifdef AF_VSOCK
	XLAT(AF_VSOCK),
#endif
	XLAT_END
};
static const struct xlat socktypes[] = {
	XLAT(SOCK_STREAM),
	XLAT(SOCK_DGRAM),
#ifdef SOCK_RAW
	XLAT(SOCK_RAW),
#endif
#ifdef SOCK_RDM
	XLAT(SOCK_RDM),
#endif
#ifdef SOCK_SEQPACKET
	XLAT(SOCK_SEQPACKET),
#endif
#ifdef SOCK_DCCP
	XLAT(SOCK_DCCP),
#endif
#ifdef SOCK_PACKET
	XLAT(SOCK_PACKET),
#endif
	XLAT_END
};
static const struct xlat sock_type_flags[] = {
#ifdef SOCK_CLOEXEC
	XLAT(SOCK_CLOEXEC),
#endif
#ifdef SOCK_NONBLOCK
	XLAT(SOCK_NONBLOCK),
#endif
	XLAT_END
};
#ifndef SOCK_TYPE_MASK
# define SOCK_TYPE_MASK 0xf
#endif
static const struct xlat socketlayers[] = {
#if defined(SOL_IP)
	XLAT(SOL_IP),
#endif
#if defined(SOL_ICMP)
	XLAT(SOL_ICMP),
#endif
#if defined(SOL_TCP)
	XLAT(SOL_TCP),
#endif
#if defined(SOL_UDP)
	XLAT(SOL_UDP),
#endif
#if defined(SOL_IPV6)
	XLAT(SOL_IPV6),
#endif
#if defined(SOL_ICMPV6)
	XLAT(SOL_ICMPV6),
#endif
#if defined(SOL_SCTP)
	XLAT(SOL_SCTP),
#endif
#if defined(SOL_UDPLITE)
	XLAT(SOL_UDPLITE),
#endif
#if defined(SOL_RAW)
	XLAT(SOL_RAW),
#endif
#if defined(SOL_IPX)
	XLAT(SOL_IPX),
#endif
#if defined(SOL_AX25)
	XLAT(SOL_AX25),
#endif
#if defined(SOL_ATALK)
	XLAT(SOL_ATALK),
#endif
#if defined(SOL_NETROM)
	XLAT(SOL_NETROM),
#endif
#if defined(SOL_ROSE)
	XLAT(SOL_ROSE),
#endif
#if defined(SOL_DECNET)
	XLAT(SOL_DECNET),
#endif
#if defined(SOL_X25)
	XLAT(SOL_X25),
#endif
#if defined(SOL_PACKET)
	XLAT(SOL_PACKET),
#endif
#if defined(SOL_ATM)
	XLAT(SOL_ATM),
#endif
#if defined(SOL_AAL)
	XLAT(SOL_AAL),
#endif
#if defined(SOL_IRDA)
	XLAT(SOL_IRDA),
#endif
#if defined(SOL_NETBEUI)
	XLAT(SOL_NETBEUI),
#endif
#if defined(SOL_LLC)
	XLAT(SOL_LLC),
#endif
#if defined(SOL_DCCP)
	XLAT(SOL_DCCP),
#endif
#if defined(SOL_NETLINK)
	XLAT(SOL_NETLINK),
#endif
#if defined(SOL_TIPC)
	XLAT(SOL_TIPC),
#endif
#if defined(SOL_RXRPC)
	XLAT(SOL_RXRPC),
#endif
#if defined(SOL_PPPOL2TP)
	XLAT(SOL_PPPOL2TP),
#endif
#if defined(SOL_BLUETOOTH)
	XLAT(SOL_BLUETOOTH),
#endif
#if defined(SOL_PNPIPE)
	XLAT(SOL_PNPIPE),
#endif
#if defined(SOL_RDS)
	XLAT(SOL_RDS),
#endif
#if defined(SOL_IUVC)
	XLAT(SOL_IUCV),
#endif
#if defined(SOL_CAIF)
	XLAT(SOL_CAIF),
#endif
	XLAT(SOL_SOCKET),	/* Never used! */
	/* The SOL_* array should remain not NULL-terminated. */
};
/*** WARNING: DANGER WILL ROBINSON: NOTE "socketlayers" array above
     falls into "inet_protocols" array below!!!!   This is intended!!! ***/
static const struct xlat inet_protocols[] = {
	XLAT(IPPROTO_IP),
	XLAT(IPPROTO_ICMP),
	XLAT(IPPROTO_TCP),
	XLAT(IPPROTO_UDP),
#ifdef IPPROTO_IGMP
	XLAT(IPPROTO_IGMP),
#endif
#ifdef IPPROTO_GGP
	XLAT(IPPROTO_GGP),
#endif
#ifdef IPPROTO_IPIP
	XLAT(IPPROTO_IPIP),
#endif
#ifdef IPPROTO_EGP
	XLAT(IPPROTO_EGP),
#endif
#ifdef IPPROTO_PUP
	XLAT(IPPROTO_PUP),
#endif
#ifdef IPPROTO_IDP
	XLAT(IPPROTO_IDP),
#endif
#ifdef IPPROTO_TP
	XLAT(IPPROTO_TP),
#endif
#ifdef IPPROTO_DCCP
	XLAT(IPPROTO_DCCP),
#endif
#ifdef IPPROTO_IPV6
	XLAT(IPPROTO_IPV6),
#endif
#ifdef IPPROTO_ROUTING
	XLAT(IPPROTO_ROUTING),
#endif
#ifdef IPPROTO_FRAGMENT
	XLAT(IPPROTO_FRAGMENT),
#endif
#ifdef IPPROTO_RSVP
	XLAT(IPPROTO_RSVP),
#endif
#ifdef IPPROTO_GRE
	XLAT(IPPROTO_GRE),
#endif
#ifdef IPPROTO_ESP
	XLAT(IPPROTO_ESP),
#endif
#ifdef IPPROTO_AH
	XLAT(IPPROTO_AH),
#endif
#ifdef IPPROTO_ICMPV6
	XLAT(IPPROTO_ICMPV6),
#endif
#ifdef IPPROTO_NONE
	XLAT(IPPROTO_NONE),
#endif
#ifdef IPPROTO_DSTOPTS
	XLAT(IPPROTO_DSTOPTS),
#endif
#ifdef IPPROTO_HELLO
	XLAT(IPPROTO_HELLO),
#endif
#ifdef IPPROTO_ND
	XLAT(IPPROTO_ND),
#endif
#ifdef IPPROTO_MTP
	XLAT(IPPROTO_MTP),
#endif
#ifdef IPPROTO_ENCAP
	XLAT(IPPROTO_ENCAP),
#endif
#ifdef IPPROTO_PIM
	XLAT(IPPROTO_PIM),
#endif
#ifdef IPPROTO_COMP
	XLAT(IPPROTO_COMP),
#endif
#ifdef IPPROTO_SCTP
	XLAT(IPPROTO_SCTP),
#endif
#ifdef IPPROTO_UDPLITE
	XLAT(IPPROTO_UDPLITE),
#endif
#ifdef IPPROTO_RAW
	XLAT(IPPROTO_RAW),
#endif
#ifdef IPPROTO_MAX
	XLAT(IPPROTO_MAX),
#endif
	XLAT_END
};

#ifdef PF_NETLINK
static const struct xlat netlink_protocols[] = {
#ifdef NETLINK_ROUTE
	XLAT(NETLINK_ROUTE),
#endif
#ifdef NETLINK_UNUSED
	XLAT(NETLINK_UNUSED),
#endif
#ifdef NETLINK_USERSOCK
	XLAT(NETLINK_USERSOCK),
#endif
#ifdef NETLINK_FIREWALL
	XLAT(NETLINK_FIREWALL),
#endif
#ifdef NETLINK_SOCK_DIAG
	XLAT(NETLINK_SOCK_DIAG),
#endif
#ifdef NETLINK_NFLOG
	XLAT(NETLINK_NFLOG),
#endif
#ifdef NETLINK_XFRM
	XLAT(NETLINK_XFRM),
#endif
#ifdef NETLINK_SELINUX
	XLAT(NETLINK_SELINUX),
#endif
#ifdef NETLINK_ISCSI
	XLAT(NETLINK_ISCSI),
#endif
#ifdef NETLINK_AUDIT
	XLAT(NETLINK_AUDIT),
#endif
#ifdef NETLINK_FIB_LOOKUP
	XLAT(NETLINK_FIB_LOOKUP),
#endif
#ifdef NETLINK_CONNECTOR
	XLAT(NETLINK_CONNECTOR),
#endif
#ifdef NETLINK_NETFILTER
	XLAT(NETLINK_NETFILTER),
#endif
#ifdef NETLINK_IP6_FW
	XLAT(NETLINK_IP6_FW),
#endif
#ifdef NETLINK_DNRTMSG
	XLAT(NETLINK_DNRTMSG),
#endif
#ifdef NETLINK_KOBJECT_UEVENT
	XLAT(NETLINK_KOBJECT_UEVENT),
#endif
#ifdef NETLINK_GENERIC
	XLAT(NETLINK_GENERIC),
#endif
#ifdef NETLINK_SCSITRANSPORT
	XLAT(NETLINK_SCSITRANSPORT),
#endif
#ifdef NETLINK_ECRYPTFS
	XLAT(NETLINK_ECRYPTFS),
#endif
#ifdef NETLINK_RDMA
	XLAT(NETLINK_RDMA),
#endif
#ifdef NETLINK_CRYPTO
	XLAT(NETLINK_CRYPTO),
#endif
	XLAT_END
};
#endif

static const struct xlat msg_flags[] = {
	XLAT(MSG_OOB),
#ifdef MSG_PEEK
	XLAT(MSG_PEEK),
#endif
#ifdef MSG_DONTROUTE
	XLAT(MSG_DONTROUTE),
#endif
#ifdef MSG_CTRUNC
	XLAT(MSG_CTRUNC),
#endif
#ifdef MSG_PROBE
	XLAT(MSG_PROBE),
#endif
#ifdef MSG_TRUNC
	XLAT(MSG_TRUNC),
#endif
#ifdef MSG_DONTWAIT
	XLAT(MSG_DONTWAIT),
#endif
#ifdef MSG_EOR
	XLAT(MSG_EOR),
#endif
#ifdef MSG_WAITALL
	XLAT(MSG_WAITALL),
#endif
#ifdef MSG_FIN
	XLAT(MSG_FIN),
#endif
#ifdef MSG_SYN
	XLAT(MSG_SYN),
#endif
#ifdef MSG_CONFIRM
	XLAT(MSG_CONFIRM),
#endif
#ifdef MSG_RST
	XLAT(MSG_RST),
#endif
#ifdef MSG_ERRQUEUE
	XLAT(MSG_ERRQUEUE),
#endif
#ifdef MSG_NOSIGNAL
	XLAT(MSG_NOSIGNAL),
#endif
#ifdef MSG_MORE
	XLAT(MSG_MORE),
#endif
#ifdef MSG_WAITFORONE
	XLAT(MSG_WAITFORONE),
#endif
#ifdef MSG_EOF
	XLAT(MSG_EOF),
#endif
#ifdef MSG_FASTOPEN
	XLAT(MSG_FASTOPEN),
#endif
#ifdef MSG_CMSG_CLOEXEC
	XLAT(MSG_CMSG_CLOEXEC),
#endif
	XLAT_END
};

static const struct xlat sockoptions[] = {
#ifdef SO_ACCEPTCONN
	XLAT(SO_ACCEPTCONN),
#endif
#ifdef SO_ALLRAW
	XLAT(SO_ALLRAW),
#endif
#ifdef SO_ATTACH_FILTER
	XLAT(SO_ATTACH_FILTER),
#endif
#ifdef SO_BINDTODEVICE
	XLAT(SO_BINDTODEVICE),
#endif
#ifdef SO_BROADCAST
	XLAT(SO_BROADCAST),
#endif
#ifdef SO_BSDCOMPAT
	XLAT(SO_BSDCOMPAT),
#endif
#ifdef SO_DEBUG
	XLAT(SO_DEBUG),
#endif
#ifdef SO_DETACH_FILTER
	XLAT(SO_DETACH_FILTER),
#endif
#ifdef SO_DONTROUTE
	XLAT(SO_DONTROUTE),
#endif
#ifdef SO_ERROR
	XLAT(SO_ERROR),
#endif
#ifdef SO_ICS
	XLAT(SO_ICS),
#endif
#ifdef SO_IMASOCKET
	XLAT(SO_IMASOCKET),
#endif
#ifdef SO_KEEPALIVE
	XLAT(SO_KEEPALIVE),
#endif
#ifdef SO_LINGER
	XLAT(SO_LINGER),
#endif
#ifdef SO_LISTENING
	XLAT(SO_LISTENING),
#endif
#ifdef SO_MGMT
	XLAT(SO_MGMT),
#endif
#ifdef SO_NO_CHECK
	XLAT(SO_NO_CHECK),
#endif
#ifdef SO_OOBINLINE
	XLAT(SO_OOBINLINE),
#endif
#ifdef SO_ORDREL
	XLAT(SO_ORDREL),
#endif
#ifdef SO_PARALLELSVR
	XLAT(SO_PARALLELSVR),
#endif
#ifdef SO_PASSCRED
	XLAT(SO_PASSCRED),
#endif
#ifdef SO_PEERCRED
	XLAT(SO_PEERCRED),
#endif
#ifdef SO_PEERNAME
	XLAT(SO_PEERNAME),
#endif
#ifdef SO_PEERSEC
	XLAT(SO_PEERSEC),
#endif
#ifdef SO_PRIORITY
	XLAT(SO_PRIORITY),
#endif
#ifdef SO_PROTOTYPE
	XLAT(SO_PROTOTYPE),
#endif
#ifdef SO_RCVBUF
	XLAT(SO_RCVBUF),
#endif
#ifdef SO_RCVLOWAT
	XLAT(SO_RCVLOWAT),
#endif
#ifdef SO_RCVTIMEO
	XLAT(SO_RCVTIMEO),
#endif
#ifdef SO_RDWR
	XLAT(SO_RDWR),
#endif
#ifdef SO_REUSEADDR
	XLAT(SO_REUSEADDR),
#endif
#ifdef SO_REUSEPORT
	XLAT(SO_REUSEPORT),
#endif
#ifdef SO_SECURITY_AUTHENTICATION
	XLAT(SO_SECURITY_AUTHENTICATION),
#endif
#ifdef SO_SECURITY_ENCRYPTION_NETWORK
	XLAT(SO_SECURITY_ENCRYPTION_NETWORK),
#endif
#ifdef SO_SECURITY_ENCRYPTION_TRANSPORT
	XLAT(SO_SECURITY_ENCRYPTION_TRANSPORT),
#endif
#ifdef SO_SEMA
	XLAT(SO_SEMA),
#endif
#ifdef SO_SNDBUF
	XLAT(SO_SNDBUF),
#endif
#ifdef SO_SNDLOWAT
	XLAT(SO_SNDLOWAT),
#endif
#ifdef SO_SNDTIMEO
	XLAT(SO_SNDTIMEO),
#endif
#ifdef SO_TIMESTAMP
	XLAT(SO_TIMESTAMP),
#endif
#ifdef SO_TYPE
	XLAT(SO_TYPE),
#endif
#ifdef SO_USELOOPBACK
	XLAT(SO_USELOOPBACK),
#endif
	XLAT_END
};

#if !defined(SOL_IP) && defined(IPPROTO_IP)
#define SOL_IP IPPROTO_IP
#endif

#ifdef SOL_IP
static const struct xlat sockipoptions[] = {
#ifdef IP_TOS
	XLAT(IP_TOS),
#endif
#ifdef IP_TTL
	XLAT(IP_TTL),
#endif
#ifdef IP_HDRINCL
	XLAT(IP_HDRINCL),
#endif
#ifdef IP_OPTIONS
	XLAT(IP_OPTIONS),
#endif
#ifdef IP_ROUTER_ALERT
	XLAT(IP_ROUTER_ALERT),
#endif
#ifdef IP_RECVOPTIONS
	XLAT(IP_RECVOPTIONS),
#endif
#ifdef IP_RECVOPTS
	XLAT(IP_RECVOPTS),
#endif
#ifdef IP_RECVRETOPTS
	XLAT(IP_RECVRETOPTS),
#endif
#ifdef IP_RECVDSTADDR
	XLAT(IP_RECVDSTADDR),
#endif
#ifdef IP_RETOPTS
	XLAT(IP_RETOPTS),
#endif
#ifdef IP_PKTINFO
	XLAT(IP_PKTINFO),
#endif
#ifdef IP_PKTOPTIONS
	XLAT(IP_PKTOPTIONS),
#endif
#ifdef IP_MTU_DISCOVER
	XLAT(IP_MTU_DISCOVER),
#endif
#ifdef IP_RECVERR
	XLAT(IP_RECVERR),
#endif
#ifdef IP_RECVTTL
	XLAT(IP_RECVTTL),
#endif
#ifdef IP_RECVTOS
	XLAT(IP_RECVTOS),
#endif
#ifdef IP_MTU
	XLAT(IP_MTU),
#endif
#ifdef IP_MULTICAST_IF
	XLAT(IP_MULTICAST_IF),
#endif
#ifdef IP_MULTICAST_TTL
	XLAT(IP_MULTICAST_TTL),
#endif
#ifdef IP_MULTICAST_LOOP
	XLAT(IP_MULTICAST_LOOP),
#endif
#ifdef IP_ADD_MEMBERSHIP
	XLAT(IP_ADD_MEMBERSHIP),
#endif
#ifdef IP_DROP_MEMBERSHIP
	XLAT(IP_DROP_MEMBERSHIP),
#endif
#ifdef IP_BROADCAST_IF
	XLAT(IP_BROADCAST_IF),
#endif
#ifdef IP_RECVIFINDEX
	XLAT(IP_RECVIFINDEX),
#endif
#ifdef IP_MSFILTER
	XLAT(IP_MSFILTER),
#endif
#ifdef MCAST_MSFILTER
	XLAT(MCAST_MSFILTER),
#endif
#ifdef IP_FREEBIND
	XLAT(IP_FREEBIND),
#endif
#ifdef IP_IPSEC_POLICY
	XLAT(IP_IPSEC_POLICY),
#endif
#ifdef IP_XFRM_POLICY
	XLAT(IP_XFRM_POLICY),
#endif
#ifdef IP_PASSSEC
	XLAT(IP_PASSSEC),
#endif
#ifdef IP_TRANSPARENT
	XLAT(IP_TRANSPARENT),
#endif
#ifdef IP_ORIGDSTADDR
	XLAT(IP_ORIGDSTADDR),
#endif
#ifdef IP_RECVORIGDSTADDR
	XLAT(IP_RECVORIGDSTADDR),
#endif
#ifdef IP_MINTTL
	XLAT(IP_MINTTL),
#endif
#ifdef IP_NODEFRAG
	XLAT(IP_NODEFRAG),
#endif
#ifdef IP_UNBLOCK_SOURCE
	XLAT(IP_UNBLOCK_SOURCE),
#endif
#ifdef IP_BLOCK_SOURCE
	XLAT(IP_BLOCK_SOURCE),
#endif
#ifdef IP_ADD_SOURCE_MEMBERSHIP
	XLAT(IP_ADD_SOURCE_MEMBERSHIP),
#endif
#ifdef IP_DROP_SOURCE_MEMBERSHIP
	XLAT(IP_DROP_SOURCE_MEMBERSHIP),
#endif
#ifdef MCAST_JOIN_GROUP
	XLAT(MCAST_JOIN_GROUP),
#endif
#ifdef MCAST_BLOCK_SOURCE
	XLAT(MCAST_BLOCK_SOURCE),
#endif
#ifdef MCAST_UNBLOCK_SOURCE
	XLAT(MCAST_UNBLOCK_SOURCE),
#endif
#ifdef MCAST_LEAVE_GROUP
	XLAT(MCAST_LEAVE_GROUP),
#endif
#ifdef MCAST_JOIN_SOURCE_GROUP
	XLAT(MCAST_JOIN_SOURCE_GROUP),
#endif
#ifdef MCAST_LEAVE_SOURCE_GROUP
	XLAT(MCAST_LEAVE_SOURCE_GROUP),
#endif
#ifdef IP_MULTICAST_ALL
	XLAT(IP_MULTICAST_ALL),
#endif
#ifdef IP_UNICAST_IF
	XLAT(IP_UNICAST_IF),
#endif
	XLAT_END
};
#endif /* SOL_IP */

#ifdef SOL_IPV6
static const struct xlat sockipv6options[] = {
#ifdef IPV6_ADDRFORM
	XLAT(IPV6_ADDRFORM),
#endif
#ifdef MCAST_FILTER
	XLAT(MCAST_FILTER),
#endif
#ifdef IPV6_PKTOPTIONS
	XLAT(IPV6_PKTOPTIONS),
#endif
#ifdef IPV6_MTU
	XLAT(IPV6_MTU),
#endif
#ifdef IPV6_V6ONLY
	XLAT(IPV6_V6ONLY),
#endif
#ifdef IPV6_PKTINFO
	XLAT(IPV6_PKTINFO),
#endif
#ifdef IPV6_HOPLIMIT
	XLAT(IPV6_HOPLIMIT),
#endif
#ifdef IPV6_RTHDR
	XLAT(IPV6_RTHDR),
#endif
#ifdef IPV6_HOPOPTS
	XLAT(IPV6_HOPOPTS),
#endif
#ifdef IPV6_DSTOPTS
	XLAT(IPV6_DSTOPTS),
#endif
#ifdef IPV6_FLOWINFO
	XLAT(IPV6_FLOWINFO),
#endif
#ifdef IPV6_UNICAST_HOPS
	XLAT(IPV6_UNICAST_HOPS),
#endif
#ifdef IPV6_MULTICAST_HOPS
	XLAT(IPV6_MULTICAST_HOPS),
#endif
#ifdef IPV6_MULTICAST_LOOP
	XLAT(IPV6_MULTICAST_LOOP),
#endif
#ifdef IPV6_MULTICAST_IF
	XLAT(IPV6_MULTICAST_IF),
#endif
#ifdef IPV6_MTU_DISCOVER
	XLAT(IPV6_MTU_DISCOVER),
#endif
#ifdef IPV6_RECVERR
	XLAT(IPV6_RECVERR),
#endif
#ifdef IPV6_FLOWINFO_SEND
	XLAT(IPV6_FLOWINFO_SEND),
#endif
#ifdef IPV6_ADD_MEMBERSHIP
	XLAT(IPV6_ADD_MEMBERSHIP),
#endif
#ifdef IPV6_DROP_MEMBERSHIP
	XLAT(IPV6_DROP_MEMBERSHIP),
#endif
#ifdef IPV6_ROUTER_ALERT
	XLAT(IPV6_ROUTER_ALERT),
#endif
	XLAT_END
};
#endif /* SOL_IPV6 */

#ifdef SOL_IPX
static const struct xlat sockipxoptions[] = {
	XLAT(IPX_TYPE),
	XLAT_END
};
#endif /* SOL_IPX */

#ifdef SOL_RAW
static const struct xlat sockrawoptions[] = {
#if defined(ICMP_FILTER)
	XLAT(ICMP_FILTER),
#endif
	XLAT_END
};
#endif /* SOL_RAW */

#ifdef SOL_PACKET
static const struct xlat sockpacketoptions[] = {
#ifdef PACKET_ADD_MEMBERSHIP
	XLAT(PACKET_ADD_MEMBERSHIP),
#endif
#ifdef PACKET_DROP_MEMBERSHIP
	XLAT(PACKET_DROP_MEMBERSHIP),
#endif
#if defined(PACKET_RECV_OUTPUT)
	XLAT(PACKET_RECV_OUTPUT),
#endif
#if defined(PACKET_RX_RING)
	XLAT(PACKET_RX_RING),
#endif
#if defined(PACKET_STATISTICS)
	XLAT(PACKET_STATISTICS),
#endif
#if defined(PACKET_COPY_THRESH)
	XLAT(PACKET_COPY_THRESH),
#endif
#if defined(PACKET_AUXDATA)
	XLAT(PACKET_AUXDATA),
#endif
#if defined(PACKET_ORIGDEV)
	XLAT(PACKET_ORIGDEV),
#endif
#if defined(PACKET_VERSION)
	XLAT(PACKET_VERSION),
#endif
#if defined(PACKET_HDRLEN)
	XLAT(PACKET_HDRLEN),
#endif
#if defined(PACKET_RESERVE)
	XLAT(PACKET_RESERVE),
#endif
#if defined(PACKET_TX_RING)
	XLAT(PACKET_TX_RING),
#endif
#if defined(PACKET_LOSS)
	XLAT(PACKET_LOSS),
#endif
	XLAT_END
};
#endif /* SOL_PACKET */

#ifdef SOL_SCTP
static const struct xlat socksctpoptions[] = {
#if defined(SCTP_RTOINFO)
	XLAT(SCTP_RTOINFO),
#endif
#if defined(SCTP_ASSOCINFO)
	XLAT(SCTP_ASSOCINFO),
#endif
#if defined(SCTP_INITMSG)
	XLAT(SCTP_INITMSG),
#endif
#if defined(SCTP_NODELAY)
	XLAT(SCTP_NODELAY),
#endif
#if defined(SCTP_AUTOCLOSE)
	XLAT(SCTP_AUTOCLOSE),
#endif
#if defined(SCTP_SET_PEER_PRIMARY_ADDR)
	XLAT(SCTP_SET_PEER_PRIMARY_ADDR),
#endif
#if defined(SCTP_PRIMARY_ADDR)
	XLAT(SCTP_PRIMARY_ADDR),
#endif
#if defined(SCTP_ADAPTATION_LAYER)
	XLAT(SCTP_ADAPTATION_LAYER),
#endif
#if defined(SCTP_DISABLE_FRAGMENTS)
	XLAT(SCTP_DISABLE_FRAGMENTS),
#endif
#if defined(SCTP_PEER_ADDR_PARAMS)
	XLAT(SCTP_PEER_ADDR_PARAMS),
#endif
#if defined(SCTP_DEFAULT_SEND_PARAM)
	XLAT(SCTP_DEFAULT_SEND_PARAM),
#endif
#if defined(SCTP_EVENTS)
	XLAT(SCTP_EVENTS),
#endif
#if defined(SCTP_I_WANT_MAPPED_V4_ADDR)
	XLAT(SCTP_I_WANT_MAPPED_V4_ADDR),
#endif
#if defined(SCTP_MAXSEG)
	XLAT(SCTP_MAXSEG),
#endif
#if defined(SCTP_STATUS)
	XLAT(SCTP_STATUS),
#endif
#if defined(SCTP_GET_PEER_ADDR_INFO)
	XLAT(SCTP_GET_PEER_ADDR_INFO),
#endif
#if defined(SCTP_DELAYED_ACK)
	XLAT(SCTP_DELAYED_ACK),
#endif
#if defined(SCTP_CONTEXT)
	XLAT(SCTP_CONTEXT),
#endif
#if defined(SCTP_FRAGMENT_INTERLEAVE)
	XLAT(SCTP_FRAGMENT_INTERLEAVE),
#endif
#if defined(SCTP_PARTIAL_DELIVERY_POINT)
	XLAT(SCTP_PARTIAL_DELIVERY_POINT),
#endif
#if defined(SCTP_MAX_BURST)
	XLAT(SCTP_MAX_BURST),
#endif
#if defined(SCTP_AUTH_CHUNK)
	XLAT(SCTP_AUTH_CHUNK),
#endif
#if defined(SCTP_HMAC_IDENT)
	XLAT(SCTP_HMAC_IDENT),
#endif
#if defined(SCTP_AUTH_KEY)
	XLAT(SCTP_AUTH_KEY),
#endif
#if defined(SCTP_AUTH_ACTIVE_KEY)
	XLAT(SCTP_AUTH_ACTIVE_KEY),
#endif
#if defined(SCTP_AUTH_DELETE_KEY)
	XLAT(SCTP_AUTH_DELETE_KEY),
#endif
#if defined(SCTP_PEER_AUTH_CHUNKS)
	XLAT(SCTP_PEER_AUTH_CHUNKS),
#endif
#if defined(SCTP_LOCAL_AUTH_CHUNKS)
	XLAT(SCTP_LOCAL_AUTH_CHUNKS),
#endif
#if defined(SCTP_GET_ASSOC_NUMBER)
	XLAT(SCTP_GET_ASSOC_NUMBER),
#endif

	/* linux specific things */
#if defined(SCTP_SOCKOPT_BINDX_ADD)
	XLAT(SCTP_SOCKOPT_BINDX_ADD),
#endif
#if defined(SCTP_SOCKOPT_BINDX_REM)
	XLAT(SCTP_SOCKOPT_BINDX_REM),
#endif
#if defined(SCTP_SOCKOPT_PEELOFF)
	XLAT(SCTP_SOCKOPT_PEELOFF),
#endif
#if defined(SCTP_GET_PEER_ADDRS_NUM_OLD)
	XLAT(SCTP_GET_PEER_ADDRS_NUM_OLD),
#endif
#if defined(SCTP_GET_PEER_ADDRS_OLD)
	XLAT(SCTP_GET_PEER_ADDRS_OLD),
#endif
#if defined(SCTP_GET_LOCAL_ADDRS_NUM_OLD)
	XLAT(SCTP_GET_LOCAL_ADDRS_NUM_OLD),
#endif
#if defined(SCTP_GET_LOCAL_ADDRS_OLD)
	XLAT(SCTP_GET_LOCAL_ADDRS_OLD),
#endif
#if defined(SCTP_SOCKOPT_CONNECTX_OLD)
	XLAT(SCTP_SOCKOPT_CONNECTX_OLD),
#endif
#if defined(SCTP_GET_PEER_ADDRS)
	XLAT(SCTP_GET_PEER_ADDRS),
#endif
#if defined(SCTP_GET_LOCAL_ADDRS)
	XLAT(SCTP_GET_LOCAL_ADDRS),
#endif

	XLAT_END
};
#endif

#if !defined(SOL_TCP) && defined(IPPROTO_TCP)
#define SOL_TCP IPPROTO_TCP
#endif

#ifdef SOL_TCP
static const struct xlat socktcpoptions[] = {
	XLAT(TCP_NODELAY),
	XLAT(TCP_MAXSEG),
#ifdef TCP_CORK
	XLAT(TCP_CORK),
#endif
#ifdef TCP_KEEPIDLE
	XLAT(TCP_KEEPIDLE),
#endif
#ifdef TCP_KEEPINTVL
	XLAT(TCP_KEEPINTVL),
#endif
#ifdef TCP_KEEPCNT
	XLAT(TCP_KEEPCNT),
#endif
#ifdef TCP_SYNCNT
	XLAT(TCP_SYNCNT),
#endif
#ifdef TCP_LINGER2
	XLAT(TCP_LINGER2),
#endif
#ifdef TCP_DEFER_ACCEPT
	XLAT(TCP_DEFER_ACCEPT),
#endif
#ifdef TCP_WINDOW_CLAMP
	XLAT(TCP_WINDOW_CLAMP),
#endif
#ifdef TCP_INFO
	XLAT(TCP_INFO),
#endif
#ifdef TCP_QUICKACK
	XLAT(TCP_QUICKACK),
#endif
#ifdef TCP_CONGESTION
	XLAT(TCP_CONGESTION),
#endif
#ifdef TCP_MD5SIG
	XLAT(TCP_MD5SIG),
#endif
#ifdef TCP_COOKIE_TRANSACTIONS
	XLAT(TCP_COOKIE_TRANSACTIONS),
#endif
#ifdef TCP_THIN_LINEAR_TIMEOUTS
	XLAT(TCP_THIN_LINEAR_TIMEOUTS),
#endif
#ifdef TCP_THIN_DUPACK
	XLAT(TCP_THIN_DUPACK),
#endif
#ifdef TCP_USER_TIMEOUT
	XLAT(TCP_USER_TIMEOUT),
#endif
#ifdef TCP_REPAIR
	XLAT(TCP_REPAIR),
#endif
#ifdef TCP_REPAIR_QUEUE
	XLAT(TCP_REPAIR_QUEUE),
#endif
#ifdef TCP_QUEUE_SEQ
	XLAT(TCP_QUEUE_SEQ),
#endif
#ifdef TCP_REPAIR_OPTIONS
	XLAT(TCP_REPAIR_OPTIONS),
#endif
#ifdef TCP_FASTOPEN
	XLAT(TCP_FASTOPEN),
#endif
#ifdef TCP_TIMESTAMP
	XLAT(TCP_TIMESTAMP),
#endif
	XLAT_END
};
#endif /* SOL_TCP */

#ifdef SOL_RAW
static const struct xlat icmpfilterflags[] = {
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
	XLAT_END
};
#endif /* SOL_RAW */

#if defined(AF_PACKET) /* from e.g. linux/if_packet.h */
static const struct xlat af_packet_types[] = {
#if defined(PACKET_HOST)
	XLAT(PACKET_HOST),
#endif
#if defined(PACKET_BROADCAST)
	XLAT(PACKET_BROADCAST),
#endif
#if defined(PACKET_MULTICAST)
	XLAT(PACKET_MULTICAST),
#endif
#if defined(PACKET_OTHERHOST)
	XLAT(PACKET_OTHERHOST),
#endif
#if defined(PACKET_OUTGOING)
	XLAT(PACKET_OUTGOING),
#endif
#if defined(PACKET_LOOPBACK)
	XLAT(PACKET_LOOPBACK),
#endif
#if defined(PACKET_FASTROUTE)
	XLAT(PACKET_FASTROUTE),
#endif
	XLAT_END
};
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

	if (addrlen < 2 || addrlen > sizeof(addrbuf))
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
			printxval(af_packet_types, addrbuf.ll.sll_pkttype, "?");
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
static const struct xlat scmvals[] = {
#ifdef SCM_RIGHTS
	XLAT(SCM_RIGHTS),
#endif
#ifdef SCM_CREDENTIALS
	XLAT(SCM_CREDENTIALS),
#endif
	XLAT_END
};

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
				tprintf("%d", *fds++);
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

static void
printmsghdr(struct tcb *tcp, long addr, unsigned long data_size)
{
	struct msghdr msg;

#if SUPPORTED_PERSONALITIES > 1 && SIZEOF_LONG > 4
	if (current_wordsize == 4) {
		struct msghdr32 msg32;

		if (umove(tcp, addr, &msg32) < 0) {
			tprintf("%#lx", addr);
			return;
		}
		msg.msg_name       = (void*)(long)msg32.msg_name;
		msg.msg_namelen    =              msg32.msg_namelen;
		msg.msg_iov        = (void*)(long)msg32.msg_iov;
		msg.msg_iovlen     =              msg32.msg_iovlen;
		msg.msg_control    = (void*)(long)msg32.msg_control;
		msg.msg_controllen =              msg32.msg_controllen;
		msg.msg_flags      =              msg32.msg_flags;
	} else
#endif
	if (umove(tcp, addr, &msg) < 0) {
		tprintf("%#lx", addr);
		return;
	}
	do_msghdr(tcp, &msg, data_size);
}

static void
printmmsghdr(struct tcb *tcp, long addr, unsigned int idx, unsigned long msg_len)
{
	struct mmsghdr {
		struct msghdr msg_hdr;
		unsigned msg_len;
	} mmsg;

#if SUPPORTED_PERSONALITIES > 1 && SIZEOF_LONG > 4
	if (current_wordsize == 4) {
		struct mmsghdr32 mmsg32;

		addr += sizeof(mmsg32) * idx;
		if (umove(tcp, addr, &mmsg32) < 0) {
			tprintf("%#lx", addr);
			return;
		}
		mmsg.msg_hdr.msg_name       = (void*)(long)mmsg32.msg_hdr.msg_name;
		mmsg.msg_hdr.msg_namelen    =              mmsg32.msg_hdr.msg_namelen;
		mmsg.msg_hdr.msg_iov        = (void*)(long)mmsg32.msg_hdr.msg_iov;
		mmsg.msg_hdr.msg_iovlen     =              mmsg32.msg_hdr.msg_iovlen;
		mmsg.msg_hdr.msg_control    = (void*)(long)mmsg32.msg_hdr.msg_control;
		mmsg.msg_hdr.msg_controllen =              mmsg32.msg_hdr.msg_controllen;
		mmsg.msg_hdr.msg_flags      =              mmsg32.msg_hdr.msg_flags;
		mmsg.msg_len                =              mmsg32.msg_len;
	} else
#endif
	{
		addr += sizeof(mmsg) * idx;
		if (umove(tcp, addr, &mmsg) < 0) {
			tprintf("%#lx", addr);
			return;
		}
	}
	tprints("{");
	do_msghdr(tcp, &mmsg.msg_hdr, msg_len ? msg_len : mmsg.msg_len);
	tprintf(", %u}", mmsg.msg_len);
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

#endif /* HAVE_SENDMSG */

/*
 * low bits of the socket type define real socket type,
 * other bits are socket type flags.
 */
static void
tprint_sock_type(struct tcb *tcp, int flags)
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
		tprint_sock_type(tcp, tcp->u_arg[1]);
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
do_accept(struct tcb *tcp, int flags_arg)
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
	return do_accept(tcp, -1);
}

int
sys_accept4(struct tcb *tcp)
{
	return do_accept(tcp, 3);
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

static const struct xlat shutdown_modes[] = {
	{ 0,	"SHUT_RD"	},
	{ 1,	"SHUT_WR"	},
	{ 2,	"SHUT_RDWR"	},
	XLAT_END
};

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
	return sys_accept(tcp);
}

int
sys_getpeername(struct tcb *tcp)
{
	return sys_accept(tcp);
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
		tprint_sock_type(tcp, tcp->u_arg[1]);
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
