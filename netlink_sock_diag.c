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

#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/netlink_diag.h>
#include <linux/packet_diag.h>
#include <linux/unix_diag.h>

#include "xlat/tcp_states.h"
#include "xlat/tcp_state_flags.h"

#include "xlat/netlink_diag_show.h"
#include "xlat/netlink_states.h"

#include "xlat/packet_diag_show.h"

#include "xlat/unix_diag_show.h"

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

	tprints("{sdiag_family=");
	printxval(addrfams, req.sdiag_family, "AF_???");

	tprints(", ");
	if (len >= sizeof(req)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(req) - offset,
					 (void *) &req + offset)) {
			tprintf("sdiag_protocol=%" PRIu8 ", udiag_states=",
				req.sdiag_protocol);
			printflags(tcp_state_flags, req.udiag_states,
				   "1<<TCP_???");
			tprintf(", udiag_ino=%" PRIu32 ", udiag_show=",
				req.udiag_ino);
			printflags(unix_diag_show, req.udiag_show,
				   "UDIAG_SHOW_???");
			tprintf(", udiag_cookie=[%" PRIu32 ", %" PRIu32 "]",
				req.udiag_cookie[0], req.udiag_cookie[1]);
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
	const size_t offset = sizeof(msg.udiag_family);

	tprints("{udiag_family=");
	printxval(addrfams, msg.udiag_family, "AF_???");

	tprints(", ");
	if (len >= sizeof(msg)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(msg) - offset,
					 (void *) &msg + offset)) {
			tprints("udiag_type=");
			printxval(socktypes, msg.udiag_type, "SOCK_???");
			tprintf(", udiag_state=");
			printxval(tcp_states, msg.udiag_state, "TCP_???");
			tprintf(", udiag_ino=%" PRIu32
				", udiag_cookie=[%" PRIu32 ", %" PRIu32 "]",
				msg.udiag_ino,
				msg.udiag_cookie[0], msg.udiag_cookie[1]);
		}
	} else
		tprints("...");
	tprints("}");
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

	tprints("{sdiag_family=");
	printxval(addrfams, req.sdiag_family, "AF_???");

	tprints(", ");
	if (len >= sizeof(req)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(req) - offset,
					 (void *) &req + offset)) {
			tprints("sdiag_protocol=");
			if (NDIAG_PROTO_ALL == req.sdiag_protocol)
				tprints("NDIAG_PROTO_ALL");
			else
				printxval(netlink_protocols,
					  req.sdiag_protocol, "NETLINK_???");
			tprintf(", ndiag_ino=%" PRIu32 ", ndiag_show=",
				req.ndiag_ino);
			printflags(netlink_diag_show, req.ndiag_show,
				   "NDIAG_SHOW_???");
			tprintf(", ndiag_cookie=[%" PRIu32 ", %" PRIu32 "]",
				req.ndiag_cookie[0], req.ndiag_cookie[1]);
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
	const size_t offset = sizeof(msg.ndiag_family);

	tprints("{ndiag_family=");
	printxval(addrfams, msg.ndiag_family, "AF_???");

	tprints(", ");
	if (len >= sizeof(msg)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(msg) - offset,
					 (void *) &msg + offset)) {
			tprints("ndiag_type=");
			printxval(socktypes, msg.ndiag_type, "SOCK_???");
			tprints(", ndiag_protocol=");
			printxval(netlink_protocols, msg.ndiag_protocol,
				  "NETLINK_???");
			tprints(", ndiag_state=");
			printxval(netlink_states, msg.ndiag_state,
				  "NETLINK_???");
			tprintf(", ndiag_portid=%" PRIu32
				", ndiag_dst_portid=%" PRIu32
				", ndiag_dst_group=%" PRIu32
				", ndiag_ino=%" PRIu32
				", ndiag_cookie=[%" PRIu32
				", %" PRIu32 "]",
				msg.ndiag_portid,
				msg.ndiag_dst_portid,
				msg.ndiag_dst_group,
				msg.ndiag_ino,
				msg.ndiag_cookie[0],
				msg.ndiag_cookie[1]);
		}
	} else
		tprints("...");
	tprints("}");
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

	tprints("{sdiag_family=");
	printxval(addrfams, req.sdiag_family, "AF_???");
	tprints(", ");
	if (len >= sizeof(req)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(req) - offset,
					 (void *) &req + offset)) {
			tprints("sdiag_protocol=");
			printxval(ethernet_protocols, req.sdiag_protocol,
				  "ETH_P_???");
			tprintf(", pdiag_ino=%" PRIu32 ", pdiag_show=",
				req.pdiag_ino);
			printflags(packet_diag_show, req.pdiag_show,
				   "PACKET_SHOW_???");
			tprintf(", pdiag_cookie=[%" PRIu32 ", %" PRIu32 "]",
				req.pdiag_cookie[0], req.pdiag_cookie[1]);
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
	const size_t offset = sizeof(msg.pdiag_family);

	tprints("{pdiag_family=");
	printxval(addrfams, msg.pdiag_family, "AF_???");

	tprints(", ");
	if (len >= sizeof(msg)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(msg) - offset,
					 (void *) &msg + offset)) {
			tprints("pdiag_type=");
			printxval(socktypes, msg.pdiag_type, "SOCK_???");
			tprintf(", pdiag_num=%" PRIu16 ", pdiag_ino=%" PRIu32
				", pdiag_cookie=[%" PRIu32 ", %" PRIu32 "]",
				msg.pdiag_num, msg.pdiag_ino, msg.pdiag_cookie[0],
				msg.pdiag_cookie[1]);
		}
	} else
		tprints("...");
	tprints("}");
}

typedef void (*netlink_diag_decoder_t)(struct tcb *,
				       const struct nlmsghdr *,
				       uint8_t family,
				       kernel_ulong_t addr,
				       kernel_ulong_t len);

static const struct {
	const netlink_diag_decoder_t request, response;
} diag_decoders[] = {
	[AF_NETLINK] = { decode_netlink_diag_req, decode_netlink_diag_msg },
	[AF_PACKET] = { decode_packet_diag_req, decode_packet_diag_msg },
	[AF_UNIX] = { decode_unix_diag_req, decode_unix_diag_msg }
};

bool
decode_netlink_sock_diag(struct tcb *const tcp,
			 const struct nlmsghdr *const nlmsghdr,
			 const kernel_ulong_t addr,
			 const kernel_ulong_t len)
{
	uint8_t family;

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
