/*
 * Check decoding of linkat syscall.
 *
 * Copyright (c) 2016-2022 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

#include "secontext.h"
#include "xmalloc.h"

static void
mangle_secontext_field(const char *path, enum secontext_field field,
		       const char *new_val, const char *fallback_val)
{
	char *orig = get_secontext_field_file(path, field);
	if (!orig)
		return;

	update_secontext_field(path, field,
			       strcmp(new_val, orig) ? new_val : fallback_val);

	free(orig);
}

int
main(void)
{
	/*
	 * Make sure the current workdir of the tracee
	 * is different from the current workdir of the tracer.
	 */
	create_and_enter_subdir("linkat_subdir");

	static const char sample_1[] = "linkat_sample_old";
	static const char sample_2[] = "linkat_sample_new";
	const long fd_old = (long) 0xdeadbeefffffffffULL;
	const long fd_new = (long) 0xdeadbeeffffffffeULL;

	char *my_secontext = SECONTEXT_PID_MY();

	(void) unlink(sample_1);
	(void) unlink(sample_2);

	long rc = syscall(__NR_linkat, fd_old, sample_1, fd_new, sample_2, 0);
	printf("%s%s(%d, \"%s\", %d, \"%s\", 0) = %ld %s (%m)\n",
	       my_secontext, "linkat",
	       (int) fd_old, sample_1, (int) fd_new, sample_2,
	       rc, errno2name());

	rc = syscall(__NR_linkat, -100, sample_1, -100, sample_2, -1L);
	printf("%s%s(%s, \"%s\", %s, \"%s\", %s) = %ld %s (%m)\n",
	       my_secontext, "linkat",
	       "AT_FDCWD", sample_1, "AT_FDCWD", sample_2,
	       "AT_SYMLINK_NOFOLLOW|AT_REMOVEDIR|AT_SYMLINK_FOLLOW"
	       "|AT_NO_AUTOMOUNT|AT_EMPTY_PATH|AT_RECURSIVE|0xffff60ff",
	       rc, errno2name());

	/*
	 * Tests with AT_FDCWD.
	 */

	int fd_sample_1 = open(sample_1, O_RDONLY | O_CREAT, 0400);
	if (fd_sample_1 < 0)
		perror_msg_and_fail("open");
	if (close(fd_sample_1))
		perror_msg_and_fail("close");

	char *sample_1_secontext = SECONTEXT_FILE(sample_1);

	rc = syscall(__NR_linkat, -100, sample_1, -100, sample_2, 0);
	/* no context printed for sample_2 since file doesn't exist yet */
	printf("%s%s(AT_FDCWD, \"%s\"%s, AT_FDCWD, \"%s\", 0) = %s\n",
	       my_secontext, "linkat",
	       sample_1, sample_1_secontext,
	       sample_2,
	       sprintrc(rc));

	const char *sample_2_secontext = sample_1_secontext;

	rc = syscall(__NR_linkat, -100, sample_1, -100, sample_2, 0);
	printf("%s%s(AT_FDCWD, \"%s\"%s, AT_FDCWD, \"%s\"%s, 0) = %s\n",
	       my_secontext, "linkat",
	       sample_1, sample_1_secontext,
	       sample_2, sample_2_secontext,
	       sprintrc(rc));

	int fd_sample_2 = open(sample_2, O_RDONLY | O_CREAT, 0400);
	if (fd_sample_2 < 0)
		perror_msg_and_fail("open");
	if (close(fd_sample_2))
		perror_msg_and_fail("close");

	if (*sample_1_secontext && strstr(sample_1_secontext, "!!"))
		reset_secontext_file(sample_1);

	free(sample_1_secontext);

#ifdef PRINT_SECONTEXT_MISMATCH
	errno = 0;
	mangle_secontext_field(sample_1, SECONTEXT_USER, "system_u",
							 "unconfined_u");
	sample_1_secontext = SECONTEXT_FILE(sample_1);

# ifdef PRINT_SECONTEXT_FULL
	/* The mismatch should be detected */
	if (*sample_1_secontext && strstr(sample_1_secontext, "!!") == NULL)
		perror_msg_and_fail("Context mismatch not detected: %s",
				    sample_1_secontext);
	if (*sample_1_secontext && strstr(sample_1_secontext, "system_u") == NULL)
		perror_msg_and_fail("Context mismatch not detected: %s",
				    sample_1_secontext);
# else
	/* The mismatch cannot be detected since it's on user part */
	if (*sample_1_secontext && strstr(sample_1_secontext, "!!") != NULL)
		perror_msg_and_fail("Context mismatch detected: %s",
				    sample_1_secontext);
# endif

	free(sample_1_secontext);
#endif

	errno = 0;
	mangle_secontext_field(sample_1, SECONTEXT_TYPE, "default_t",
							 "unconfined_t");
	sample_1_secontext = SECONTEXT_FILE(sample_1);
	sample_2_secontext = sample_1_secontext;

#ifdef PRINT_SECONTEXT_MISMATCH
	if (*sample_1_secontext && strstr(sample_1_secontext, "!!") == NULL)
		perror_msg_and_fail("Context mismatch not detected: %s",
				    sample_1_secontext);
	if (*sample_1_secontext && strstr(sample_1_secontext, "default_t") == NULL)
		perror_msg_and_fail("Context mismatch not detected: %s",
				    sample_1_secontext);
#endif

	rc = syscall(__NR_linkat, -100, sample_1, -100, sample_2, 0);
	printf("%s%s(AT_FDCWD, \"%s\"%s, AT_FDCWD, \"%s\"%s, 0) = %s\n",
	       my_secontext, "linkat",
	       sample_1, sample_1_secontext,
	       sample_2, sample_2_secontext,
	       sprintrc(rc));

	if (unlink(sample_2))
		perror_msg_and_fail("unlink: %s", sample_2);

	/*
	 * Tests with dirfd.
	 */

	int dfd_old = get_dir_fd(".");
	char *cwd = get_fd_path(dfd_old);

	errno = 0;
	mangle_secontext_field(".", SECONTEXT_TYPE, "default_t",
						    "unconfined_t");
	char *dfd_old_secontext = SECONTEXT_FILE(".");

#ifdef PRINT_SECONTEXT_MISMATCH
	if (*dfd_old_secontext && strstr(dfd_old_secontext, "!!") == NULL)
		perror_msg_and_fail("Context mismatch not detected: %s",
				    dfd_old_secontext);
	if (*dfd_old_secontext && strstr(dfd_old_secontext, "default_t") == NULL)
		perror_msg_and_fail("Context mismatch not detected: %s",
				    dfd_old_secontext);
#endif

	rc = syscall(__NR_linkat, dfd_old, sample_1, -100, sample_2, 0);
	/* no context printed for sample_2 since file doesn't exist yet */
	printf("%s%s(%d%s, \"%s\"%s, AT_FDCWD, \"%s\", 0) = %s\n",
	       my_secontext, "linkat",
	       dfd_old, dfd_old_secontext,
	       sample_1, sample_1_secontext,
	       sample_2,
	       sprintrc(rc));

	rc = syscall(__NR_linkat, dfd_old, sample_1, -100, sample_2, 0);
	printf("%s%s(%d%s, \"%s\"%s, AT_FDCWD, \"%s\"%s, 0) = %s\n",
	       my_secontext, "linkat",
	       dfd_old, dfd_old_secontext,
	       sample_1, sample_1_secontext,
	       sample_2, sample_2_secontext,
	       sprintrc(rc));

	if (unlink(sample_2))
		perror_msg_and_fail("unlink: %s", sample_2);

	static const char new_dir[] = "new";
	char *new_sample_2 = xasprintf("%s/%s", new_dir, sample_2);

	(void) unlink(new_sample_2);
	(void) rmdir(new_dir);

	if (mkdir(new_dir, 0700))
		perror_msg_and_fail("mkdir");
	char *new_dir_realpath = xasprintf("%s/%s", cwd, new_dir);
	char *new_dir_secontext = SECONTEXT_FILE(new_dir);
	int dfd_new = get_dir_fd(new_dir);

	rc = syscall(__NR_linkat, dfd_old, sample_1, dfd_new, sample_2, 0);
	/* no context printed for sample_2 since file doesn't exist yet */
	printf("%s%s(%d%s, \"%s\"%s, %d%s, \"%s\", 0) = %s\n",
	       my_secontext, "linkat",
	       dfd_old, dfd_old_secontext,
	       sample_1, sample_1_secontext,
	       dfd_new, new_dir_secontext,
	       sample_2,
	       sprintrc(rc));

	rc = syscall(__NR_linkat, dfd_old, sample_1, dfd_new, sample_2, 0);
	printf("%s%s(%d%s, \"%s\"%s, %d%s, \"%s\"%s, 0) = %s\n",
	       my_secontext, "linkat",
	       dfd_old, dfd_old_secontext,
	       sample_1, sample_1_secontext,
	       dfd_new, new_dir_secontext,
	       sample_2, SECONTEXT_FILE(new_sample_2),
	       sprintrc(rc));

	char *new_sample_2_realpath = xasprintf("%s/%s", new_dir_realpath, sample_2);

	/* dfd ignored when path is absolute */
	if (chdir("../.."))
		perror_msg_and_fail("chdir");

	rc = syscall(__NR_linkat, dfd_old, sample_1, -100, new_sample_2_realpath, 0);
	printf("%s%s(%d%s, \"%s\"%s, AT_FDCWD, \"%s\"%s, 0) = %s\n",
	       my_secontext, "linkat",
	       dfd_old, dfd_old_secontext,
	       sample_1, sample_1_secontext,
	       new_sample_2_realpath, SECONTEXT_FILE(new_sample_2_realpath),
	       sprintrc(rc));

	if (fchdir(dfd_old))
		perror_msg_and_fail("fchdir");

	if (unlink(sample_1))
		perror_msg_and_fail("unlink: %s", sample_1);
	if (unlink(new_sample_2))
		perror_msg_and_fail("unlink: %s", new_sample_2);
	if (rmdir(new_dir))
		perror_msg_and_fail("rmdir: %s", new_dir);

	leave_and_remove_subdir();

	puts("+++ exited with 0 +++");
	return 0;
}
