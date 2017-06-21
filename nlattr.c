/*
 * Copyright (c) 2016 Fabien Siron <fabien.siron@epita.fr>
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2016-2017 The strace developers.
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

static bool
fetch_nlattr(struct tcb *const tcp, struct nlattr *const nlattr,
	     const kernel_ulong_t addr, const kernel_ulong_t len)
{
	if (len < sizeof(struct nlattr)) {
		printstrn(tcp, addr, len);
		return false;
	}

	if (umove_or_printaddr(tcp, addr, nlattr))
		return false;

	return true;
}

static void
print_nlattr(const struct nlattr *const nla,
	     const struct xlat *const table,
	     const char *const dflt)
{
	tprintf("{nla_len=%u, nla_type=", nla->nla_len);
	if (nla->nla_type & NLA_F_NESTED)
		tprints("NLA_F_NESTED|");
	if (nla->nla_type & NLA_F_NET_BYTEORDER)
		tprints("NLA_F_NET_BYTEORDER|");
	printxval(table, nla->nla_type & NLA_TYPE_MASK, dflt);
	tprints("}");
}

static void
decode_nlattr_with_data(struct tcb *tcp,
			const struct nlattr *const nla,
			kernel_ulong_t addr,
			kernel_ulong_t len,
			const struct xlat *const table,
			const char *const dflt)
{
	const unsigned int nla_len = nla->nla_len > len ? len : nla->nla_len;

	if (nla_len > NLA_HDRLEN)
		tprints("{");

	print_nlattr(nla, table, dflt);

	if (nla_len > NLA_HDRLEN) {
		tprints(", ");
		printstrn(tcp, addr + NLA_HDRLEN, nla_len - NLA_HDRLEN);
		tprints("}");
	}
}

void
decode_nlattr(struct tcb *const tcp,
	      kernel_ulong_t addr,
	      kernel_ulong_t len,
	      const struct xlat *const table,
	      const char *const dflt)
{
	struct nlattr nla;
	bool print_array = false;
	unsigned int elt;

	for (elt = 0; fetch_nlattr(tcp, &nla, addr, len); elt++) {
		if (abbrev(tcp) && elt == max_strlen) {
			tprints("...");
			break;
		}

		const unsigned long nla_len = NLA_ALIGN(nla.nla_len);
		kernel_ulong_t next_addr = 0;
		kernel_ulong_t next_len = 0;

		if (nla.nla_len >= NLA_HDRLEN) {
			next_len = (len >= nla_len) ? len - nla_len : 0;

			if (next_len && addr + nla_len > addr)
				next_addr = addr + nla_len;
		}

		if (!print_array && next_addr) {
			tprints("[");
			print_array = true;
		}

		decode_nlattr_with_data(tcp, &nla, addr, len, table, dflt);

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
