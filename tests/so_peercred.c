/*
 * Check decoding of SO_PEERCRED socket option.
 *
 * Copyright (c) 2017 Dmitry V. Levin <ldv@altlinux.org>
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

#include <stddef.h>
#include <stdio.h>
#include <string.h>
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

	const unsigned int sizeof_pid = sizeof(peercred->pid);
	struct ucred *const pid = tail_alloc(sizeof_pid);

	const unsigned int sizeof_pid_truncated = sizeof_pid - 1;
	struct ucred *const pid_truncated =
		tail_alloc(sizeof_pid_truncated);

	const unsigned int sizeof_uid = offsetofend(struct ucred, uid);
	struct ucred *const uid = tail_alloc(sizeof_uid);

	const unsigned int sizeof_uid_truncated = sizeof_uid - 1;
	struct ucred *const uid_truncated =
		tail_alloc(sizeof_uid_truncated);

	const unsigned int sizeof_gid_truncated =
		offsetofend(struct ucred, gid) - 1;
	struct ucred *const gid_truncated =
		tail_alloc(sizeof_gid_truncated);

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

	/* getsockopt with zero optlen */
	*len = 0;
	get_peercred(sv[0], peercred, len);
	printf("getsockopt(%d, %s, %p, [0]) = %s\n",
	       sv[0], so_str(), peercred, errstr);

	/* getsockopt with optlen larger than necessary - shortened */
	*len = sizeof(*peercred) + 1;
	get_peercred(sv[0], peercred, len);
	printf("getsockopt(%d, %s", sv[0], so_str());
	PRINT_FIELD_D(", {", *peercred, pid);
	PRINT_FIELD_UID(", ", *peercred, uid);
	PRINT_FIELD_UID(", ", *peercred, gid);
	printf("}, [%u->%d]) = %s\n",
	       (unsigned int) sizeof(*peercred) + 1, *len, errstr);

	/*
	 * getsockopt with optlen less than offsetofend(struct ucred, pid):
	 * the part of struct ucred.pid is printed in hex.
	 */
	*len = sizeof_pid_truncated;
	get_peercred(sv[0], pid_truncated, len);
	printf("getsockopt(%d, %s, {pid=", sv[0], so_str());
	print_quoted_hex(pid_truncated, *len);
	printf("}, [%d]) = %s\n", *len, errstr);

	/*
	 * getsockopt with optlen equals to sizeof(struct ucred.pid):
	 * struct ucred.uid and struct ucred.gid are not printed.
	 */
	*len = sizeof_pid;
	get_peercred(sv[0], pid, len);
	printf("getsockopt(%d, %s", sv[0], so_str());
	PRINT_FIELD_D(", {", *pid, pid);
	printf("}, [%d]) = %s\n", *len, errstr);

	/*
	 * getsockopt with optlen greater than sizeof(struct ucred.pid)
	 * but smaller than offsetofend(struct ucred, uid):
	 * the part of struct ucred.uid is printed in hex.
	 */
	*len = sizeof_uid_truncated;
	get_peercred(sv[0], uid_truncated, len);
	/*
	 * Copy to a properly aligned structure to avoid unaligned access
	 * to struct ucred.pid field.
	 */
	memcpy(uid, uid_truncated, sizeof_uid_truncated);
	printf("getsockopt(%d, %s", sv[0], so_str());
	PRINT_FIELD_D(", {", *uid, pid);
	printf(", uid=");
	print_quoted_hex(&uid->uid, sizeof_uid_truncated -
				    offsetof(struct ucred, uid));
	printf("}, [%d]) = %s\n", *len, errstr);

	/*
	 * getsockopt with optlen equals to offsetofend(struct ucred, uid):
	 * struct ucred.gid is not printed.
	 */
	*len = sizeof_uid;
	get_peercred(sv[0], uid, len);
	printf("getsockopt(%d, %s", sv[0], so_str());
	PRINT_FIELD_D(", {", *uid, pid);
	PRINT_FIELD_UID(", ", *uid, uid);
	printf("}, [%d]) = %s\n", *len, errstr);

	/*
	 * getsockopt with optlen greater than sizeof(struct ucred.uid)
	 * but smaller than offsetofend(struct ucred, gid):
	 * the part of struct ucred.gid is printed in hex.
	 */
	*len = sizeof_gid_truncated;
	get_peercred(sv[0], gid_truncated, len);
	/*
	 * Copy to a properly aligned structure to avoid unaligned access
	 * to struct ucred.pid and struct ucred.uid fields.
	 */
	memcpy(peercred, gid_truncated, sizeof_gid_truncated);
	printf("getsockopt(%d, %s", sv[0], so_str());
	PRINT_FIELD_D(", {", *peercred, pid);
	PRINT_FIELD_UID(", ", *peercred, uid);
	printf(", gid=");
	print_quoted_hex(&peercred->gid, sizeof_gid_truncated -
				    offsetof(struct ucred, gid));
	printf("}, [%d]) = %s\n", *len, errstr);

	/* getsockopt optval EFAULT */
	*len = sizeof(*peercred);
	get_peercred(sv[0], &peercred->uid, len);
	printf("getsockopt(%d, %s, %p, [%d]) = %s\n",
	       sv[0], so_str(), &peercred->uid, *len, errstr);

	/* getsockopt optlen EFAULT */
	get_peercred(sv[0], peercred, len + 1);
	printf("getsockopt(%d, %s, %p, %p) = %s\n",
	       sv[0], so_str(), peercred, len + 1, errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
