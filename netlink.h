/*
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

#ifndef STRACE_NETLINK_H
#define STRACE_NETLINK_H

#include <sys/socket.h>
#include <linux/netlink.h>

#ifndef NETLINK_SOCK_DIAG
# define NETLINK_SOCK_DIAG 4
#endif

#ifndef NLM_F_NONREC
# define NLM_F_NONREC	0x100
#endif
#ifndef NLM_F_CAPPED
# define NLM_F_CAPPED	0x100
#endif

#undef NLMSG_HDRLEN
#define NLMSG_HDRLEN ((unsigned int) NLMSG_ALIGN(sizeof(struct nlmsghdr)))

#ifndef NLMSG_MIN_TYPE
# define NLMSG_MIN_TYPE		0x10
#endif

#ifndef NLA_ALIGN
# define NLA_ALIGN(len) (((len) + 3) & ~3)
#endif

#undef NLA_HDRLEN
#define NLA_HDRLEN ((unsigned int) NLA_ALIGN(sizeof(struct nlattr)))

#ifndef NLA_TYPE_MASK
# define NLA_F_NESTED		(1 << 15)
# define NLA_F_NET_BYTEORDER	(1 << 14)
# define NLA_TYPE_MASK		~(NLA_F_NESTED | NLA_F_NET_BYTEORDER)
#endif

#endif /* !STRACE_NETLINK_H */
