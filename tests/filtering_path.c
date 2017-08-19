/*
 * Check decoding of non-standard path filters
 *
 * Copyright (c) 2017 Nikolay Marchuk <marchuk.nikolay.a@gmail.com>
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
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#define FD_TRACED_DEFAULT 5

#ifdef __NR_dup2
void
test_dup2(int fd_base)
{
	int rc = syscall(__NR_dup2, fd_base, -1);
	printf("dup2(%d, -1) = %s\n", fd_base, sprintrc(rc));
	syscall(__NR_dup2, fd_base + 1, -1);

	rc = syscall(__NR_dup2, -1, fd_base);
	printf("dup2(-1, %d) = %s\n", fd_base, sprintrc(rc));
	syscall(__NR_dup2, -1, fd_base + 1);
}
#else
void
test_dup2(int fd_base)
{
}
#endif

#ifdef __NR_linkat
void
test_linkat(const char *path, int fd_base)
{
	int rc = syscall(__NR_linkat, fd_base, "old", -1, "new", 0);
	printf("linkat(%d, \"old\", -1, \"new\", 0) = %s\n",
	       fd_base, sprintrc(rc));
	rc = syscall(__NR_linkat, -1, "old", fd_base, "new", 0);
	printf("linkat(-1, \"old\", %d, \"new\", 0) = %s\n",
	       fd_base, sprintrc(rc));
	syscall(__NR_linkat, fd_base + 1, "old", fd_base + 1, "new", 0);

	rc = syscall(__NR_linkat, -1, path, -1, "new", 0);
	printf("linkat(-1, \"%s\", -1, \"new\", 0) = %s\n", path, sprintrc(rc));
	rc = syscall(__NR_linkat, -1, "old", -1, path, 0);
	printf("linkat(-1, \"old\", -1, \"%s\", 0) = %s\n", path, sprintrc(rc));

	syscall(__NR_linkat, -1, "old", -1, "new", 0);
}
#else
void
test_linkat(const char *path, int fd_base)
{
}
#endif

#ifdef __NR_symlinkat
void
test_symlinkat(const char *path, int fd_base)
{
	int rc = syscall(__NR_symlinkat, "new", fd_base, "old");
	printf("symlinkat(\"new\", %d, \"old\") = %s\n",
	       fd_base, sprintrc(rc));
	syscall(__NR_symlinkat, "new", fd_base + 1, "old");

	rc = syscall(__NR_symlinkat, "new", -1, path);
	printf("symlinkat(\"new\", -1, \"%s\") = %s\n", path, sprintrc(rc));
	syscall(__NR_symlinkat, "new", -1, "old");

	syscall(__NR_symlinkat, path, -1, "old");
}
#else
void
test_symlinkat(const char *path, int fd_base)
{
}
#endif

#ifdef __NR_epoll_ctl
# include <sys/epoll.h>
void
test_epoll(int fd_base)
{
	int rc = syscall(__NR_epoll_ctl, -1, EPOLL_CTL_ADD, fd_base, NULL);
	printf("epoll_ctl(-1, EPOLL_CTL_ADD, %d, NULL) = %s\n",
	       fd_base, sprintrc(rc));
	syscall(__NR_epoll_ctl, -1, EPOLL_CTL_ADD, fd_base + 1, NULL);
}
#else
void
test_epoll(int fd_base)
{
}
#endif

#if defined HAVE_SYS_FANOTIFY_H && defined HAVE_FANOTIFY_MARK && \
	defined __NR_fanotify_mark
# include <sys/fanotify.h>
void
test_fanotify_mark(const char *path, int fd_base)
{
	int rc = fanotify_mark(-1, 0, 0, fd_base, ".");
	printf("fanotify_mark(-1, 0, 0, %d, \".\") = %s\n",
	       fd_base, sprintrc(rc));
	fanotify_mark(-1, 0, 0, fd_base + 1, ".");

	rc = fanotify_mark(-1, 0, 0, -1, path);
	printf("fanotify_mark(-1, 0, 0, FAN_NOFD, \"%s\") = %s\n",
	       path, sprintrc(rc));
	fanotify_mark(-1, 0, 0, -1, "not_file");
}
#else
void
test_fanotify_mark(const char *path, int fd_base)
{
}
#endif

#if defined __NR_select || defined __NR__newselect
# include <sys/select.h>
void
test_select(int fd_base)
{
	fd_set positive_set, negative_set;

	FD_ZERO(&positive_set);
	FD_SET(fd_base, &positive_set);
	FD_ZERO(&negative_set);
	FD_SET(fd_base + 1, &negative_set);
# ifndef __NR__newselect
	syscall(__NR_select, fd_base + 1, &positive_set, NULL, NULL, NULL);
	printf("select(%d, [%d], NULL, NULL, NULL) = 1 (in [%d])\n",
	       fd_base + 1, fd_base, fd_base);
	syscall(__NR_select, fd_base + 2, &negative_set, NULL, NULL,NULL);
# else
	syscall(__NR__newselect, fd_base + 1, &positive_set, NULL, NULL,
		     NULL);
	printf("_newselect(%d, [%d], NULL, NULL, NULL) = 1 (in [%d])\n",
	       fd_base + 1, fd_base, fd_base);
	syscall(__NR__newselect, fd_base + 2, &negative_set, NULL, NULL, NULL);
# endif
}
#else
void
test_select(int fd_base)
{
}
#endif

#ifdef __NR_poll
# include <poll.h>
void
test_poll(int fd_base)
{
	struct pollfd positive_fds = {.fd = fd_base, .events = POLLIN};
	struct pollfd negative_fds = {.fd = fd_base + 1, .events = POLLIN};

	syscall(__NR_poll, &positive_fds, 1, 1);
	printf("poll([{fd=%d, events=POLLIN}], 1, 1) = 1 "
	       "([{fd=%d, revents=POLLIN}])\n", fd_base, fd_base);
	syscall(__NR_poll, &negative_fds, 1, 1);
}
#else
void
test_poll(int fd_base)
{
}
#endif

#ifdef __NR_faccessat
void
test_faccessat(const char *path, int fd_base)
{
	int rc = syscall(__NR_faccessat, -1, path, 0, 0);
	printf("faccessat(-1, \"%s\", F_OK) = %s\n", path, sprintrc(rc));
	syscall(__NR_faccessat, -1, "not_file", 0, 0);

	rc = syscall(__NR_faccessat, fd_base, "not_file", 0, 0);
	printf("faccessat(%d, \"not_file\", F_OK) = %s\n", fd_base, sprintrc(rc));
	syscall(__NR_faccessat, fd_base + 1, "not_file", 0, 0);
}
#else
void
test_faccessat(const char *path, int fd_base)
{
}
#endif

#ifdef __NR_link
void
test_link(const char *path, int fd_base)
{
	int rc;

	rc = syscall(__NR_link, path, NULL);
	printf("link(\"%s\", NULL) = %s\n",
	       path, sprintrc(rc));

	rc = syscall(__NR_link, NULL, path);
	printf("link(NULL, \"%s\") = %s\n",
	       path, sprintrc(rc));

	syscall(__NR_link, "old", "new");
}
#else
void
test_link(const char *path, int fd_base)
{
}
#endif

static const char *path = "path_trace test.sample";

#ifdef __NR_open
int
main(int argc, char **argv)
{
	const char *const name = argc > 1 ? argv[1] : "mmap";
	long tmp_fd_base = argc > 2 ? strtol(argv[2], NULL, 10) :
			   FD_TRACED_DEFAULT;
	int fd_base = (tmp_fd_base > 0 && tmp_fd_base < 1023) ?
		      (int) tmp_fd_base : FD_TRACED_DEFAULT;
	int tmp_fd;

	assert((tmp_fd = syscall(__NR_open, path, O_RDONLY | O_CREAT, 0600)) >= 0);
	printf("open(\"%s\", O_RDONLY|O_CREAT, 0600) = %d\n", path, tmp_fd);
	assert(dup2(tmp_fd, fd_base) == fd_base);
	printf("dup2(%d, %d) = %d\n", tmp_fd, fd_base, fd_base);
	assert(close(tmp_fd) == 0);
	printf("close(%d) = 0\n", tmp_fd);


	test_dup2(fd_base);
	test_linkat(path, fd_base);
	test_symlinkat(path, fd_base);
	test_epoll(fd_base);
	test_fanotify_mark(path, fd_base);
	test_select(fd_base);
	test_poll(fd_base);
	mmap(NULL, 0, PROT_NONE, MAP_FILE, fd_base, 0);
	printf("%s(NULL, 0, PROT_NONE, MAP_FILE, %d, 0) = -1 "
	       "EINVAL (Invalid argument)\n", name, fd_base);

	test_faccessat(path, fd_base);
	test_link(path, fd_base);
	test_linkat(path, fd_base);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_open")

#endif
