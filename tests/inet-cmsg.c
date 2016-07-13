/*
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
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
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static void
print_pktinfo(const struct cmsghdr *c)
{
	printf("IP_PKTINFO, cmsg_data={ipi_ifindex=if_nametoindex(\"lo\")"
	       ", ipi_spec_dst=inet_addr(\"127.0.0.1\")"
	       ", ipi_addr=inet_addr(\"127.0.0.1\")}");
}

static void
print_ttl(const struct cmsghdr *c)
{
	const unsigned int *ttl = (const unsigned int *) CMSG_DATA(c);

	printf("IP_TTL, cmsg_data=[%u]", *ttl);
}

static void
print_tos(const struct cmsghdr *c)
{
	const uint8_t *tos = (const uint8_t *) CMSG_DATA(c);

	printf("IP_TOS, cmsg_data=[%#x]", *tos);
}

static void
print_opts(const char *name, const struct cmsghdr *c)
{
	const unsigned char *opts = (const unsigned char *) CMSG_DATA(c);
	const size_t len = c->cmsg_len - CMSG_ALIGN(sizeof(*c));

	printf("%s", name);
	if (len) {
		printf(", cmsg_data=[");
		size_t i;
		for (i = 0; i < len; ++i)
			printf("%s0x%02x", i ? ", " : "", opts[i]);
		printf("]");
	}
}

#ifdef IP_ORIGDSTADDR
static void
print_origdstaddr(const struct cmsghdr *c)
{
	const struct sockaddr_in *sin =
		(const struct sockaddr_in *) CMSG_DATA(c);

	printf("IP_ORIGDSTADDR, cmsg_data={sa_family=AF_INET, sin_port=htons(%u)"
	       ", sin_addr=inet_addr(\"127.0.0.1\")}", ntohs(sin->sin_port));
}
#endif

int
main(void)
{
	int i;
	while ((i = open("/dev/null", O_RDWR)) < 3)
		assert(i >= 0);
	assert(!close(0));
	assert(!close(3));

	if (socket(AF_INET, SOCK_DGRAM, 0))
		perror_msg_and_skip("socket");
	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_addr.s_addr = htonl(INADDR_LOOPBACK)
	};
	socklen_t len = sizeof(addr);
	if (bind(0, (struct sockaddr *) &addr, len))
		perror_msg_and_skip("bind");
	assert(!getsockname(0, (struct sockaddr *) &addr, &len));

	assert(socket(AF_INET, SOCK_DGRAM, 0) == 3);
	assert(!connect(3, (struct sockaddr *) &addr, len));

	const int opt_1 = htonl(0x01000000);
#define SETSOCKOPT(fd, name) assert(!setsockopt(fd, IPPROTO_IP, (name), &opt_1, sizeof(opt_1)))
	SETSOCKOPT(3, IP_OPTIONS);
	SETSOCKOPT(0, IP_PKTINFO);
	SETSOCKOPT(0, IP_RECVTTL);
	SETSOCKOPT(0, IP_RECVTOS);
	SETSOCKOPT(0, IP_RECVOPTS);
	SETSOCKOPT(0, IP_RETOPTS);
#ifdef IP_RECVORIGDSTADDR
	SETSOCKOPT(0, IP_RECVORIGDSTADDR);
#endif

	static const char data[] = "data";
	const size_t size = sizeof(data) - 1;
	assert(send(3, data, size, 0) == (int) size);
	assert(!close(3));

	char buf[size];
	struct iovec iov = {
		.iov_base = buf,
		.iov_len = sizeof(buf)
	};
	struct cmsghdr control[16];
	struct msghdr mh = {
		.msg_name = &addr,
		.msg_namelen = len,
		.msg_iov = &iov,
		.msg_iovlen = 1,
		.msg_control = control,
		.msg_controllen = sizeof(control)
	};

	assert(recvmsg(0, &mh, 0) == (int) size);
	assert(!close(0));

	printf("recvmsg(0, {msg_name={sa_family=AF_INET, sin_port=htons(%u)"
	       ", sin_addr=inet_addr(\"127.0.0.1\")}, msg_namelen=%u"
	       ", msg_iov=[{iov_base=\"%s\", iov_len=%u}], msg_iovlen=1"
	       ", msg_control=[",
	       ntohs(addr.sin_port), (unsigned) mh.msg_namelen,
	       data, (unsigned) size);

	struct cmsghdr *c;
	for (c = CMSG_FIRSTHDR(&mh); c; c = CMSG_NXTHDR(&mh, c)) {
		if (IPPROTO_IP != c->cmsg_level)
			continue;
		if (c != control)
			printf(", ");
		printf("{cmsg_len=%lu, cmsg_level=SOL_IP, cmsg_type=",
		       (unsigned long) c->cmsg_len);
		switch (c->cmsg_type) {
			case IP_PKTINFO:
				print_pktinfo(c);
				break;
			case IP_TTL:
				print_ttl(c);
				break;
			case IP_TOS:
				print_tos(c);
				break;
			case IP_RECVOPTS:
				print_opts("IP_RECVOPTS", c);
				break;
			case IP_RETOPTS:
				print_opts("IP_RETOPTS", c);
				break;
#ifdef IP_ORIGDSTADDR
			case IP_ORIGDSTADDR:
				print_origdstaddr(c);
				break;
#endif
			default:
				printf("%d", c->cmsg_type);
				break;
		}
		printf("}");
	}
	printf("], msg_controllen=%lu, msg_flags=0}, 0) = %u\n",
	       (unsigned long) mh.msg_controllen, (unsigned) size);
	puts("+++ exited with 0 +++");

	return 0;
}
