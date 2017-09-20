/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2017 The strace developers.
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

#ifndef STRACE_KERNEL_TYPES_H
#define STRACE_KERNEL_TYPES_H

# if defined HAVE___KERNEL_LONG_T && defined HAVE___KERNEL_ULONG_T

# include <asm/posix_types.h>

typedef __kernel_long_t kernel_long_t;
typedef __kernel_ulong_t kernel_ulong_t;

# elif (defined __x86_64__ && defined __ILP32__) || defined LINUX_MIPSN32

typedef long long kernel_long_t;
typedef unsigned long long kernel_ulong_t;

# else

typedef long kernel_long_t;
typedef unsigned long kernel_ulong_t;

# endif

typedef struct {
	kernel_ulong_t	d_ino;
	kernel_ulong_t	d_off;
	unsigned short	d_reclen;
	char		d_name[1];
} kernel_dirent;

#if SIZEOF_KERNEL_LONG_T > SIZEOF_LONG
# define PRI_kl "ll"
#else
# define PRI_kl "l"
#endif

#define PRI_kld PRI_kl"d"
#define PRI_klu PRI_kl"u"
#define PRI_klx PRI_kl"x"

/*
 * The kernel used to define 64-bit types on 64-bit systems on a per-arch
 * basis.  Some architectures would use unsigned long and others would use
 * unsigned long long.  These types were exported as part of the
 * kernel-userspace ABI and now must be maintained forever.  This matches
 * what the kernel exports for each architecture so we don't need to cast
 * every printing of __u64 or __s64 to stdint types.
 * The exception is Android, where for MIPS64 unsigned long long is used.
 */
#if SIZEOF_LONG == 4
# define PRI__64 "ll"
#elif defined ALPHA || defined IA64 || defined __powerpc64__ \
      || (defined MIPS && !defined __ANDROID__)
# define PRI__64 "l"
#else
# define PRI__64 "ll"
#endif

#define PRI__d64 PRI__64"d"
#define PRI__u64 PRI__64"u"
#define PRI__x64 PRI__64"x"

#endif /* !STRACE_KERNEL_TYPES_H */
