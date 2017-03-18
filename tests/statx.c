/*
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

#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_statx

# include <linux/stat.h>
# include "xlat.h"
# include "xlat/statx_masks.h"
# include "xlat/statx_attrs.h"
# include "xlat/at_statx_sync_types.h"

# define IS_STATX 1
# define TEST_SYSCALL_STR "statx"
# define STRUCT_STAT struct statx
# define STRUCT_STAT_STR "struct statx"
# define STRUCT_STAT_IS_STAT64 0

static unsigned    TEST_SYSCALL_STATX_FLAGS     = AT_STATX_SYNC_AS_STAT;
static const char *TEST_SYSCALL_STATX_FLAGS_STR = "AT_STATX_SYNC_AS_STAT";
static unsigned    TEST_SYSCALL_STATX_MASK      = STATX_ALL;
static const char *TEST_SYSCALL_STATX_MASK_STR  = "STATX_ALL";

# define TEST_SYSCALL_INVOKE(sample, pst) \
	syscall(__NR_statx, AT_FDCWD, sample, TEST_SYSCALL_STATX_FLAGS, \
	        TEST_SYSCALL_STATX_MASK, pst)
# define PRINT_SYSCALL_HEADER(sample) \
	do { \
		int saved_errno = errno; \
		printf("%s(AT_FDCWD, \"%s\", %s, %s, ", \
		       TEST_SYSCALL_STR, sample, TEST_SYSCALL_STATX_FLAGS_STR, \
		       TEST_SYSCALL_STATX_MASK_STR)
# define PRINT_SYSCALL_FOOTER(rc) \
		errno = saved_errno; \
		printf(") = %s\n", sprintrc(rc)); \
	} while (0)

# include "xstatx.c"

#else

SKIP_MAIN_UNDEFINED("__NR_statx")

#endif
