/*
 * Copyright (c) 2016 Fabien Siron <fabien.siron@epita.fr>
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017 The strace developers.
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
#include "netlink.h"
#include "nlattr.h"

#include <arpa/inet.h>
#include <linux/inet_diag.h>
#include <linux/netlink_diag.h>
#include <linux/packet_diag.h>
#ifdef AF_SMC
# include <linux/smc_diag.h>
#endif
#include <linux/unix_diag.h>

#include "xlat/inet_diag_attrs.h"
#include "xlat/inet_diag_extended_flags.h"
#include "xlat/inet_diag_req_attrs.h"

#include "xlat/tcp_states.h"
#include "xlat/tcp_state_flags.h"

#include "xlat/netlink_diag_attrs.h"
#include "xlat/netlink_diag_show.h"
#include "xlat/netlink_states.h"

#include "xlat/packet_diag_attrs.h"
#include "xlat/packet_diag_show.h"

#ifdef AF_SMC
# include "xlat/smc_diag_attrs.h"
# include "xlat/smc_diag_extended_flags.h"
# include "xlat/smc_states.h"
#endif

#include "xlat/unix_diag_attrs.h"
#include "xlat/unix_diag_show.h"

#define PRINT_FIELD_U(prefix_, where_, field_)				\
	tprintf("%s%s=%llu", (prefix_), #field_,			\
		zero_extend_signed_to_ull((where_).field_))

#define PRINT_FIELD_COOKIE(prefix_, where_, field_)			\
	tprintf("%s%s=[%llu, %llu]", (prefix_), #field_,		\
		zero_extend_signed_to_ull((where_).field_[0]),		\
		zero_extend_signed_to_ull((where_).field_[1]))

#define PRINT_FIELD_FLAGS(prefix_, where_, field_, xlat_, dflt_)	\
	do {								\
		tprintf("%s%s=", (prefix_), #field_);			\
		printflags((xlat_), (where_).field_, (dflt_));		\
	} while (0)

#define PRINT_FIELD_XVAL(prefix_, where_, field_, xlat_, dflt_)		\
	do {								\
		tprintf("%s%s=", (prefix_), #field_);			\
		printxval((xlat_), (where_).field_, (dflt_));		\
	} while (0)

static void
decode_family(struct tcb *const tcp, const uint8_t family,
	      const kernel_ulong_t addr, const kernel_ulong_t len)
{
	tprints("{family=");
	printxval(addrfams, family, "AF_???");
	if (len > sizeof(family)) {
		tprints(", ");
		printstrn(tcp, addr + sizeof(family),
			  len - sizeof(family));
	}
	tprints("}");
}

static void
decode_unix_diag_req(struct tcb *const tcp,
		     const struct nlmsghdr *const nlmsghdr,
		     const uint8_t family,
		     const kernel_ulong_t addr,
		     const kernel_ulong_t len)
{
	struct unix_diag_req req = { .sdiag_family = family };
	const size_t offset = sizeof(req.sdiag_family);

	PRINT_FIELD_XVAL("{", req, sdiag_family, addrfams, "AF_???");
	tprints(", ");
	if (len >= sizeof(req)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(req) - offset,
					 (void *) &req + offset)) {
			PRINT_FIELD_U("", req, sdiag_protocol);
			PRINT_FIELD_FLAGS(", ", req, udiag_states,
					  tcp_state_flags, "1<<TCP_???");
			PRINT_FIELD_U(", ", req, udiag_ino);
			PRINT_FIELD_FLAGS(", ", req, udiag_show,
					  unix_diag_show, "UDIAG_SHOW_???");
			PRINT_FIELD_COOKIE(", ", req, udiag_cookie);
		}
	} else
		tprints("...");
	tprints("}");
}

static void
decode_unix_diag_msg(struct tcb *const tcp,
		     const struct nlmsghdr *const nlmsghdr,
		     const uint8_t family,
		     const kernel_ulong_t addr,
		     const kernel_ulong_t len)
{
	struct unix_diag_msg msg = { .udiag_family = family };
	size_t offset = sizeof(msg.udiag_family);
	bool decode_nla = false;

	PRINT_FIELD_XVAL("{", msg, udiag_family, addrfams, "AF_???");
	tprints(", ");
	if (len >= sizeof(msg)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(msg) - offset,
					 (void *) &msg + offset)) {
			PRINT_FIELD_XVAL("", msg, udiag_type,
					 socktypes, "SOCK_???");
			PRINT_FIELD_XVAL(", ", msg, udiag_state,
					 tcp_states, "TCP_???");
			PRINT_FIELD_U(", ", msg, udiag_ino);
			PRINT_FIELD_COOKIE(", ", msg, udiag_cookie);
			decode_nla = true;
		}
	} else
		tprints("...");
	tprints("}");

	offset = NLMSG_ALIGN(sizeof(msg));
	if (decode_nla && len > offset) {
		tprints(", ");
		decode_nlattr(tcp, addr + offset, len - offset,
			      unix_diag_attrs, "UNIX_DIAG_???");
	}
}

static void
decode_netlink_diag_req(struct tcb *const tcp,
			const struct nlmsghdr *const nlmsghdr,
			const uint8_t family,
			const kernel_ulong_t addr,
			const kernel_ulong_t len)
{
	struct netlink_diag_req req = { .sdiag_family = family };
	const size_t offset = sizeof(req.sdiag_family);

	PRINT_FIELD_XVAL("{", req, sdiag_family, addrfams, "AF_???");
	tprints(", ");
	if (len >= sizeof(req)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(req) - offset,
					 (void *) &req + offset)) {
			if (NDIAG_PROTO_ALL == req.sdiag_protocol)
				tprintf("%s=%s",
					"sdiag_protocol", "NDIAG_PROTO_ALL");
			else
				PRINT_FIELD_XVAL("", req, sdiag_protocol,
						 netlink_protocols,
						 "NETLINK_???");
			PRINT_FIELD_U(", ", req, ndiag_ino);
			PRINT_FIELD_FLAGS(", ", req, ndiag_show,
					  netlink_diag_show, "NDIAG_SHOW_???");
			PRINT_FIELD_COOKIE(", ", req, ndiag_cookie);
		}
	} else
		tprints("...");
	tprints("}");
}

static void
decode_netlink_diag_msg(struct tcb *const tcp,
			const struct nlmsghdr *const nlmsghdr,
			const uint8_t family,
			const kernel_ulong_t addr,
			const kernel_ulong_t len)
{
	struct netlink_diag_msg msg = { .ndiag_family = family };
	size_t offset = sizeof(msg.ndiag_family);
	bool decode_nla = false;

	PRINT_FIELD_XVAL("{", msg, ndiag_family, addrfams, "AF_???");
	tprints(", ");
	if (len >= sizeof(msg)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(msg) - offset,
					 (void *) &msg + offset)) {
			PRINT_FIELD_XVAL("", msg, ndiag_type,
					 socktypes, "SOCK_???");
			PRINT_FIELD_XVAL(", ", msg, ndiag_protocol,
					 netlink_protocols, "NETLINK_???");
			PRINT_FIELD_XVAL(", ", msg, ndiag_state,
					 netlink_states, "NETLINK_???");
			PRINT_FIELD_U(", ", msg, ndiag_portid);
			PRINT_FIELD_U(", ", msg, ndiag_dst_portid);
			PRINT_FIELD_U(", ", msg, ndiag_dst_group);
			PRINT_FIELD_U(", ", msg, ndiag_ino);
			PRINT_FIELD_COOKIE(", ", msg, ndiag_cookie);
			decode_nla = true;
		}
	} else
		tprints("...");
	tprints("}");

	offset = NLA_ALIGN(sizeof(msg));
	if (decode_nla && len > offset) {
		tprints(", ");
		decode_nlattr(tcp, addr + offset, len - offset,
			      netlink_diag_attrs, "NETLINK_DIAG_???");
	}
}

static void
decode_packet_diag_req(struct tcb *const tcp,
		       const struct nlmsghdr *const nlmsghdr,
		       const uint8_t family,
		       const kernel_ulong_t addr,
		       const kernel_ulong_t len)
{
	struct packet_diag_req req = { .sdiag_family = family };
	const size_t offset = sizeof(req.sdiag_family);

	PRINT_FIELD_XVAL("{", req, sdiag_family, addrfams, "AF_???");
	tprints(", ");
	if (len >= sizeof(req)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(req) - offset,
					 (void *) &req + offset)) {
			PRINT_FIELD_XVAL("", req, sdiag_protocol,
					 ethernet_protocols, "ETH_P_???");
			PRINT_FIELD_U(", ", req, pdiag_ino);
			PRINT_FIELD_FLAGS(", ", req, pdiag_show,
					  packet_diag_show, "PACKET_SHOW_???");
			PRINT_FIELD_COOKIE(", ", req, pdiag_cookie);
		}
	} else
		tprints("...");
	tprints("}");
}

static void
decode_packet_diag_msg(struct tcb *const tcp,
		       const struct nlmsghdr *const nlmsghdr,
		       const uint8_t family,
		       const kernel_ulong_t addr,
		       const kernel_ulong_t len)
{
	struct packet_diag_msg msg = { .pdiag_family = family };
	size_t offset = sizeof(msg.pdiag_family);
	bool decode_nla = false;

	PRINT_FIELD_XVAL("{", msg, pdiag_family, addrfams, "AF_???");
	tprints(", ");
	if (len >= sizeof(msg)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(msg) - offset,
					 (void *) &msg + offset)) {
			PRINT_FIELD_XVAL("", msg, pdiag_type,
					 socktypes, "SOCK_???");
			PRINT_FIELD_U(", ", msg, pdiag_num);
			PRINT_FIELD_U(", ", msg, pdiag_ino);
			PRINT_FIELD_COOKIE(", ", msg, pdiag_cookie);
			decode_nla = true;
		}
	} else
		tprints("...");
	tprints("}");

	offset = NLA_ALIGN(sizeof(msg));
	if (decode_nla && len > offset) {
		tprints(", ");
		decode_nlattr(tcp, addr + offset, len - offset,
			      packet_diag_attrs, "PACKET_DIAG_???");
	}
}

static void
print_inet_diag_sockid(const struct inet_diag_sockid *id, const uint8_t family)
{
	tprintf("{idiag_sport=htons(%u), idiag_dport=htons(%u)",
		ntohs(id->idiag_sport), ntohs(id->idiag_dport));

	tprints(", ");
	print_inet_addr(family, id->idiag_src,
			sizeof(id->idiag_src), "idiag_src");
	tprints(", ");
	print_inet_addr(family, id->idiag_dst,
			sizeof(id->idiag_dst), "idiag_dst");

	PRINT_FIELD_U(", ", *id, idiag_if);
	PRINT_FIELD_COOKIE(", ", *id, idiag_cookie);

	tprints("}");
}

static void
decode_inet_diag_req_compat(struct tcb *const tcp,
			    const struct nlmsghdr *const nlmsghdr,
			    const uint8_t family,
			    const kernel_ulong_t addr,
			    const kernel_ulong_t len)
{
	struct inet_diag_req req = { .idiag_family = family };
	size_t offset = sizeof(req.idiag_family);
	bool decode_nla = false;

	PRINT_FIELD_XVAL("{", req, idiag_family, addrfams, "AF_???");
	tprints(", ");
	if (len >= sizeof(req)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(req) - offset,
					 (void *) &req + offset)) {
			PRINT_FIELD_U("", req, idiag_src_len);
			PRINT_FIELD_U(", ", req, idiag_dst_len);
			PRINT_FIELD_FLAGS(", ", req, idiag_ext,
					  inet_diag_extended_flags,
					  "1<<INET_DIAG_\?\?\?-1");
			tprints(", id=");
			print_inet_diag_sockid(&req.id, req.idiag_family);
			PRINT_FIELD_FLAGS(", ", req, idiag_states,
					  tcp_state_flags, "1<<TCP_???");
			PRINT_FIELD_U(", ", req, idiag_dbs);
			decode_nla = true;
		}
	} else
		tprints("...");
	tprints("}");

	offset = NLA_ALIGN(sizeof(req));
	if (decode_nla && len > offset) {
		tprints(", ");
		decode_nlattr(tcp, addr + offset, len - offset,
			      inet_diag_req_attrs, "INET_DIAG_REQ_???");
	}
}

static void
decode_inet_diag_req_v2(struct tcb *const tcp,
			const struct nlmsghdr *const nlmsghdr,
			const uint8_t family,
			const kernel_ulong_t addr,
			const kernel_ulong_t len)
{
	struct inet_diag_req_v2 req = { .sdiag_family = family };
	size_t offset = sizeof(req.sdiag_family);
	bool decode_nla = false;

	PRINT_FIELD_XVAL("{", req, sdiag_family, addrfams, "AF_???");
	tprints(", ");
	if (len >= sizeof(req)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(req) - offset,
					 (void *) &req + offset)) {
			PRINT_FIELD_XVAL("", req, sdiag_protocol,
					 inet_protocols, "IPPROTO_???");
			PRINT_FIELD_FLAGS(", ", req, idiag_ext,
					  inet_diag_extended_flags,
					  "1<<INET_DIAG_\?\?\?-1");
			PRINT_FIELD_FLAGS(", ", req, idiag_states,
					  tcp_state_flags, "1<<TCP_???");
			tprints(", id=");
			print_inet_diag_sockid(&req.id, req.sdiag_family);
			decode_nla = true;
		}
	} else
		tprints("...");
	tprints("}");

	offset = NLA_ALIGN(sizeof(req));
	if (decode_nla && len > offset) {
		tprints(", ");
		decode_nlattr(tcp, addr + offset, len - offset,
			      inet_diag_req_attrs, "INET_DIAG_REQ_???");
	}
}

static void
decode_inet_diag_req(struct tcb *const tcp,
		     const struct nlmsghdr *const nlmsghdr,
		     const uint8_t family,
		     const kernel_ulong_t addr,
		     const kernel_ulong_t len)
{
	if (nlmsghdr->nlmsg_type == TCPDIAG_GETSOCK
	    || nlmsghdr->nlmsg_type == DCCPDIAG_GETSOCK)
		return decode_inet_diag_req_compat(tcp, nlmsghdr,
						   family, addr, len);
	else
		return decode_inet_diag_req_v2(tcp, nlmsghdr,
					       family, addr, len);
}

static void
decode_inet_diag_msg(struct tcb *const tcp,
		     const struct nlmsghdr *const nlmsghdr,
		     const uint8_t family,
		     const kernel_ulong_t addr,
		     const kernel_ulong_t len)
{
	struct inet_diag_msg msg = { .idiag_family = family };
	size_t offset = sizeof(msg.idiag_family);
	bool decode_nla = false;

	PRINT_FIELD_XVAL("{", msg, idiag_family, addrfams, "AF_???");
	tprints(", ");
	if (len >= sizeof(msg)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(msg) - offset,
					 (void *) &msg + offset)) {
			PRINT_FIELD_XVAL("", msg, idiag_state,
					 tcp_states, "TCP_???");
			PRINT_FIELD_U(", ", msg, idiag_timer);
			PRINT_FIELD_U(", ", msg, idiag_retrans);
			tprints(", id=");
			print_inet_diag_sockid(&msg.id, msg.idiag_family);
			PRINT_FIELD_U(", ", msg, idiag_expires);
			PRINT_FIELD_U(", ", msg, idiag_rqueue);
			PRINT_FIELD_U(", ", msg, idiag_wqueue);
			PRINT_FIELD_U(", ", msg, idiag_uid);
			PRINT_FIELD_U(", ", msg, idiag_inode);
			decode_nla = true;
		}
	} else
		tprints("...");
	tprints("}");

	offset = NLA_ALIGN(sizeof(msg));
	if (decode_nla && len > offset) {
		tprints(", ");
		decode_nlattr(tcp, addr + offset, len - offset,
			      inet_diag_attrs, "INET_DIAG_???");
	}
}

#ifdef AF_SMC
static void
decode_smc_diag_req(struct tcb *const tcp,
		    const struct nlmsghdr *const nlmsghdr,
		    const uint8_t family,
		    const kernel_ulong_t addr,
		    const kernel_ulong_t len)
{
	struct smc_diag_req req = { .diag_family = family };
	const size_t offset = sizeof(req.diag_family);

	PRINT_FIELD_XVAL("{", req, diag_family, addrfams, "AF_???");
	tprints(", ");
	if (len >= sizeof(req)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(req) - offset,
					 (void *) &req + offset)) {
			PRINT_FIELD_FLAGS("", req, diag_ext,
					  smc_diag_extended_flags,
					  "1<<SMC_DIAG_\?\?\?-1");
			tprints(", id=");
			/*
			 * AF_SMC protocol family socket handler
			 * keeping the AF_INET sock address.
			 */
			print_inet_diag_sockid(&req.id, AF_INET);
		}
	} else
		tprints("...");
	tprints("}");
}

static void
decode_smc_diag_msg(struct tcb *const tcp,
		    const struct nlmsghdr *const nlmsghdr,
		    const uint8_t family,
		    const kernel_ulong_t addr,
		    const kernel_ulong_t len)
{
	struct smc_diag_msg msg = { .diag_family = family };
	size_t offset = sizeof(msg.diag_family);
	bool decode_nla = false;

	PRINT_FIELD_XVAL("{", msg, diag_family, addrfams, "AF_???");
	tprints(", ");
	if (len >= sizeof(msg)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(msg) - offset,
					 (void *) &msg + offset)) {
			PRINT_FIELD_XVAL("", msg, diag_state,
					 smc_states, "SMC_???");
			PRINT_FIELD_U(", ", msg, diag_fallback);
			PRINT_FIELD_U(", ", msg, diag_shutdown);
			tprints(", id=");
			/*
			 * AF_SMC protocol family socket handler
			 * keeping the AF_INET sock address.
			 */
			print_inet_diag_sockid(&msg.id, AF_INET);
			PRINT_FIELD_U(", ", msg, diag_uid);
			PRINT_FIELD_U(", ", msg, diag_inode);
			decode_nla = true;
		}
	} else
		tprints("...");
	tprints("}");

	offset = NLA_ALIGN(sizeof(msg));
	if (decode_nla && len > offset) {
		tprints(", ");
		decode_nlattr(tcp, addr + offset, len - offset,
			      smc_diag_attrs, "SMC_DIAG_???");
	}
}
#endif

typedef void (*netlink_diag_decoder_t)(struct tcb *,
				       const struct nlmsghdr *,
				       uint8_t family,
				       kernel_ulong_t addr,
				       kernel_ulong_t len);

static const struct {
	const netlink_diag_decoder_t request, response;
} diag_decoders[] = {
	[AF_INET] = { decode_inet_diag_req, decode_inet_diag_msg },
	[AF_INET6] = { decode_inet_diag_req, decode_inet_diag_msg },
	[AF_NETLINK] = { decode_netlink_diag_req, decode_netlink_diag_msg },
	[AF_PACKET] = { decode_packet_diag_req, decode_packet_diag_msg },
#ifdef AF_SMC
	[AF_SMC] = { decode_smc_diag_req, decode_smc_diag_msg },
#endif
	[AF_UNIX] = { decode_unix_diag_req, decode_unix_diag_msg }
};

bool
decode_netlink_sock_diag(struct tcb *const tcp,
			 const struct nlmsghdr *const nlmsghdr,
			 const kernel_ulong_t addr,
			 const kernel_ulong_t len)
{
	uint8_t family;

	if (nlmsghdr->nlmsg_type == NLMSG_DONE)
		return false;

	if (!umove_or_printaddr(tcp, addr, &family)) {
		if (family < ARRAY_SIZE(diag_decoders)
		    && len > sizeof(family)) {
			const netlink_diag_decoder_t decoder =
				(nlmsghdr->nlmsg_flags & NLM_F_REQUEST)
				? diag_decoders[family].request
				: diag_decoders[family].response;

			if (decoder) {
				decoder(tcp, nlmsghdr, family, addr, len);
				return true;
			}
		}

		decode_family(tcp, family, addr, len);
	}

	return true;
}
