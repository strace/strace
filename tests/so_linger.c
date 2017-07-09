/*
 * Check decoding of SO_LINGER socket option.
 *
 * Copyright (c) 2017 Dmitry V. Levin <ldv@altlinux.org>
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

#include <stdio.h>
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

	/* getsockopt with optlen larger than necessary - shortened */
	*len = sizeof(*linger) + 1;
	get_linger(fd, linger, len);
	printf("getsockopt(%d, SOL_SOCKET, SO_LINGER, {l_onoff=%d, l_linger=%d}"
	       ", [%u->%d]) = %s\n",
	       fd, linger->l_onoff, linger->l_linger,
	       (unsigned int) sizeof(*linger) + 1, *len, errstr);

	/* getsockopt with optlen larger than usual - truncated to l_onoff */
	*len = sizeof(linger->l_onoff);
	get_linger(fd, linger, len);
	printf("getsockopt(%d, SOL_SOCKET, SO_LINGER, {l_onoff=%d}"
	       ", [%d]) = %s\n",
	       fd, linger->l_onoff, *len, errstr);

	/* getsockopt with optlen larger than usual - truncated to raw */
	*len = sizeof(*linger) - 1;
	get_linger(fd, linger, len);
	printf("getsockopt(%d, SOL_SOCKET, SO_LINGER, ", fd);
	print_quoted_hex(linger, *len);
	printf(", [%d]) = %s\n", *len, errstr);

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
