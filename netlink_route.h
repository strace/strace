/*
 * Copyright (c) 2016 Fabien Siron <fabien.siron@epita.fr>
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_NETLINK_ROUTE_H
# define STRACE_NETLINK_ROUTE_H

# define DECL_NETLINK_ROUTE_DECODER(route_decode_name)	\
void							\
route_decode_name(struct tcb *tcp,			\
		  const struct nlmsghdr *nlmsghdr,	\
		  uint8_t family,			\
		  kernel_ulong_t addr,			\
		  unsigned int len)			\
/* End of DECL_NETLINK_ROUTE_DECODER definition. */

extern DECL_NETLINK_ROUTE_DECODER(decode_br_port_msg);
extern DECL_NETLINK_ROUTE_DECODER(decode_dcbmsg);
extern DECL_NETLINK_ROUTE_DECODER(decode_fib_rule_hdr);
extern DECL_NETLINK_ROUTE_DECODER(decode_ifaddrlblmsg);
extern DECL_NETLINK_ROUTE_DECODER(decode_ifaddrmsg);
extern DECL_NETLINK_ROUTE_DECODER(decode_ifinfomsg);
extern DECL_NETLINK_ROUTE_DECODER(decode_ndmsg);
extern DECL_NETLINK_ROUTE_DECODER(decode_ndtmsg);
extern DECL_NETLINK_ROUTE_DECODER(decode_netconfmsg);
extern DECL_NETLINK_ROUTE_DECODER(decode_rtgenmsg);
extern DECL_NETLINK_ROUTE_DECODER(decode_rtm_getneigh);
extern DECL_NETLINK_ROUTE_DECODER(decode_rtmsg);
extern DECL_NETLINK_ROUTE_DECODER(decode_tcamsg);
extern DECL_NETLINK_ROUTE_DECODER(decode_tcmsg);

#endif /* !STRACE_NETLINK_ROUTE_H */
