/*
 * Copyright (c) 2016 Fabien Siron <fabien.siron@epita.fr>
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

#include "defs.h"
#include <sys/socket.h>
#include <linux/netlink.h>
#include "xlat/netlink_flags.h"
#include "xlat/netlink_types.h"

#undef NLMSG_HDRLEN
#define NLMSG_HDRLEN NLMSG_ALIGN(sizeof(struct nlmsghdr))

/*
 * Fetch a struct nlmsghdr from the given address.
 */
static bool
fetch_nlmsghdr(struct tcb *const tcp, struct nlmsghdr *const nlmsghdr,
	       const kernel_ulong_t addr, const kernel_ulong_t len)
{
	if (len < sizeof(struct nlmsghdr)) {
		printstrn(tcp, addr, len);
		return false;
	}

	if (umove_or_printaddr(tcp, addr, nlmsghdr))
		return false;

	return true;
}

static void
print_nlmsghdr(struct tcb *tcp, const struct nlmsghdr *const nlmsghdr)
{
	/* print the whole structure regardless of its nlmsg_len */

	tprintf("{len=%u, type=", nlmsghdr->nlmsg_len);

	printxval(netlink_types, nlmsghdr->nlmsg_type, "NLMSG_???");

	tprints(", flags=");
	printflags(netlink_flags, nlmsghdr->nlmsg_flags, "NLM_F_???");

	tprintf(", seq=%u, pid=%u}", nlmsghdr->nlmsg_seq,
		nlmsghdr->nlmsg_pid);
}

static void
decode_nlmsghdr_with_payload(struct tcb *const tcp,
			     const struct nlmsghdr *const nlmsghdr,
			     const kernel_ulong_t addr,
			     const kernel_ulong_t len);

static void
decode_nlmsgerr(struct tcb *const tcp,
	       kernel_ulong_t addr,
	       kernel_ulong_t len)
{
	struct nlmsgerr err;

	if (umove_or_printaddr(tcp, addr, &err.error))
		return;

	tprints("{error=");
	if (err.error < 0 && (unsigned) -err.error < nerrnos) {
		tprintf("-%s", errnoent[-err.error]);
	} else {
		tprintf("%d", err.error);
	}

	addr += offsetof(struct nlmsgerr, msg);
	len -= offsetof(struct nlmsgerr, msg);

	if (len) {
		tprints(", msg=");
		if (fetch_nlmsghdr(tcp, &err.msg, addr, len)) {
			decode_nlmsghdr_with_payload(tcp, &err.msg, addr, len);
		}
	}

	tprints("}");
}

static void
decode_payload(struct tcb *const tcp,
	       const struct nlmsghdr *const nlmsghdr,
	       const kernel_ulong_t addr,
	       const kernel_ulong_t len)
{
	if (nlmsghdr->nlmsg_type == NLMSG_ERROR && len >= sizeof(int)) {
		decode_nlmsgerr(tcp, addr, len);
	} else {
		printstrn(tcp, addr, len);
	}
}

static void
decode_nlmsghdr_with_payload(struct tcb *const tcp,
			     const struct nlmsghdr *const nlmsghdr,
			     const kernel_ulong_t addr,
			     const kernel_ulong_t len)
{
	tprints("{");

	print_nlmsghdr(tcp, nlmsghdr);

	unsigned int nlmsg_len =
		nlmsghdr->nlmsg_len > len ? len : nlmsghdr->nlmsg_len;
	if (nlmsg_len > NLMSG_HDRLEN) {
		tprints(", ");
		decode_payload(tcp, nlmsghdr, addr + NLMSG_HDRLEN,
					 nlmsg_len - NLMSG_HDRLEN);
	}

	tprints("}");
}

void
decode_netlink(struct tcb *const tcp, kernel_ulong_t addr, kernel_ulong_t len)
{
	struct nlmsghdr nlmsghdr;
	bool print_array = false;
	unsigned int elt;

	for (elt = 0; fetch_nlmsghdr(tcp, &nlmsghdr, addr, len); elt++) {
		if (abbrev(tcp) && elt == max_strlen) {
			tprints("...");
			break;
		}

		unsigned int nlmsg_len = NLMSG_ALIGN(nlmsghdr.nlmsg_len);
		kernel_ulong_t next_addr = 0;
		kernel_ulong_t next_len = 0;

		if (nlmsghdr.nlmsg_len >= NLMSG_HDRLEN) {
			next_len = (len >= nlmsg_len) ? len - nlmsg_len : 0;

			if (next_len && addr + nlmsg_len > addr)
				next_addr = addr + nlmsg_len;
		}

		if (!print_array && next_addr) {
			tprints("[");
			print_array = true;
		}

		decode_nlmsghdr_with_payload(tcp, &nlmsghdr, addr, len);

		if (!next_addr)
			break;

		tprints(", ");
		addr = next_addr;
		len = next_len;
	}

	if (print_array) {
		tprints("]");
	}
}
