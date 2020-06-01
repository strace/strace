/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_KERNEL_TYPES_H
# define STRACE_KERNEL_TYPES_H

# if defined HAVE___KERNEL_LONG_T && defined HAVE___KERNEL_ULONG_T

#  include <asm/posix_types.h>

typedef __kernel_long_t kernel_long_t;
typedef __kernel_ulong_t kernel_ulong_t;

# elif (defined __x86_64__ && defined __ILP32__) || defined LINUX_MIPSN32

typedef long long kernel_long_t;
typedef unsigned long long kernel_ulong_t;

# else

typedef long kernel_long_t;
typedef unsigned long kernel_ulong_t;

# endif

# if SIZEOF_KERNEL_LONG_T > SIZEOF_LONG
#  define PRI_kl "ll"
# else
#  define PRI_kl "l"
# endif

# define PRI_kld PRI_kl"d"
# define PRI_klu PRI_kl"u"
# define PRI_klx PRI_kl"x"

/*
 * The kernel used to define 64-bit types on 64-bit systems on a per-arch
 * basis.  Some architectures would use unsigned long and others would use
 * unsigned long long.  These types were exported as part of the
 * kernel-userspace ABI and now must be maintained forever.  This matches
 * what the kernel exports for each architecture so we don't need to cast
 * every printing of __u64 or __s64 to stdint types.
 * The exception is Android, where for MIPS64 unsigned long long is used.
 */
# if SIZEOF_LONG == 4
#  define PRI__64 "ll"
# elif defined ALPHA || defined IA64 || defined __powerpc64__ \
      || (defined MIPS && !defined __ANDROID__)
#  define PRI__64 "l"
# else
#  define PRI__64 "ll"
# endif

# define PRI__d64 PRI__64"d"
# define PRI__u64 PRI__64"u"
# define PRI__x64 PRI__64"x"

#endif /* !STRACE_KERNEL_TYPES_H */
