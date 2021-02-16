/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_NETLINK_SOCK_DIAG_H
# define STRACE_NETLINK_SOCK_DIAG_H

# define DECL_NETLINK_DIAG_DECODER(diag_decode_name)	\
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

# define PRINT_FIELD_INET_DIAG_SOCKID(where_, field_, af_)		\
	do {								\
		tprints_field_name(#field_);				\
		print_inet_diag_sockid(&(where_).field_, (af_));	\
	} while (0)


#endif /* !STRACE_NETLINK_SOCK_DIAG_H */
