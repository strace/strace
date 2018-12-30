/*
 * Copyright (c) 2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_F_OWNER_EX_H
# define STRACE_F_OWNER_EX_H

# include <linux/fcntl.h>

# if defined HAVE_STRUCT_F_OWNER_EX
typedef struct f_owner_ex struct_kernel_f_owner_ex;
# elif defined HAVE_STRUCT___KERNEL_F_OWNER_EX
typedef struct __kernel_f_owner_ex struct_kernel_f_owner_ex;
# else
#  error struct f_owner_ex definition not found in <linux/fcntl.h>
# endif

#endif /* !STRACE_F_OWNER_EX_H */
