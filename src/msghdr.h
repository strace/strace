/*
 * Copyright (c) 2016-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_MSGHDR_H
# define STRACE_MSGHDR_H

/* For definitions of struct msghdr and struct mmsghdr. */
# include <sys/socket.h>

# ifndef HAVE_STRUCT_MMSGHDR
struct mmsghdr {
	struct msghdr msg_hdr;
	unsigned msg_len;
};
# endif

struct tcb;

extern void
print_struct_msghdr(struct tcb *, const struct msghdr *,
		    const int *p_user_msg_namelen, kernel_ulong_t data_size);

#endif /* !STRACE_MSGHDR_H */
