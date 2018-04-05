/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2018 The strace developers.
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
#include "netlink.h"

static const char *errstr;

static ssize_t
sys_send(const int fd, const void *const buf, const size_t len)
{
	const ssize_t rc = sendto(fd, buf, len, MSG_DONTWAIT, NULL, 0);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	int fd = create_nl_socket(NETLINK_KOBJECT_UEVENT);

	/* test using data that looks like a zero-length C string */
	char *const buf = tail_alloc(DEFAULT_STRLEN + 1);
	buf[0] = '=';
	fill_memory_ex(buf + 1, DEFAULT_STRLEN, 0, DEFAULT_STRLEN);

	sys_send(fd, buf + 1, DEFAULT_STRLEN);
	printf("sendto(%d, ", fd);
	print_quoted_memory(buf + 1, DEFAULT_STRLEN);
	printf(", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       DEFAULT_STRLEN, errstr);

	sys_send(fd, buf, DEFAULT_STRLEN + 1);
	printf("sendto(%d, ", fd);
	print_quoted_memory(buf, DEFAULT_STRLEN);
	printf("..., %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       DEFAULT_STRLEN + 1, errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
