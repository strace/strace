/*
 * Copyright (c) 2016 Fabien Siron <fabien.siron@epita.fr>
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2016-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_NLATTR_H
# define STRACE_NLATTR_H

# include "xlat.h"

struct decode_nla_xlat_opts {
	const struct xlat *xlat;
	const char *dflt;
	enum xlat_style style;
	const char *fn_str;
	uint64_t (*process_fn)(uint64_t val);
	size_t size;
};

/*
 * Used for IFLA_LINKINFO decoding.  Since there are no other indicators
 * regarding the nature of data except for previously provided string
 * in an IFLA_LINKINFO_KIND attribute, we have to store it in order to pass
 * between calls as an opaque data.
 */
struct ifla_linkinfo_ctx {
	char kind[64];
	char slave_kind[64];
};

typedef bool (*nla_decoder_t)(struct tcb *, kernel_ulong_t addr,
			      unsigned int len, const void *opaque_data);

/**
 * The case of non-NULL decoders and zero size is handled in a special way:
 * the zeroth decoder is always called with nla_type being passed as opaque
 * data.
 */
extern void
decode_nlattr(struct tcb *,
	      kernel_ulong_t addr,
	      unsigned int len,
	      const struct xlat *,
	      const char *dflt,
	      const nla_decoder_t *decoders,
	      unsigned int size,
	      const void *opaque_data);

/**
 * The special case of decode_nlattr with zero size that takes only
 * a single decoder, and the type is being passed via opaque_data.
 */
static inline void
decode_nlattr_notype(struct tcb *tcp,
		     kernel_ulong_t addr,
		     unsigned int len,
		     const struct xlat *table,
		     const char *dflt,
		     const nla_decoder_t decoder,
		     const void *opaque_data)
{
	decode_nlattr(tcp, addr, len, table, dflt, &decoder, -1U, opaque_data);
}

# define DECL_NLA(name)					\
bool							\
decode_nla_ ## name(struct tcb *const tcp,		\
		    const kernel_ulong_t addr,		\
		    const unsigned int len,		\
		    const void *const opaque_data)	\
/* End of DECL_NLA definition. */

DECL_NLA(x8);
DECL_NLA(x16);
DECL_NLA(x32);
DECL_NLA(x64);
DECL_NLA(xint);
DECL_NLA(u8);
DECL_NLA(u16);
DECL_NLA(u32);
DECL_NLA(u64);
DECL_NLA(s8);
DECL_NLA(s16);
DECL_NLA(s32);
DECL_NLA(s64);
DECL_NLA(be16);
DECL_NLA(be64);
DECL_NLA(xval);
DECL_NLA(flags);
DECL_NLA(str);
DECL_NLA(strn);
DECL_NLA(fd);
DECL_NLA(uid);
DECL_NLA(gid);
DECL_NLA(clock_t);
DECL_NLA(ifindex);
DECL_NLA(ifla_af_spec);
DECL_NLA(ether_proto);
DECL_NLA(ip_proto);
DECL_NLA(in_addr);
DECL_NLA(in6_addr);
DECL_NLA(lwt_encap_type);
DECL_NLA(meminfo);
DECL_NLA(rt_class);
DECL_NLA(rt_proto);
DECL_NLA(rtnl_link_stats64);
DECL_NLA(tc_stats);

# define NLA_HWADDR_FAMILY_OFFSET 1024

/**
 * Print hardware (low-level, L2) address.
 * @param opaque_data Interpreted as integer value, not pointer
 */
DECL_NLA(hwaddr);

/** Non-standard decoder that accepts ARPHRD_* value as an argument */
static inline bool
decode_nla_hwaddr_family(struct tcb *const tcp,
		const kernel_ulong_t addr,
		const unsigned int len,
		const unsigned int arphrd)
{
	return decode_nla_hwaddr(tcp, addr, len, (void *) (uintptr_t) (
				 arphrd > NLA_HWADDR_FAMILY_OFFSET ? 0
					: NLA_HWADDR_FAMILY_OFFSET | arphrd));
}

/** decode_nla_hwaddr wrapper that ignores opaque_data */
static inline bool
decode_nla_hwaddr_nofamily(struct tcb *const tcp,
		const kernel_ulong_t addr,
		const unsigned int len,
		const void *const opaque_data)
{
	return decode_nla_hwaddr(tcp, addr, len, NULL);
}

/* Common handling for AF_SPEC-type decoders */

struct af_spec_decoder_desc {
	uint8_t af;
	const struct xlat *xlat;
	const char *dflt;
	const nla_decoder_t *table;
	size_t size;
};

/**
 * Non-standard decoder for handling AF_SPEC-type netlink attributes.
 *
 * @param af        Address family (AF_*), as passed to decoder in opaque_data
 *                  parameter through zero-sized decoder table decode_nlattr
 *                  hack.
 * @param descs     List of supported decoders.
 * @param desc_cnt  Number of items in descs.
 */
extern void decode_nla_af_spec(struct tcb *const tcp,
			       const kernel_ulong_t addr,
			       const unsigned int len,
			       uint8_t af,
			       const struct af_spec_decoder_desc *descs,
			       size_t desc_cnt);

#endif /* !STRACE_NLATTR_H */
