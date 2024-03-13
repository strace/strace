/*
 * Check decoding of TCP_AO_ADD_KEY socket option.
 *
 * Copyright (c) 2017-2021 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <linux/tcp.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static const char *errstr;

static int
add_key(int fd, void *val, int len)
{
	int rc = setsockopt(fd, IPPROTO_TCP, TCP_AO_ADD_KEY, val, len);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(struct tcp_ao_add, key);

	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
		perror_msg_and_skip("socket AF_TCP SOCK_STREAM");

	add_key(fd, 0, 0);
	printf("setsockopt(%d, SOL_TCP, TCP_AO_ADD_KEY, NULL, 0) = %s\n",
	       fd, errstr);

	void *bad_key = (char *) key + 1;
	add_key(fd, bad_key, sizeof(*key));
	printf("setsockopt(%d, SOL_TCP, TCP_AO_ADD_KEY, %p, %zu) = %s\n",
	       fd, bad_key, sizeof(*key), errstr);

	add_key(fd, key, 0);
	printf("setsockopt(%d, SOL_TCP, TCP_AO_ADD_KEY, %p, 0) = %s\n",
	       fd, key, errstr);

#define KEY1 "\x42\xe9\xd2\xd3\xd1\xec\x9f\x55\x56\x9c\xd7\x89\x1a\x90\x53\xba\x59\x6d\x5f\x0a"
	*key = (struct tcp_ao_add) {
		.prefix = 0,
		.alg_name = "hmac(sha1)",
		.ifindex = 0,
		.set_current = 0,
		.set_rnext = 0,
		.sndid = 200,
		.rcvid = 100,
		.maclen = 12,
		.keyflags = 0,
		.key = KEY1,
		.keylen = sizeof(KEY1) - 1,
	};
	struct sockaddr_in6 addr6 =
		{.sin6_family = AF_INET6, .sin6_addr = IN6ADDR_ANY_INIT};
	memcpy(&key->addr, &addr6, sizeof(addr6));
	add_key(fd, key, sizeof(*key) + 1);
	printf("setsockopt(%d, SOL_TCP, TCP_AO_ADD_KEY, "
	       "{addr={sa_family=AF_INET6, sin6_port=htons(0), "
	       "sin6_flowinfo=htonl(0), inet_pton(AF_INET6, \"::\", "
	       "&sin6_addr), sin6_scope_id=0}, prefix=0, "
	       "alg_name=\"hmac(sha1)\", ifindex=0, set_current=0, "
	       "set_rnext=0, sndid=200, rcvid=100, maclen=12, keyflags=0, "
	       "key="
	       "\"\\x42\\xe9\\xd2\\xd3\\xd1\\xec\\x9f\\x55\\x56\\x9c\\xd7\\x89"
	       "\\x1a\\x90\\x53\\xba\\x59\\x6d\\x5f\\x0a\", keylen=%zu}"
	       ", %zu) = %s\n",
	       fd, sizeof(KEY1) - 1, sizeof(*key) + 1, errstr);

#define KEY2 "\x7a\x66\x25\xc9\x80\xdb\x68\x95\xf5\xaf\x84\x1b\xd6\x50\x29\xe1"
	*key = (struct tcp_ao_add) {
		.prefix = 32,
		.alg_name = "cmac(aes)",
		.ifindex = 0,
		.set_current = 0,
		.set_rnext = 0,
		.sndid = 200,
		.rcvid = 100,
		.maclen = 12,
		.keyflags = TCP_AO_KEYF_IFINDEX|TCP_AO_KEYF_EXCLUDE_OPT,
		.key = KEY2,
		.keylen = sizeof(KEY2) - 1,
	};
	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_addr = { htonl(INADDR_LOOPBACK) }
	};
	memcpy(&key->addr, &addr, sizeof(addr));
	add_key(fd, key, sizeof(*key));
	printf("setsockopt(%d, SOL_TCP, TCP_AO_ADD_KEY, "
	       "{addr={sa_family=AF_INET, sin_port=htons(0), "
	       "sin_addr=inet_addr(\"127.0.0.1\")}, prefix=32, "
	       "alg_name=\"cmac(aes)\", ifindex=0, set_current=0, set_rnext=0, "
	       "sndid=200, rcvid=100, maclen=12, "
	       "keyflags=TCP_AO_KEYF_IFINDEX|TCP_AO_KEYF_EXCLUDE_OPT, key="
	       "\"\\x7a\\x66\\x25\\xc9\\x80\\xdb\\x68\\x95\\xf5\\xaf\\x84\\x1b"
	       "\\xd6\\x50\\x29\\xe1\", keylen=%zu}"
	       ", %zu) = %s\n",
	       fd, sizeof(KEY2) - 1, sizeof(*key), errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
