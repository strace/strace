/*
 * Copyright (c) 2016 Fabien Siron <fabien.siron@epita.fr>
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2016-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <endian.h>
#include "netlink.h"
#include "nlattr.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/sock_diag.h>
#include "static_assert.h"

#include "xlat/netlink_sk_meminfo_indices.h"

static bool
fetch_nlattr(struct tcb *const tcp, struct nlattr *const nlattr,
	     const kernel_ulong_t addr, const unsigned int len,
	     const bool in_array)
{
	if (len < sizeof(struct nlattr)) {
		printstr_ex(tcp, addr, len, QUOTE_FORCE_HEX);
		return false;
	}

	if (tfetch_obj(tcp, addr, nlattr))
		return true;

	if (in_array) {
		tprints("...");
		printaddr_comment(addr);
	} else {
		printaddr(addr);
	}

	return false;
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
	const unsigned int nla_len = MIN(nla->nla_len, len);

	if (nla_len > NLA_HDRLEN)
		tprints("{");

	print_nlattr(nla, table, dflt);

	if (nla_len > NLA_HDRLEN) {
		const unsigned int idx =
			size ? nla->nla_type & NLA_TYPE_MASK : 0;

		tprints(", ");
		if (!decoders
		    || (size && idx >= size)
		    || !decoders[idx]
		    || !decoders[idx](
				tcp, addr + NLA_HDRLEN,
				nla_len - NLA_HDRLEN,
				size ? opaque_data
				     : (const void *) (uintptr_t) nla->nla_type)
		    )
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
	bool is_array = false;
	unsigned int elt;

	if (decoders && !size && opaque_data)
		error_func_msg("[xlat %p, dflt \"%s\", decoders %p] "
			       "size is zero (going to pass nla_type as "
			       "decoder argument), but opaque data (%p) is not "
			       "- will be ignored",
			       table, dflt, decoders, opaque_data);

	for (elt = 0; fetch_nlattr(tcp, &nla, addr, len, is_array); elt++) {
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

		if (!is_array && next_addr) {
			tprints("[");
			is_array = true;
		}

		decode_nlattr_with_data(tcp, &nla, addr, len, table, dflt,
					decoders, size, opaque_data);

		if (!next_addr)
			break;

		tprints(", ");
		addr = next_addr;
		len = next_len;
	}

	if (is_array) {
		tprints("]");
	}
}

bool
decode_nla_str(struct tcb *const tcp,
	       const kernel_ulong_t addr,
	       const unsigned int len,
	       const void *const opaque_data)
{
	printstr_ex(tcp, addr, len,
		    QUOTE_OMIT_TRAILING_0 | QUOTE_EXPECT_TRAILING_0);

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
	print_array_ex(tcp, addr, nmemb, &mem, sizeof(mem),
		       tfetch_mem, print_uint32_array_member, &count,
		       PAF_PRINT_INDICES | XLAT_STYLE_FMT_U,
		       netlink_sk_meminfo_indices, "SK_MEMINFO_???");

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
decode_nla_uid(struct tcb *const tcp,
	       const kernel_ulong_t addr,
	       const unsigned int len,
	       const void *const opaque_data)
{
	uint32_t uid;

	if (len < sizeof(uid))
		return false;
	else if (!umove_or_printaddr(tcp, addr, &uid))
		printuid("", uid);

	return true;
}

bool
decode_nla_gid(struct tcb *const tcp,
	       const kernel_ulong_t addr,
	       const unsigned int len,
	       const void *const opaque_data)
{
	return decode_nla_uid(tcp, addr, len, opaque_data);
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
decode_nla_xval(struct tcb *const tcp,
		const kernel_ulong_t addr,
		unsigned int len,
		const void *const opaque_data)
{
	const struct decode_nla_xlat_opts * const opts = opaque_data;
	union {
		uint64_t val;
		uint8_t  bytes[sizeof(uint64_t)];
	} data = { .val = 0 };

	if (len > sizeof(data) || len < opts->size)
		return false;

	if (opts->size)
		len = MIN(len, opts->size);

	const size_t bytes_offs = is_bigendian ? sizeof(data) - len : 0;

	if (!umoven_or_printaddr(tcp, addr, len, data.bytes + bytes_offs)) {
		if (opts->process_fn)
			data.val = opts->process_fn(data.val);
		if (opts->prefix)
			tprints(opts->prefix);
		printxval_ex(opts->xlat, data.val, opts->dflt, opts->style);
		if (opts->suffix)
			tprints(opts->suffix);
	}

	return true;
}

static uint64_t
process_host_order(uint64_t val)
{
	return ntohs(val);
}

bool
decode_nla_ether_proto(struct tcb *const tcp,
		       const kernel_ulong_t addr,
		       const unsigned int len,
		       const void *const opaque_data)
{
	static const struct decode_nla_xlat_opts opts = {
		.xlat = ethernet_protocols,
		.dflt = "ETHER_P_???",
		.prefix = "htons(",
		.suffix = ")",
		.size = 2,
		.process_fn = process_host_order,
	};

	return decode_nla_xval(tcp, addr, len, &opts);
}

bool
decode_nla_ip_proto(struct tcb *const tcp,
		    const kernel_ulong_t addr,
		    const unsigned int len,
		    const void *const opaque_data)
{
	static const struct decode_nla_xlat_opts opts = {
		.xlat = inet_protocols,
		.dflt = "IPPROTO_???",
		.size = 1,
	};

	return decode_nla_xval(tcp, addr, len, &opts);
}

bool
decode_nla_hwaddr(struct tcb *const tcp,
		const kernel_ulong_t addr,
		const unsigned int len,
		const void *const opaque_data)
{
	if (len > MAX_ADDR_LEN)
		return false;

	uint8_t buf[len];
	const uintptr_t arphrd = (uintptr_t) opaque_data;

	if (!umoven_or_printaddr(tcp, addr, len, buf)) {
		print_hwaddr("", buf, len, arphrd & NLA_HWADDR_FAMILY_OFFSET
				? arphrd & ~NLA_HWADDR_FAMILY_OFFSET : -1U);
	}

	return true;
}

bool
decode_nla_in_addr(struct tcb *const tcp,
		   const kernel_ulong_t addr,
		   const unsigned int len,
		   const void *const opaque_data)
{
	struct in_addr in;

	if (len < sizeof(in))
		return false;
	else if (!umove_or_printaddr(tcp, addr, &in))
		print_inet_addr(AF_INET, &in, sizeof(in), NULL);

	return true;
}

bool
decode_nla_in6_addr(struct tcb *const tcp,
		    const kernel_ulong_t addr,
		    const unsigned int len,
		    const void *const opaque_data)
{
	struct in6_addr in6;

	if (len < sizeof(in6))
		return false;
	else if (!umove_or_printaddr(tcp, addr, &in6))
		print_inet_addr(AF_INET6, &in6, sizeof(in6), NULL);

	return true;
}

bool
decode_nla_flags(struct tcb *const tcp,
		 const kernel_ulong_t addr,
		 unsigned int len,
		 const void *const opaque_data)
{
	const struct decode_nla_xlat_opts * const opts = opaque_data;
	union {
		uint64_t flags;
		uint8_t  bytes[sizeof(uint64_t)];
	} data = { .flags = 0 };

	if (len > sizeof(data) || len < opts->size)
		return false;

	if (opts->size)
		len = MIN(len, opts->size);

	const size_t bytes_offs = is_bigendian ? sizeof(data) - len : 0;

	if (!umoven_or_printaddr(tcp, addr, len, data.bytes + bytes_offs)) {
		if (opts->process_fn)
			data.flags = opts->process_fn(data.flags);
		if (opts->prefix)
			tprints(opts->prefix);
		printflags_ex(data.flags, opts->dflt, opts->style, opts->xlat,
			      NULL);
		if (opts->suffix)
			tprints(opts->suffix);
	}

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

DECODE_NLA_INTEGER(x8, uint8_t, "%#" PRIx8)
DECODE_NLA_INTEGER(x16, uint16_t, "%#" PRIx16)
DECODE_NLA_INTEGER(x32, uint32_t, "%#" PRIx32)
DECODE_NLA_INTEGER(x64, uint64_t, "%#" PRIx64)
DECODE_NLA_INTEGER(u8, uint8_t, "%" PRIu8)
DECODE_NLA_INTEGER(u16, uint16_t, "%" PRIu16)
DECODE_NLA_INTEGER(u32, uint32_t, "%" PRIu32)
DECODE_NLA_INTEGER(u64, uint64_t, "%" PRIu64)
DECODE_NLA_INTEGER(s8, int8_t, "%" PRId8)
DECODE_NLA_INTEGER(s16, int16_t, "%" PRId16)
DECODE_NLA_INTEGER(s32, int32_t, "%" PRId32)
DECODE_NLA_INTEGER(s64, int64_t, "%" PRId64)
