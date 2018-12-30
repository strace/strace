/*
 * Copyright (c) 2017-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_NSFS_H
# define STRACE_NSFS_H

# include <linux/ioctl.h>

# ifdef HAVE_LINUX_NSFS_H
#  include <linux/nsfs.h>
# else
#  define NSIO    0xb7
#  define NS_GET_USERNS   _IO(NSIO, 0x1)
#  define NS_GET_PARENT   _IO(NSIO, 0x2)
# endif

# ifndef NS_GET_NSTYPE
#  define NS_GET_NSTYPE    _IO(NSIO, 0x3)
# endif
# ifndef NS_GET_OWNER_UID
#  define NS_GET_OWNER_UID _IO(NSIO, 0x4)
# endif

#endif /* !STRACE_NSFS_H */
