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

#ifndef STRACE_NLATTR_H
#define STRACE_NLATTR_H

#include "xlat.h"

struct decode_nla_xlat_opts {
	const struct xlat *xlat;
	size_t xlat_size; /* is not needed for XT_NORMAL */
	const char *dflt;
	enum xlat_type xt;
	enum xlat_style style;
	const char *prefix;
	const char *suffix;
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
	char kind[16];
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

#define DECL_NLA(name)					\
extern bool						\
decode_nla_ ## name(struct tcb *, kernel_ulong_t addr,	\
		    unsigned int len, const void *)	\
/* End of DECL_NLA definition. */

DECL_NLA(x8);
DECL_NLA(x16);
DECL_NLA(x32);
DECL_NLA(x64);
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
DECL_NLA(ifindex);
DECL_NLA(ifla_af_spec);
DECL_NLA(ether_proto);
DECL_NLA(ip_proto);
DECL_NLA(in_addr);
DECL_NLA(in6_addr);
DECL_NLA(meminfo);
DECL_NLA(rt_class);
DECL_NLA(rt_proto);
DECL_NLA(rtnl_link_stats64);
DECL_NLA(tc_stats);

#endif /* !STRACE_NLATTR_H */
