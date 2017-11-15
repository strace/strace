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

#ifndef STRACE_NLATTR_H
#define STRACE_NLATTR_H

typedef bool (*nla_decoder_t)(struct tcb *, kernel_ulong_t addr,
			      unsigned int len, const void *opaque_data);
extern void
decode_nlattr(struct tcb *,
	      kernel_ulong_t addr,
	      unsigned int len,
	      const struct xlat *,
	      const char *dflt,
	      const nla_decoder_t *,
	      unsigned int size,
	      const void *opaque_data);

#define DECL_NLA(name)					\
extern bool						\
decode_nla_ ## name(struct tcb *, kernel_ulong_t addr,	\
		    unsigned int len, const void *)	\
/* End of DECL_NLA definition. */

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
DECL_NLA(str);
DECL_NLA(strn);
DECL_NLA(fd);
DECL_NLA(ifindex);
DECL_NLA(meminfo);
DECL_NLA(rt_class);
DECL_NLA(tc_stats);

#endif /* !STRACE_NLATTR_H */
