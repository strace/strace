/*
 * Copyright (c) 2016 Fabien Siron <fabien.siron@epita.fr>
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2016-2018 The strace developers.
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
#include <endian.h>
#include "netlink.h"
#include "nlattr.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/sock_diag.h>
#include "static_assert.h"

static bool
fetch_nlattr(struct tcb *const tcp, struct nlattr *const nlattr,
	     const kernel_ulong_t addr, const unsigned int len)
{
	if (len < sizeof(struct nlattr)) {
		printstr_ex(tcp, addr, len, QUOTE_FORCE_HEX);
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
	static_assert(NLA_TYPE_MASK == ~(NLA_F_NESTED | NLA_F_NET_BYTEORDER),
		      "wrong NLA_TYPE_MASK");

	tprintf("{nla_len=%u, nla_type=", nla->nla_len);
	if (nla->nla_type & NLA_F_NESTED) {
		print_xlat(NLA_F_NESTED);
		tprints("|");
	}
	if (nla->nla_type & NLA_F_NET_BYTEORDER) {
		print_xlat(NLA_F_NET_BYTEORDER);
		tprints("|");
	}
	printxval(table, nla->nla_type & NLA_TYPE_MASK, dflt);
	tprints("}");
}

static void
decode_nlattr_with_data(struct tcb *const tcp,
			const struct nlattr *const nla,
			const kernel_ulong_t addr,
			const unsigned int len,
			const struct xlat *const table,
			const char *const dflt,
			const nla_decoder_t *const decoders,
			const unsigned int size,
			const void *const opaque_data)
{
	const unsigned int nla_len = nla->nla_len > len ? len : nla->nla_len;

	if (nla_len > NLA_HDRLEN)
		tprints("{");

	print_nlattr(nla, table, dflt);

	if (nla_len > NLA_HDRLEN) {
		tprints(", ");
		if (!decoders
		    || nla->nla_type >= size
		    || !decoders[nla->nla_type]
		    || !decoders[nla->nla_type](tcp, addr + NLA_HDRLEN,
						nla_len - NLA_HDRLEN,
						opaque_data))
			printstr_ex(tcp, addr + NLA_HDRLEN,
				    nla_len - NLA_HDRLEN, QUOTE_FORCE_HEX);
		tprints("}");
	}
}

void
decode_nlattr(struct tcb *const tcp,
	      kernel_ulong_t addr,
	      unsigned int len,
	      const struct xlat *const table,
	      const char *const dflt,
	      const nla_decoder_t *const decoders,
	      const unsigned int size,
	      const void *const opaque_data)
{
	struct nlattr nla;
	bool print_array = false;
	unsigned int elt;

	for (elt = 0; fetch_nlattr(tcp, &nla, addr, len); elt++) {
		if (abbrev(tcp) && elt == max_strlen) {
			tprints("...");
			break;
		}

		const unsigned int nla_len = NLA_ALIGN(nla.nla_len);
		kernel_ulong_t next_addr = 0;
		unsigned int next_len = 0;

		if (nla.nla_len >= NLA_HDRLEN) {
			next_len = (len >= nla_len) ? len - nla_len : 0;

			if (next_len && addr + nla_len > addr)
				next_addr = addr + nla_len;
		}

		if (!print_array && next_addr) {
			tprints("[");
			print_array = true;
		}

		decode_nlattr_with_data(tcp, &nla, addr, len, table, dflt,
					decoders, size, opaque_data);

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

bool
decode_nla_str(struct tcb *const tcp,
	       const kernel_ulong_t addr,
	       const unsigned int len,
	       const void *const opaque_data)
{
	printstr_ex(tcp, addr, len, QUOTE_0_TERMINATED);

	return true;
}

bool
decode_nla_strn(struct tcb *const tcp,
		const kernel_ulong_t addr,
		const unsigned int len,
		const void *const opaque_data)
{
	printstrn(tcp, addr, len);

	return true;
}

static bool
print_meminfo(struct tcb *const tcp,
	      void *const elem_buf,
	      const size_t elem_size,
	      void *const opaque_data)
{
	unsigned int *const count = opaque_data;

	if ((*count)++ >= SK_MEMINFO_VARS) {
		tprints("...");
		return false;
	}

	tprintf("%" PRIu32, *(uint32_t *) elem_buf);

	return true;
}

bool
decode_nla_meminfo(struct tcb *const tcp,
		   const kernel_ulong_t addr,
		   const unsigned int len,
		   const void *const opaque_data)
{
	uint32_t mem;
	const size_t nmemb = len / sizeof(mem);

	if (!nmemb)
		return false;

	unsigned int count = 0;
	print_array(tcp, addr, nmemb, &mem, sizeof(mem),
		    umoven_or_printaddr, print_meminfo, &count);

	return true;
}

bool
decode_nla_fd(struct tcb *const tcp,
	      const kernel_ulong_t addr,
	      const unsigned int len,
	      const void *const opaque_data)
{
	int fd;

	if (len < sizeof(fd))
		return false;
	else if (!umove_or_printaddr(tcp, addr, &fd))
		printfd(tcp, fd);

	return true;
}

bool
decode_nla_ifindex(struct tcb *const tcp,
	       const kernel_ulong_t addr,
	       const unsigned int len,
	       const void *const opaque_data)
{
	uint32_t ifindex;

	if (len < sizeof(ifindex))
		return false;
	else if (!umove_or_printaddr(tcp, addr, &ifindex))
		print_ifindex(ifindex);

	return true;
}

bool
decode_nla_be16(struct tcb *const tcp,
		const kernel_ulong_t addr,
		const unsigned int len,
		const void *const opaque_data)
{
	uint16_t num;

	if (len < sizeof(num))
		return false;
	else if (!umove_or_printaddr(tcp, addr, &num))
		tprintf("htons(%u)", ntohs(num));

	return true;
}

bool
decode_nla_be64(struct tcb *const tcp,
		const kernel_ulong_t addr,
		const unsigned int len,
		const void *const opaque_data)
{
#if defined HAVE_BE64TOH || defined be64toh
	uint64_t num;

	if (len < sizeof(num))
		return false;
	else if (!umove_or_printaddr(tcp, addr, &num))
		tprintf("htobe64(%" PRIu64 ")", be64toh(num));

	return true;
#else
	return false;
#endif
}

#define DECODE_NLA_INTEGER(name, type, fmt)		\
bool							\
decode_nla_ ## name(struct tcb *const tcp,		\
		    const kernel_ulong_t addr,		\
		    const unsigned int len,		\
		    const void *const opaque_data)	\
{							\
	type num;					\
							\
	if (len < sizeof(num))				\
		return false;				\
	if (!umove_or_printaddr(tcp, addr, &num))	\
		tprintf(fmt, num);			\
	return true;					\
}

DECODE_NLA_INTEGER(u8, uint8_t, "%" PRIu8)
DECODE_NLA_INTEGER(u16, uint16_t, "%" PRIu16)
DECODE_NLA_INTEGER(u32, uint32_t, "%" PRIu32)
DECODE_NLA_INTEGER(u64, uint64_t, "%" PRIu64)
DECODE_NLA_INTEGER(s8, int8_t, "%" PRId8)
DECODE_NLA_INTEGER(s16, int16_t, "%" PRId16)
DECODE_NLA_INTEGER(s32, int32_t, "%" PRId32)
DECODE_NLA_INTEGER(s64, int64_t, "%" PRId64)
