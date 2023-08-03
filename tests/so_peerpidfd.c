/*
 * Check decoding of SO_PEERPIDFD socket option.
 *
 * Copyright (c) 2023 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>

#define XLAT_MACROS_ONLY
# include "xlat/sock_options.h"
#undef XLAT_MACROS_ONLY

static const char *errstr;
static int rc;

static int
get_peerpidfd(int fd, void *val, socklen_t *len)
{
	rc = getsockopt(fd, SOL_SOCKET, SO_PEERPIDFD, val, len);
	errstr = sprintrc(rc);
	printf("getsockopt(%d<socket:[%lu]>, SOL_SOCKET, SO_PEERPIDFD, ",
	       fd, inode_of_sockfd(fd));
	return rc;
}

static void
print_pidfd(int *p)
{
	static const char pidfd_suffix[] = "<anon_inode:[pidfd]>";

	if (rc < 0)
		printf("%p", p);
	else
		printf("%d%s", *p, pidfd_suffix);
}

static void
print_optlen(int oldval, int newval)
{
	printf(", [%d", oldval);
	if (oldval != newval)
		printf(" => %d", newval);
	printf("]) = %s\n", errstr);
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	TAIL_ALLOC_OBJECT_CONST_PTR(int, val);
	TAIL_ALLOC_OBJECT_CONST_ARR(int, bigval, 2);
	TAIL_ALLOC_OBJECT_CONST_PTR(socklen_t, len);
	void *const efault = val + 1;

	int sv[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv))
                perror_msg_and_skip("socketpair AF_UNIX SOCK_STREAM");
	int fd = sv[0];

	/* classic */
	*len = sizeof(*val);
	rc = get_peerpidfd(fd, val, len);
	print_pidfd(val);
	print_optlen(sizeof(*val), *len);

	/* optlen larger than accessible memory */
	*len = sizeof(*val) + 1;
	get_peerpidfd(fd, val, len);
	print_pidfd(val);
	print_optlen(sizeof(*val) + 1, *len);

	/* optlen larger than necessary */
	*len = sizeof(*bigval);
	get_peerpidfd(fd, bigval, len);
	print_pidfd(bigval);
	print_optlen(sizeof(*bigval), *len);

	/* zero optlen - print returned optlen */
	*len = 0;
	get_peerpidfd(fd, NULL, len);
	printf("NULL");
	print_optlen(0, *len);

	/* optlen shorter than necessary */
	*len = sizeof(*val) - 1;
	get_peerpidfd(fd, val, len);
	if (rc < 0)
		printf("%p", val);
	else
		print_quoted_hex(val, sizeof(*val) - 1);
	print_optlen(sizeof(*val) - 1, *len);

	/* optval EFAULT - print address */
	*len = sizeof(*val);
	get_peerpidfd(fd, efault, len);
	printf("%p", efault);
	print_optlen(sizeof(*val), *len);

	/* optlen EFAULT - print address */
	get_peerpidfd(fd, val, len + 1);
	printf("%p, %p) = %s\n", val, len + 1, errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
