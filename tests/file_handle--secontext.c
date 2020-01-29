/*
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

/*
 * This test is designed to be executed with the following strace options:
 * --secontext[=full] -y
 */

#if defined __NR_name_to_handle_at && defined __NR_open_by_handle_at && defined HAVE_SELINUX_RUNTIME

# include <assert.h>
# include <errno.h>
# include <inttypes.h>
# include <fcntl.h>
# include <stdio.h>
# include <unistd.h>
# include <string.h>

/* for getcwd()/opendir() */
# include <limits.h>
# include <sys/types.h>
# include <dirent.h>

# include "selinux.c"

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
	unsigned int len = MIN(size, MAX_HANDLE_SZ);
	print_quoted_hex(bytes, len);
	if (size > len)
		printf("...");
}

int
main(void)
{
	static const kernel_ulong_t fdcwd =
		(kernel_ulong_t) 0x87654321ffffff9cULL;
	struct file_handle *handle =
		tail_alloc(sizeof(struct file_handle) + MAX_HANDLE_SZ);
	const int flags = 0x400;
	int mount_id;

	handle->handle_bytes = 0;

	assert(syscall(__NR_name_to_handle_at, fdcwd, ".", handle, &mount_id,
		flags | 1) == -1);
	if (EINVAL != errno)
		perror_msg_and_skip("name_to_handle_at");
	printf("%sname_to_handle_at(AT_FDCWD, \".\"%s, {handle_bytes=0}, %p"
	       ", AT_SYMLINK_FOLLOW|0x1) = -1 EINVAL (%m)\n",
	       SELINUX_MYCONTEXT(), SELINUX_FILECONTEXT("."),
	       &mount_id);

	assert(syscall(__NR_name_to_handle_at, fdcwd, ".", handle, &mount_id,
		flags) == -1);
	if (EOVERFLOW != errno)
		perror_msg_and_skip("name_to_handle_at");
	printf("%sname_to_handle_at(AT_FDCWD, \".\"%s, {handle_bytes=0 => %u}"
	       ", %p, AT_SYMLINK_FOLLOW) = -1 EOVERFLOW (%m)\n",
	       SELINUX_MYCONTEXT(), SELINUX_FILECONTEXT("."),
	       handle->handle_bytes, &mount_id);

	assert(syscall(__NR_name_to_handle_at, fdcwd, ".", handle, &mount_id,
		flags) == 0);
	printf("%sname_to_handle_at(AT_FDCWD, \".\"%s, {handle_bytes=%u"
	       ", handle_type=%d, f_handle=",
	       SELINUX_MYCONTEXT(), SELINUX_FILECONTEXT("."),
	       handle->handle_bytes, handle->handle_type);
	print_handle_data((unsigned char *) handle +
			  sizeof(struct file_handle),
			  handle->handle_bytes);
	printf("}, [%d], AT_SYMLINK_FOLLOW) = 0\n", mount_id);

	printf("%sopen_by_handle_at(-1, {handle_bytes=%u, handle_type=%d"
	       ", f_handle=",
	       SELINUX_MYCONTEXT(),
	       handle->handle_bytes, handle->handle_type);
	print_handle_data((unsigned char *) handle +
			  sizeof(struct file_handle),
			  handle->handle_bytes);
	int rc = syscall(__NR_open_by_handle_at, -1, handle,
		O_RDONLY | O_DIRECTORY);
	printf("}, O_RDONLY|O_DIRECTORY) = %s\n", sprintrc(rc));

	/*
	 * Tests with dirfd
	 */

	char cwd[PATH_MAX + 1];
	DIR *dir = NULL;
	if (getcwd(cwd, sizeof (cwd)) == NULL)
		perror_msg_and_fail("getcwd");
	dir = opendir(cwd);
	if (dir == NULL)
		perror_msg_and_fail("opendir");
	int dfd = dirfd(dir);
	if (dfd == -1)
		perror_msg_and_fail("dirfd");

	assert(syscall(__NR_name_to_handle_at, dfd, ".", handle, &mount_id,
		flags) == 0);
	printf("%sname_to_handle_at(%d<%s>%s, \".\"%s, {handle_bytes=%u"
	       ", handle_type=%d, f_handle=",
	       SELINUX_MYCONTEXT(),
	       dfd, cwd, SELINUX_FILECONTEXT(cwd),
	       SELINUX_FILECONTEXT("."),
	       handle->handle_bytes, handle->handle_type);
	print_handle_data((unsigned char *) handle +
			  sizeof(struct file_handle),
			  handle->handle_bytes);
	printf("}, [%d], AT_SYMLINK_FOLLOW) = 0\n", mount_id);

	printf("%sopen_by_handle_at(-1, {handle_bytes=%u, handle_type=%d"
	       ", f_handle=",
	       SELINUX_MYCONTEXT(),
	       handle->handle_bytes, handle->handle_type);
	print_handle_data((unsigned char *) handle +
			  sizeof(struct file_handle),
			  handle->handle_bytes);
	rc = syscall(__NR_open_by_handle_at, -1, handle,
		O_RDONLY | O_DIRECTORY);
	printf("}, O_RDONLY|O_DIRECTORY) = %s\n", sprintrc(rc));

	/* dfd ignored when path is absolute */
	if (chdir("..") == -1)
		perror_msg_and_fail("chdir");

	assert(syscall(__NR_name_to_handle_at, dfd, cwd, handle, &mount_id,
		flags) == 0);
	printf("%sname_to_handle_at(%d<%s>%s, \"%s\"%s, {handle_bytes=%u"
	       ", handle_type=%d, f_handle=",
	       SELINUX_MYCONTEXT(),
	       dfd, cwd, SELINUX_FILECONTEXT(cwd),
	       cwd, SELINUX_FILECONTEXT(cwd),
	       handle->handle_bytes, handle->handle_type);
	print_handle_data((unsigned char *) handle +
			  sizeof(struct file_handle),
			  handle->handle_bytes);
	printf("}, [%d], AT_SYMLINK_FOLLOW) = 0\n", mount_id);

	printf("%sopen_by_handle_at(-1, {handle_bytes=%u, handle_type=%d"
	       ", f_handle=",
	       SELINUX_MYCONTEXT(),
	       handle->handle_bytes, handle->handle_type);
	print_handle_data((unsigned char *) handle +
			  sizeof(struct file_handle),
			  handle->handle_bytes);
	rc = syscall(__NR_open_by_handle_at, -1, handle,
		O_RDONLY | O_DIRECTORY);
	printf("}, O_RDONLY|O_DIRECTORY) = %s\n", sprintrc(rc));

	if (chdir(cwd) == -1)
		perror_msg_and_fail("chdir");

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_name_to_handle_at && __NR_open_by_handle_at && HAVE_SELINUX_RUNTIME")

#endif
