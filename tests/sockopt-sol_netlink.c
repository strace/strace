/*
 * Check decoding of getsockopt and setsockopt for SOL_NETLINK level.
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
#include "netlink.h"
#include <stdio.h>

#ifndef SOL_NETLINK
# define SOL_NETLINK 270
#endif

static int rc;
static const char *errstr;

static int
get_sockopt(int fd, int name, void *val, socklen_t *len)
{
	rc = getsockopt(fd, SOL_NETLINK, name, val, len);
	errstr = sprintrc(rc);
	return rc;
}

static int
set_sockopt(int fd, int name, void *val, socklen_t len)
{
	rc = setsockopt(fd, SOL_NETLINK, name, val, len);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	static const struct {
		int val;
		const char *str;
	} names[] = {
#ifdef NETLINK_ADD_MEMBERSHIP
		{ ARG_STR(NETLINK_ADD_MEMBERSHIP) },
#endif
#ifdef NETLINK_DROP_MEMBERSHIP
		{ ARG_STR(NETLINK_DROP_MEMBERSHIP) },
#endif
#ifdef NETLINK_PKTINFO
		{ ARG_STR(NETLINK_PKTINFO) },
#endif
#ifdef NETLINK_BROADCAST_ERROR
		{ ARG_STR(NETLINK_BROADCAST_ERROR) },
#endif
#ifdef NETLINK_NO_ENOBUFS
		{ ARG_STR(NETLINK_NO_ENOBUFS) },
#endif
#ifdef NETLINK_RX_RING
		{ ARG_STR(NETLINK_RX_RING) },
#endif
#ifdef NETLINK_TX_RING
		{ ARG_STR(NETLINK_TX_RING) },
#endif
#ifdef NETLINK_LISTEN_ALL_NSID
		{ ARG_STR(NETLINK_LISTEN_ALL_NSID) },
#endif
#ifdef NETLINK_LIST_MEMBERSHIPS
		{ ARG_STR(NETLINK_LIST_MEMBERSHIPS) },
#endif
#ifdef NETLINK_CAP_ACK
		{ ARG_STR(NETLINK_CAP_ACK) },
#endif
#ifdef NETLINK_EXT_ACK
		{ ARG_STR(NETLINK_EXT_ACK) },
#endif
	};

	TAIL_ALLOC_OBJECT_CONST_PTR(int, val);
	TAIL_ALLOC_OBJECT_CONST_PTR(socklen_t, len);
	void *const efault = val + 1;
        int fd = socket(AF_NETLINK, SOCK_RAW, 0);
        if (fd < 0)
                perror_msg_and_skip("socket AF_NETLINK SOCK_RAW");
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(names); ++i) {
		/* getsockopt */

		/* classic */
		*len = sizeof(*val);
		get_sockopt(fd, names[i].val, val, len);
		printf("getsockopt(%d, SOL_NETLINK, %s, ", fd, names[i].str);
		if (rc)
			printf("%p", val);
		else
			printf("[%d]", *val);
		printf(", [%d]) = %s\n", *len, errstr);

		/* optlen larger than necessary - shortened */
		*len = sizeof(*val) + 1;
		get_sockopt(fd, names[i].val, val, len);
		printf("getsockopt(%d, SOL_NETLINK, %s, ", fd, names[i].str);
		if (rc)
			printf("%p", val);
		else
			printf("[%d]", *val);
		printf(", [%d", (int) sizeof(*val) + 1);
		if ((int) sizeof(*val) + 1 != *len)
			printf("->%d", *len);
		printf("]) = %s\n", errstr);

		/* zero optlen - print returned optlen */
		*len = 0;
		get_sockopt(fd, names[i].val, NULL, len);
		printf("getsockopt(%d, SOL_NETLINK, %s, NULL, [0",
		       fd, names[i].str);
		if (*len)
			printf("->%d", *len);
		printf("]) = %s\n", errstr);

#ifdef NETLINK_LIST_MEMBERSHIPS
		if (names[i].val != NETLINK_LIST_MEMBERSHIPS) {
#endif
			/* optlen shorter than necessary - print address */
			*len = sizeof(*val) - 1;
			get_sockopt(fd, names[i].val, val, len);
			printf("getsockopt(%d, SOL_NETLINK, %s, %p, [%d",
			       fd, names[i].str, val, (int) sizeof(*val) - 1);
			if ((int) sizeof(*val) - 1 != *len)
				printf("->%d", *len);
			printf("]) = %s\n", errstr);
#ifdef NETLINK_LIST_MEMBERSHIPS
		} else {
			/* optlen shorter than required for the first element */
			*len = sizeof(*val) - 1;
			get_sockopt(fd, names[i].val, efault, len);
			printf("getsockopt(%d, SOL_NETLINK, %s, ",
			       fd, names[i].str);
			if (rc)
				printf("%p", efault);
			else
				printf("[]");
			printf(", [%d", (int) sizeof(*val) - 1);
			if ((int) sizeof(*val) - 1 != *len)
				printf("->%d", *len);
			printf("]) = %s\n", errstr);
		}
#endif

		/* optval EFAULT - print address */
		*len = sizeof(*val);
		get_sockopt(fd, names[i].val, efault, len);
		printf("getsockopt(%d, SOL_NETLINK, %s, %p, [%d]) = %s\n",
		       fd, names[i].str, efault, *len, errstr);

		/* optlen EFAULT - print address */
		get_sockopt(fd, names[i].val, val, len + 1);
		printf("getsockopt(%d, SOL_NETLINK, %s, %p, %p) = %s\n",
		       fd, names[i].str, val, len + 1, errstr);

		/* setsockopt */

		/* classic */
		*val = 0xdefaced;
		set_sockopt(fd, names[i].val, val, sizeof(*val));
		printf("setsockopt(%d, SOL_NETLINK, %s, [%d], %d) = %s\n",
		       fd, names[i].str, *val, (int) sizeof(*val), errstr);

		/* optlen larger than necessary - shortened */
		set_sockopt(fd, names[i].val, val, sizeof(*val) + 1);
		printf("setsockopt(%d, SOL_NETLINK, %s, [%d], %d) = %s\n",
		       fd, names[i].str, *val, (int) sizeof(*val) + 1, errstr);

		/* optlen < 0 - print address */
		set_sockopt(fd, names[i].val, val, -1U);
		printf("setsockopt(%d, SOL_NETLINK, %s, %p, -1) = %s\n",
		       fd, names[i].str, val, errstr);

		/* optlen smaller than necessary - print address */
		set_sockopt(fd, names[i].val, val, sizeof(*val) - 1);
		printf("setsockopt(%d, SOL_NETLINK, %s, %p, %d) = %s\n",
		       fd, names[i].str, val, (int) sizeof(*val) - 1, errstr);

		/* optval EFAULT - print address */
		set_sockopt(fd, names[i].val, efault, sizeof(*val));
		printf("setsockopt(%d, SOL_NETLINK, %s, %p, %d) = %s\n",
		       fd, names[i].str, efault, (int) sizeof(*val), errstr);
	}

	puts("+++ exited with 0 +++");
	return 0;
}
