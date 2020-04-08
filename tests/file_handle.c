/*
 * Check decoding of name_to_handle_at and open_by_handle_at syscalls.
 *
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2015-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#if defined __NR_name_to_handle_at && defined __NR_open_by_handle_at

# include <assert.h>
# include <errno.h>
# include <inttypes.h>
# include <fcntl.h>
# include <stdio.h>
# include <unistd.h>

enum assert_rc {
	ASSERT_NONE,
	ASSERT_SUCCESS,
	ASSERT_ERROR,
};

# ifndef MAX_HANDLE_SZ

#  define MAX_HANDLE_SZ 128

struct file_handle {
	unsigned int handle_bytes;
	int handle_type;
	unsigned char f_handle[0];
};
# endif /* !MAX_HANDLE_SZ */


void
print_handle_data(unsigned char *bytes, unsigned int size)
{
	unsigned int i;

	if (size > MAX_HANDLE_SZ)
		size = MAX_HANDLE_SZ;

	printf("0x");
	for (i = 0; i < size; ++i)
		printf("%02x", bytes[i]);
}

void
do_name_to_handle_at(kernel_ulong_t dirfd, const char *dirfd_str,
		     kernel_ulong_t pathname, const char *pathname_str,
		     kernel_ulong_t handle, const char *handle_str,
		     kernel_ulong_t mount_id,
		     kernel_ulong_t flags, const char *flags_str,
		     enum assert_rc assert_rc, long assert_errno)
{
	long rc;
	const char *errstr;

	rc = syscall(__NR_name_to_handle_at, dirfd, pathname, handle, mount_id,
		flags);
	errstr = sprintrc(rc);

	if (assert_rc != ASSERT_NONE)
		assert(rc == (assert_rc == ASSERT_SUCCESS ? 0 : -1));
	if (assert_errno)
		assert(errno != assert_errno);

	printf("name_to_handle_at(%s, %s, %s",
	       dirfd_str, pathname_str, handle_str);

	if (rc != -1) {
		struct file_handle *fh =
			(struct file_handle *) (uintptr_t) handle;
		int *mount_id_ptr = (int *) (uintptr_t) mount_id;

		printf(" => %u, handle_type=%d, f_handle=",
			fh->handle_bytes, fh->handle_type);
		print_handle_data((unsigned char *) fh +
				  sizeof(struct file_handle),
				  fh->handle_bytes);
		printf("}, [%d]", *mount_id_ptr);
	} else {
		if (mount_id)
			printf(", %#llx", (unsigned long long) mount_id);
		else
			printf(", NULL");
	}

	printf(", %s) = %s\n", flags_str, errstr);
}

void
do_open_by_handle_at(kernel_ulong_t mount_fd,
		     kernel_ulong_t handle, bool valid_handle, bool valid_data,
		     kernel_ulong_t flags, const char *flags_str)
{
	long rc;

	printf("open_by_handle_at(%d, ", (int) mount_fd);
	if (valid_handle) {
		struct file_handle *fh =
			(struct file_handle *) (uintptr_t) handle;

		printf("{handle_bytes=%u, handle_type=%d", fh->handle_bytes,
		       fh->handle_type);

		if (valid_data) {
			printf(", f_handle=");
			print_handle_data((unsigned char *) fh +
					  sizeof(struct file_handle),
					  fh->handle_bytes);
		}

		printf("}");
	} else {
		if (handle)
			printf("%#llx", (unsigned long long) handle);
		else
			printf("NULL");
	}
	printf(", %s) = ", flags_str);

	rc = syscall(__NR_open_by_handle_at, mount_fd, handle, flags);

	printf("%s\n", sprintrc(rc));
}

struct strval {
	kernel_ulong_t val;
	const char *str;
};

# define STR16 "0123456789abcdef"
# define STR64 STR16 STR16 STR16 STR16

int
main(void)
{
	enum {
		PATH1_SIZE = 64,
	};

	static const kernel_ulong_t fdcwd =
		(kernel_ulong_t) 0x87654321ffffff9cULL;
	static const struct strval dirfds[] = {
		{ (kernel_ulong_t) 0xdeadca57badda7a1ULL, "-1159878751" },
		{ (kernel_ulong_t) 0x12345678ffffff9cULL, "AT_FDCWD" },
	};
	static const struct strval name_flags[] = {
		{ (kernel_ulong_t) 0xdeadf15700000000ULL, "0" },
		{ (kernel_ulong_t) 0xbadc0ded00001000ULL,
			"AT_EMPTY_PATH" },
		{ (kernel_ulong_t) 0xdeadc0deda7a1457ULL,
			"AT_SYMLINK_FOLLOW|AT_EMPTY_PATH|0xda7a0057" },
		{ (kernel_ulong_t) 0xdefaced1ffffebffULL,
			"0xffffebff /* AT_??? */" },
	};
	static const kernel_ulong_t mount_fds[] = {
		(kernel_ulong_t) 0xdeadca5701234567ULL,
		(kernel_ulong_t) 0x12345678ffffff9cULL,
	};
	static const struct strval open_flags[] = {
		{ F8ILL_KULONG_MASK, "O_RDONLY" },
		{ (kernel_ulong_t) 0xdeadbeef80000001ULL,
			"O_WRONLY|0x80000000" }
	};

	static const char str64[] = STR64;


	char *bogus_path1 = tail_memdup(str64, PATH1_SIZE);
	char *bogus_path2 = tail_memdup(str64, sizeof(str64));

	struct file_handle *handle =
		tail_alloc(sizeof(struct file_handle) + MAX_HANDLE_SZ);
	struct file_handle *handle_0 =
		tail_alloc(sizeof(struct file_handle) + 0);
	struct file_handle *handle_8 =
		tail_alloc(sizeof(struct file_handle) + 8);
	struct file_handle *handle_128 =
		tail_alloc(sizeof(struct file_handle) + 128);
	struct file_handle *handle_256 =
		tail_alloc(sizeof(struct file_handle) + 256);
	TAIL_ALLOC_OBJECT_CONST_PTR(int, bogus_mount_id);

	char handle_0_addr[sizeof("0x") + sizeof(void *) * 2];

	char bogus_path1_addr[sizeof("0x") + sizeof(void *) * 2];
	char bogus_path1_after_addr[sizeof("0x") + sizeof(void *) * 2];


	struct strval paths[] = {
		{ (kernel_ulong_t) 0, "NULL" },
		{ (kernel_ulong_t) (uintptr_t) (bogus_path1 + PATH1_SIZE),
			bogus_path1_after_addr },
		{ (kernel_ulong_t) (uintptr_t) bogus_path1, bogus_path1_addr },
		{ (kernel_ulong_t) (uintptr_t) bogus_path2, "\"" STR64 "\"" },
	};
	struct strval name_handles[] = {
		{ (uintptr_t) (handle_0 + sizeof(struct file_handle)),
			handle_0_addr },
		{ (uintptr_t) handle_0,   "{handle_bytes=256}" },
		{ (uintptr_t) handle_8,   "{handle_bytes=0}" },
		{ (uintptr_t) handle_128, "{handle_bytes=128}" },
		{ (uintptr_t) handle_256, "{handle_bytes=256}" },
	};
	struct {
		kernel_ulong_t addr;
		bool valid;
		bool valid_data;
	} open_handles[] = {
		{ 0, false, false },
		{ (uintptr_t) (handle_0 + sizeof(struct file_handle)),
			false, false },
		{ (uintptr_t) handle_0 + 4, false, false },
		{ (uintptr_t) handle_0, true, false },
		{ (uintptr_t) handle_8, true, true },
		{ (uintptr_t) handle_128, true, true },
		{ (uintptr_t) handle_256, true, true },
	};
	kernel_ulong_t mount_ids[] = {
		0,
		(kernel_ulong_t) (uintptr_t) (bogus_mount_id + 1),
		(kernel_ulong_t) (uintptr_t) bogus_mount_id,
	};

	const int flags = 0x400;
	int mount_id;
	unsigned int i;
	unsigned int j;
	unsigned int k;
	unsigned int l;
	unsigned int m;


	snprintf(bogus_path1_addr, sizeof(bogus_path1_addr), "%p", bogus_path1);
	snprintf(bogus_path1_after_addr, sizeof(bogus_path1_after_addr), "%p",
		bogus_path1 + PATH1_SIZE);

	handle_0->handle_bytes = 256;
	handle_8->handle_bytes = 0;
	handle_128->handle_bytes = 128;
	handle_256->handle_bytes = 256;

	fill_memory((char *) handle_128 + sizeof(struct file_handle), 128);
	fill_memory((char *) handle_256 + sizeof(struct file_handle), 256);

	snprintf(handle_0_addr, sizeof(handle_0_addr), "%p",
		handle_0 + sizeof(struct file_handle));

	handle->handle_bytes = 0;

	assert(syscall(__NR_name_to_handle_at, fdcwd, ".", handle, &mount_id,
		flags | 1) == -1);
	if (EINVAL != errno)
		perror_msg_and_skip("name_to_handle_at");
	printf("name_to_handle_at(AT_FDCWD, \".\", {handle_bytes=0}, %p"
	       ", AT_SYMLINK_FOLLOW|0x1) = -1 EINVAL (%m)\n", &mount_id);

	assert(syscall(__NR_name_to_handle_at, fdcwd, ".", handle, &mount_id,
		flags) == -1);
	if (EOVERFLOW != errno)
		perror_msg_and_skip("name_to_handle_at");
	printf("name_to_handle_at(AT_FDCWD, \".\", {handle_bytes=0 => %u}"
	       ", %p, AT_SYMLINK_FOLLOW) = -1 EOVERFLOW (%m)\n",
	       handle->handle_bytes, &mount_id);

	assert(syscall(__NR_name_to_handle_at, fdcwd, ".", handle, &mount_id,
		flags) == 0);
	printf("name_to_handle_at(AT_FDCWD, \".\", {handle_bytes=%u"
	       ", handle_type=%d, f_handle=0x",
	       handle->handle_bytes, handle->handle_type);
	for (i = 0; i < handle->handle_bytes; ++i)
		printf("%02x", handle->f_handle[i]);
	printf("}, [%d], AT_SYMLINK_FOLLOW) = 0\n", mount_id);

	printf("open_by_handle_at(-1, {handle_bytes=%u, handle_type=%d"
	       ", f_handle=0x", handle->handle_bytes, handle->handle_type);
	for (i = 0; i < handle->handle_bytes; ++i)
		printf("%02x", handle->f_handle[i]);
	int rc = syscall(__NR_open_by_handle_at, -1, handle,
		O_RDONLY | O_DIRECTORY);
	printf("}, O_RDONLY|O_DIRECTORY) = %d %s (%m)\n", rc, errno2name());

	for (i = 0; i < ARRAY_SIZE(dirfds); i++) {
		for (j = 0; j < ARRAY_SIZE(paths); j++) {
			for (k = 0; k < ARRAY_SIZE(name_handles); k++) {
				for (l = 0; l < ARRAY_SIZE(mount_ids); l++) {
					for (m = 0; m < ARRAY_SIZE(name_flags);
					    m++) {
						do_name_to_handle_at(
							dirfds[i].val,
							dirfds[i].str,
							paths[j].val,
							paths[j].str,
							name_handles[k].val,
							name_handles[k].str,
							mount_ids[l],
							name_flags[m].val,
							name_flags[m].str,
							ASSERT_ERROR, 0);
					}
				}
			}
		}
	}

	for (i = 0; i < ARRAY_SIZE(mount_fds); i++) {
		for (j = 0; j < ARRAY_SIZE(open_handles); j++) {
			for (k = 0; k < ARRAY_SIZE(open_flags); k++) {
				do_open_by_handle_at(mount_fds[i],
						     open_handles[j].addr,
						     open_handles[j].valid,
						     open_handles[j].valid_data,
						     open_flags[k].val,
						     open_flags[k].str);
			}
		}
	}

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_name_to_handle_at && __NR_open_by_handle_at")

#endif
