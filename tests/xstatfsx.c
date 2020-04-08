/*
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
	do {								\
		if (sizeof(b->arg) == sizeof(int))			\
			printf(", %s=%u", #arg,				\
			       (unsigned int) b->arg);			\
		else if (sizeof(b->arg) == sizeof(long))		\
			printf(", %s=%lu", #arg,			\
			       (unsigned long) b->arg);			\
		else							\
			printf(", %s=%llu", #arg,			\
			       (unsigned long long) b->arg);		\
	} while (0)

static void
print_statfs_type(const char *const prefix, const unsigned int magic)
{
	fputs(prefix, stdout);
	for (unsigned int i = 0; i < fsmagic->size; ++i)
		if (magic == fsmagic->data[i].val) {
			fputs(fsmagic->data[i].str, stdout);
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
