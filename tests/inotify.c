/*
 * Check decoding of inotify_add_watch and inotify_rm_watch syscalls.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
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

#if defined(__NR_inotify_add_watch) && defined(__NR_inotify_rm_watch)

# include <stdio.h>
# include <string.h>
# include <unistd.h>

int
main(void)
{
	static const struct {
		const char *path;
		const char *str;
	} bogus_path_str = {
		ARG_STR("/abc\1/def\2/ghi\3/jkl\4/mno\5/pqr\6/stu\7/vwx\10") };
	static const kernel_ulong_t bogus_fd =
		(kernel_ulong_t) 0xfffffeedfffffaceULL;
	static const kernel_ulong_t bogus_mask =
		(kernel_ulong_t) 0xffffda7affffdeadULL;
	static const char *bogus_mask_str = "IN_ACCESS|IN_ATTRIB|"
		"IN_CLOSE_WRITE|IN_OPEN|IN_MOVED_TO|IN_DELETE|IN_DELETE_SELF|"
		"IN_MOVE_SELF|IN_Q_OVERFLOW|IN_IGNORED|IN_ONLYDIR|"
		"IN_DONT_FOLLOW|IN_EXCL_UNLINK|IN_MASK_CREATE|IN_MASK_ADD|"
		"IN_ISDIR|IN_ONESHOT|0x8ff1000";

	long rc;
	char *bogus_path = tail_memdup(bogus_path_str.path,
		strlen(bogus_path_str.path) + 1);

	rc = syscall(__NR_inotify_add_watch, 0, NULL, 0);
	printf("inotify_add_watch(0, NULL, 0) = %s\n", sprintrc(rc));

	rc = syscall(__NR_inotify_add_watch, bogus_fd, bogus_path + 4096, 0);
	printf("inotify_add_watch(%d, %p, %u) = %s\n",
	       (int) bogus_fd, bogus_path + 4096, 0, sprintrc(rc));

	rc = syscall(__NR_inotify_add_watch, bogus_fd, bogus_path, bogus_mask);
	printf("inotify_add_watch(%d, %s, %s) = %s\n",
	       (int) bogus_fd, bogus_path_str.str, bogus_mask_str,
	       sprintrc(rc));

	rc = syscall(__NR_inotify_rm_watch, 0, 0);
	printf("inotify_rm_watch(0, 0) = %s\n", sprintrc(rc));

	rc = syscall(__NR_inotify_rm_watch, bogus_fd, bogus_fd);
	printf("inotify_rm_watch(%d, %d) = %s\n",
	       (int) bogus_fd, (int) bogus_fd, sprintrc(rc));

	puts("+++ exited with 0 +++");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_inotify_add_watch && __NR_inotify_rm_watch");

#endif
