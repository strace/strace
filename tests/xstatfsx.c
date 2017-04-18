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

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include <linux/types.h>
#include <asm/statfs.h>

#include "xlat.h"
#include "xlat/fsmagic.h"
#include "xlat/statfs_flags.h"

#define PRINT_NUM(arg)							\
	if (sizeof(b->arg) == sizeof(int))				\
		printf(", %s=%u", #arg, (unsigned int) b->arg);		\
	else if (sizeof(b->arg) == sizeof(long))				\
		printf(", %s=%lu", #arg, (unsigned long) b->arg);	\
	else								\
		printf(", %s=%llu", #arg, (unsigned long long) b->arg)

static void
print_statfs_type(const char *const prefix, const unsigned int magic)
{
	fputs(prefix, stdout);
	unsigned int i;
	for (i = 0; i < ARRAY_SIZE(fsmagic); ++i)
		if (magic == fsmagic[i].val) {
			fputs(fsmagic[i].str, stdout);
			return;
		}
	printf("%#x", magic);
}

static void
print_statfs(const char *const sample, const char *magic_str)
{
	int fd = open(sample, O_RDONLY);
	if (fd < 0)
		perror_msg_and_skip("open: %s", sample);

	TAIL_ALLOC_OBJECT_CONST_PTR(STRUCT_STATFS, b);
	long rc = SYSCALL_INVOKE(sample, fd, b, sizeof(*b));
	if (rc)
		perror_msg_and_skip(SYSCALL_NAME);

	PRINT_SYSCALL_HEADER(sample, fd, sizeof(*b));
	if (magic_str)
		printf("{f_type=%s", magic_str);
	else
		print_statfs_type("{f_type=", b->f_type);
	PRINT_NUM(f_bsize);
	PRINT_NUM(f_blocks);
	PRINT_NUM(f_bfree);
	PRINT_NUM(f_bavail);
	PRINT_NUM(f_files);
	PRINT_NUM(f_ffree);
#ifdef PRINT_F_FSID
	printf(", f_fsid={val=[%u, %u]}",
	       (unsigned) b->PRINT_F_FSID[0], (unsigned) b->PRINT_F_FSID[1]);
#endif
	PRINT_NUM(f_namelen);
#ifdef PRINT_F_FRSIZE
	PRINT_NUM(f_frsize);
#endif
#ifdef PRINT_F_FLAGS
	if (b->f_flags & ST_VALID) {
		printf(", f_flags=");
		printflags(statfs_flags, b->f_flags, "ST_???");
	}
#endif
	printf("}) = 0\n");
}

int
main(void)
{
	print_statfs("/proc/self/status", "PROC_SUPER_MAGIC");

	print_statfs(".", NULL);

	long rc = SYSCALL_INVOKE("", -1, 0, sizeof(STRUCT_STATFS));
	const char *errstr = sprintrc(rc);
	PRINT_SYSCALL_HEADER("", -1, sizeof(STRUCT_STATFS));
	printf("NULL) = %s\n", errstr);

#ifdef CHECK_ODD_SIZE
	const unsigned long addr = (unsigned long) 0xfacefeeddeadbeefULL;
	rc = SYSCALL_INVOKE("", -1, addr, sizeof(STRUCT_STATFS) + 1);
	errstr = sprintrc(rc);
	PRINT_SYSCALL_HEADER("", -1, sizeof(STRUCT_STATFS) + 1);
	printf("%#lx) = %s\n", addr, errstr);
#endif

	puts("+++ exited with 0 +++");
	return 0;
}
