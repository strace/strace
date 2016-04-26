/*
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
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

#define SYSCALL_INVOKE(file, desc, ptr, size) \
	syscall(SYSCALL_NR, SYSCALL_ARG(file, desc), size, ptr)
#define PRINT_SYSCALL_HEADER(file, desc, size) \
	printf("%s(" SYSCALL_ARG_FMT ", %u, ", SYSCALL_NAME, \
	       SYSCALL_ARG(file, desc), (unsigned) size)

#define STRUCT_STATFS	struct statfs64
#ifdef HAVE_STRUCT_STATFS64_F_FRSIZE
# define PRINT_F_FRSIZE
#endif
#ifdef HAVE_STRUCT_STATFS64_F_FLAGS
# define PRINT_F_FLAGS
#endif
#if defined HAVE_STRUCT_STATFS64_F_FSID_VAL
# define PRINT_F_FSID	f_fsid.val
#elif defined HAVE_STRUCT_STATFS64_F_FSID___VAL
# define PRINT_F_FSID	f_fsid.__val
#endif
#define CHECK_ODD_SIZE

#include "xstatfsx.c"
