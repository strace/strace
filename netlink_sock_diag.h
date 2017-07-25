/*
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

#ifndef STRACE_NETLINK_SOCK_DIAG_H
#define STRACE_NETLINK_SOCK_DIAG_H

#define DECL_NETLINK_DIAG_DECODER(diag_decode_name)	\
void							\
diag_decode_name(struct tcb *tcp,			\
		 const struct nlmsghdr *nlmsghdr,	\
		 uint8_t family,			\
		 kernel_ulong_t addr,			\
		 unsigned int len)			\
/* End of DECL_NETLINK_DIAG_DECODER definition. */

extern DECL_NETLINK_DIAG_DECODER(decode_inet_diag_msg);
extern DECL_NETLINK_DIAG_DECODER(decode_inet_diag_req);
extern DECL_NETLINK_DIAG_DECODER(decode_netlink_diag_msg);
extern DECL_NETLINK_DIAG_DECODER(decode_netlink_diag_req);
extern DECL_NETLINK_DIAG_DECODER(decode_packet_diag_msg);
extern DECL_NETLINK_DIAG_DECODER(decode_packet_diag_req);
extern DECL_NETLINK_DIAG_DECODER(decode_smc_diag_msg);
extern DECL_NETLINK_DIAG_DECODER(decode_smc_diag_req);
extern DECL_NETLINK_DIAG_DECODER(decode_unix_diag_msg);
extern DECL_NETLINK_DIAG_DECODER(decode_unix_diag_req);

struct inet_diag_sockid;

extern void
print_inet_diag_sockid(const struct inet_diag_sockid *, const uint8_t family);

#define PRINT_FIELD_INET_DIAG_SOCKID(prefix_, where_, field_, af_)	\
	do {								\
		STRACE_PRINTF("%s%s=", (prefix_), #field_);		\
		print_inet_diag_sockid(&(where_).field_, (af_));	\
	} while (0)


#endif /* !STRACE_NETLINK_SOCK_DIAG_H */
