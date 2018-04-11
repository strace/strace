/*
 * Check decoding of SO_PEERCRED socket option.
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

#include <stddef.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#include "print_fields.h"

static const char *errstr;

static int
get_peercred(int fd, void *val, socklen_t *len)
{
	int rc = getsockopt(fd, SOL_SOCKET, SO_PEERCRED, val, len);
	errstr = sprintrc(rc);
	return rc;
}

static const char *
so_str(void)
{
	static char buf[256];

	if (!buf[0]) {
#if XLAT_RAW
		snprintf(buf, sizeof(buf),
			 "%#x, %#x", SOL_SOCKET, SO_PEERCRED);
#elif XLAT_VERBOSE
		snprintf(buf, sizeof(buf),
			 "%#x /* SOL_SOCKET */, %#x /* SO_PEERCRED */",
			 SOL_SOCKET, SO_PEERCRED);
#else
		snprintf(buf, sizeof(buf),
			 "SOL_SOCKET, SO_PEERCRED");
#endif
	}

	return buf;
}

int
main(void)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(struct ucred, peercred);
	TAIL_ALLOC_OBJECT_CONST_PTR(socklen_t, len);

	int sv[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv))
                perror_msg_and_skip("socketpair AF_UNIX SOCK_STREAM");

	/* classic getsockopt */
	*len = sizeof(*peercred);
	get_peercred(sv[0], peercred, len);
	printf("getsockopt(%d, %s", sv[0], so_str());
	PRINT_FIELD_D(", {", *peercred, pid);
	PRINT_FIELD_UID(", ", *peercred, uid);
	PRINT_FIELD_UID(", ", *peercred, gid);
	printf("}, [%d]) = %s\n", *len, errstr);

	int fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0)
		perror_msg_and_skip("socket AF_UNIX SOCK_STREAM");

	/* getsockopt with optlen larger than necessary - shortened */
	*len = sizeof(*peercred) + 1;
	get_peercred(fd, peercred, len);
	printf("getsockopt(%d, %s", fd, so_str());
	PRINT_FIELD_D(", {", *peercred, pid);
	PRINT_FIELD_UID(", ", *peercred, uid);
	PRINT_FIELD_UID(", ", *peercred, gid);
	printf("}, [%u->%d]) = %s\n",
	       (unsigned int) sizeof(*peercred) + 1, *len, errstr);

	/* getsockopt with optlen smaller than usual - truncated to ucred.pid */
	*len = sizeof(peercred->pid);
	get_peercred(fd, peercred, len);
	printf("getsockopt(%d, %s", fd, so_str());
	PRINT_FIELD_D(", {", *peercred, pid);
	printf("}, [%d]) = %s\n", *len, errstr);

	/* getsockopt with optlen smaller than usual - truncated to ucred.uid */
	*len = offsetof(struct ucred, gid);
	get_peercred(fd, peercred, len);
	printf("getsockopt(%d, %s", fd, so_str());
	PRINT_FIELD_D(", {", *peercred, pid);
	PRINT_FIELD_UID(", ", *peercred, uid);
	printf("}, [%d]) = %s\n", *len, errstr);

	/* getsockopt with optlen larger than usual - truncated to raw */
	*len = sizeof(*peercred) - 1;
	get_peercred(fd, peercred, len);
	printf("getsockopt(%d, %s, ", fd, so_str());
	print_quoted_hex(peercred, *len);
	printf(", [%d]) = %s\n", *len, errstr);

	/* getsockopt optval EFAULT */
	*len = sizeof(*peercred);
	get_peercred(fd, &peercred->uid, len);
	printf("getsockopt(%d, %s, %p, [%d]) = %s\n",
	       fd, so_str(), &peercred->uid, *len, errstr);

	/* getsockopt optlen EFAULT */
	get_peercred(fd, peercred, len + 1);
	printf("getsockopt(%d, %s, %p, %p) = %s\n",
	       fd, so_str(), peercred, len + 1, errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
