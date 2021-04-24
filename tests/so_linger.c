/*
 * Check decoding of SO_LINGER socket option.
 *
 * Copyright (c) 2017-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static const char *errstr;

static int
get_linger(int fd, void *val, socklen_t *len)
{
	int rc = getsockopt(fd, SOL_SOCKET, SO_LINGER, val, len);
	errstr = sprintrc(rc);
	return rc;
}

static int
set_linger(int fd, void *val, socklen_t len)
{
	int rc = setsockopt(fd, SOL_SOCKET, SO_LINGER, val, len);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(struct linger, linger);
	TAIL_ALLOC_OBJECT_CONST_PTR(socklen_t, len);

	const unsigned int sizeof_l_onoff = sizeof(linger->l_onoff);
	struct linger *const l_onoff = tail_alloc(sizeof_l_onoff);

	const unsigned int sizeof_l_onoff_truncated = sizeof_l_onoff - 1;
	struct linger *const l_onoff_truncated =
		tail_alloc(sizeof_l_onoff_truncated);

	const unsigned int sizeof_l_linger_truncated =
		offsetofend(struct linger, l_linger) - 1;
	struct linger *const l_linger_truncated =
		tail_alloc(sizeof_l_linger_truncated);

        int fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (fd < 0)
                perror_msg_and_skip("socket AF_UNIX SOCK_STREAM");

	/* classic getsockopt */
	*len = sizeof(*linger);
	get_linger(fd, linger, len);
	printf("getsockopt(%d, SOL_SOCKET, SO_LINGER, {l_onoff=%d, l_linger=%d}"
	       ", [%d]) = %s\n",
	       fd, linger->l_onoff, linger->l_linger, *len, errstr);

	/* classic setsockopt */
	linger->l_onoff = -15;
	linger->l_linger = -42;
	set_linger(fd, linger, sizeof(*linger));
	printf("setsockopt(%d, SOL_SOCKET, SO_LINGER, {l_onoff=%d, l_linger=%d}"
	       ", %d) = %s\n",
	       fd, linger->l_onoff, linger->l_linger,
	       (unsigned int) sizeof(*linger), errstr);

	/* setsockopt with optlen larger than necessary */
	set_linger(fd, linger, sizeof(*linger) + 1);
	printf("setsockopt(%d, SOL_SOCKET, SO_LINGER, {l_onoff=%d, l_linger=%d}"
	       ", %d) = %s\n",
	       fd, linger->l_onoff, linger->l_linger,
	       (unsigned int) sizeof(*linger) + 1, errstr);

	/* setsockopt with optlen < 0 - EINVAL */
	set_linger(fd, linger, -1U);
	printf("setsockopt(%d, SOL_SOCKET, SO_LINGER, %p, -1) = %s\n",
	       fd, linger, errstr);

	/* setsockopt with optlen smaller than necessary - EINVAL */
	set_linger(fd, linger, sizeof(linger->l_onoff));
	printf("setsockopt(%d, SOL_SOCKET, SO_LINGER, %p, %d) = %s\n",
	       fd, linger, (unsigned int) sizeof(linger->l_onoff), errstr);

	/* setsockopt optval EFAULT */
	set_linger(fd, &linger->l_linger, sizeof(*linger));
	printf("setsockopt(%d, SOL_SOCKET, SO_LINGER, %p, %d) = %s\n",
	       fd, &linger->l_linger, (unsigned int) sizeof(*linger), errstr);

	/* getsockopt with zero optlen */
	*len = 0;
	get_linger(fd, linger, len);
	printf("getsockopt(%d, SOL_SOCKET, SO_LINGER, %p, [0]) = %s\n",
	       fd, linger, errstr);

	/* getsockopt with optlen larger than necessary - shortened */
	*len = sizeof(*linger) + 1;
	get_linger(fd, linger, len);
	printf("getsockopt(%d, SOL_SOCKET, SO_LINGER, {l_onoff=%d, l_linger=%d}"
	       ", [%u => %d]) = %s\n",
	       fd, linger->l_onoff, linger->l_linger,
	       (unsigned int) sizeof(*linger) + 1, *len, errstr);

	/*
	 * getsockopt with optlen less than sizeof(linger->l_onoff):
	 * the part of struct linger.l_onoff is printed in hex.
	 */
	*len = sizeof_l_onoff_truncated;
	get_linger(fd, l_onoff_truncated, len);
	printf("getsockopt(%d, SOL_SOCKET, SO_LINGER, {l_onoff=", fd);
	print_quoted_hex(l_onoff_truncated, *len);
	printf("}, [%d]) = %s\n", *len, errstr);

	/*
	 * getsockopt with optlen equals to sizeof(struct linger.l_onoff):
	 * struct linger.l_linger is not printed.
	 */
	*len = sizeof_l_onoff;
	get_linger(fd, l_onoff, len);
	printf("getsockopt(%d, SOL_SOCKET, SO_LINGER, {l_onoff=%d}"
	       ", [%d]) = %s\n",
	       fd, l_onoff->l_onoff, *len, errstr);

	/*
	 * getsockopt with optlen greater than sizeof(struct linger.l_onoff)
	 * but smaller than sizeof(struct linger):
	 * the part of struct linger.l_linger is printed in hex.
	 */
	*len = sizeof_l_linger_truncated;
	get_linger(fd, l_linger_truncated, len);
	/*
	 * Copy to a properly aligned structure to avoid unaligned access
	 * to struct linger.l_onoff field.
	 */
	memcpy(linger, l_linger_truncated, sizeof_l_linger_truncated);
	printf("getsockopt(%d, SOL_SOCKET, SO_LINGER, {l_onoff=%d, l_linger=",
	       fd, linger->l_onoff);
	print_quoted_hex(&linger->l_linger, sizeof_l_linger_truncated -
					    offsetof(struct linger, l_linger));
	printf("}, [%d]) = %s\n", *len, errstr);

	/* getsockopt optval EFAULT */
	*len = sizeof(*linger);
	get_linger(fd, &linger->l_linger, len);
	printf("getsockopt(%d, SOL_SOCKET, SO_LINGER, %p, [%d]) = %s\n",
	       fd, &linger->l_linger, *len, errstr);

	/* getsockopt optlen EFAULT */
	get_linger(fd, linger, len + 1);
	printf("getsockopt(%d, SOL_SOCKET, SO_LINGER, %p, %p) = %s\n",
	       fd, linger, len + 1, errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
