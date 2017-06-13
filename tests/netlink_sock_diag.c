/*
 * Copyright (c) 2016 Fabien Siron <fabien.siron@epita.fr>
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017 The strace developers.
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
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <linux/netlink.h>
#include <linux/netlink_diag.h>
#include <linux/sock_diag.h>
#include <linux/unix_diag.h>

# if !defined NETLINK_SOCK_DIAG && defined NETLINK_INET_DIAG
#  define NETLINK_SOCK_DIAG NETLINK_INET_DIAG
# endif

static void
test_nlmsg_type(const int fd)
{
	long rc;
	struct nlmsghdr nlh = {
		.nlmsg_len = sizeof(nlh),
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_REQUEST,
	};

	rc = sendto(fd, &nlh, sizeof(nlh), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh.nlmsg_len, (unsigned) sizeof(nlh), sprintrc(rc));
}

static void
test_nlmsg_flags(const int fd)
{
	long rc;
	struct nlmsghdr nlh = {
		.nlmsg_len = sizeof(nlh),
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP,
	};

	rc = sendto(fd, &nlh, sizeof(nlh), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_REQUEST|NLM_F_DUMP, seq=0, pid=0}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh.nlmsg_len, (unsigned) sizeof(nlh), sprintrc(rc));
}

static void
test_odd_family_req(const int fd)
{
	struct nlmsghdr *nlh;
	uint8_t *family;
	void *const nlh0 = tail_alloc(NLMSG_HDRLEN);
	long rc;

	/* unspecified family only */
	nlh = nlh0 - sizeof(*family);
	/* beware of unaligned access to nlh members */
	*nlh = (struct nlmsghdr) {
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*family),
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_REQUEST,
	};
	family = NLMSG_DATA(nlh);
	*family = 0;

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*family), MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}"
	       ", {family=AF_UNSPEC}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*family),
	       NLMSG_HDRLEN + (unsigned int) sizeof(*family),
	       sprintrc(rc));

	/* unknown family only */
	*family = 0xff;

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*family), MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}"
	       ", {family=0xff /* AF_??? */}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*family),
	       NLMSG_HDRLEN + (unsigned int) sizeof(*family),
	       sprintrc(rc));

	/* short read of family */
	memmove(nlh0, nlh, NLMSG_HDRLEN);
	nlh = nlh0;

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*family), MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}, %p}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*family),
	       NLMSG_DATA(nlh),
	       NLMSG_HDRLEN + (unsigned int) sizeof(*family),
	       sprintrc(rc));

	/* unspecified family and string */
	nlh = nlh0 - (sizeof(*family) + 4);
	/* beware of unaligned access to nlh members */
	*nlh = (struct nlmsghdr) {
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*family) + 4,
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_REQUEST,
	};
	family = NLMSG_DATA(nlh);
	*family = 0;
	memcpy(family + 1, "1234", 4);

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*family) + 4, MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}"
	       ", {family=AF_UNSPEC, \"1234\"}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*family) + 4,
	       NLMSG_HDRLEN + (unsigned int) sizeof(*family) + 4,
	       sprintrc(rc));

	/* unknown family and string */
	*family = 0xfd;

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*family) + 4, MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}"
	       ", {family=0xfd /* AF_??? */, \"1234\"}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*family) + 4,
	       NLMSG_HDRLEN + (unsigned int) sizeof(*family) + 4,
	       sprintrc(rc));
}

static void
test_odd_family_msg(const int fd)
{
	struct nlmsghdr *nlh;
	uint8_t *family;
	void *const nlh0 = tail_alloc(NLMSG_HDRLEN);
	long rc;

	/* unspecified family only */
	nlh = nlh0 - sizeof(*family);
	/* beware of unaligned access to nlh members */
	*nlh = (struct nlmsghdr) {
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*family),
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_DUMP,
	};
	family = NLMSG_DATA(nlh);
	*family = 0;

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*family), MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}"
	       ", {family=AF_UNSPEC}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*family),
	       NLMSG_HDRLEN + (unsigned int) sizeof(*family),
	       sprintrc(rc));

	/* unknown family only */
	*family = 0xff;

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*family), MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}"
	       ", {family=0xff /* AF_??? */}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*family),
	       NLMSG_HDRLEN + (unsigned int) sizeof(*family),
	       sprintrc(rc));

	/* short read of family */
	memmove(nlh0, nlh, NLMSG_HDRLEN);
	nlh = nlh0;

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*family), MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, %p}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*family),
	       NLMSG_DATA(nlh),
	       NLMSG_HDRLEN + (unsigned int) sizeof(*family),
	       sprintrc(rc));

	/* unspecified family and string */
	nlh = nlh0 - (sizeof(*family) + 4);
	/* beware of unaligned access to nlh members */
	*nlh = (struct nlmsghdr) {
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*family) + 4,
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_DUMP,
	};
	family = NLMSG_DATA(nlh);
	*family = 0;
	memcpy(family + 1, "1234", 4);

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*family) + 4, MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}"
	       ", {family=AF_UNSPEC, \"1234\"}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*family) + 4,
	       NLMSG_HDRLEN + (unsigned int) sizeof(*family) + 4,
	       sprintrc(rc));

	/* unknown family and string */
	*family = 0xfb;

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*family) + 4, MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}"
	       ", {family=0xfb /* AF_??? */, \"1234\"}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*family) + 4,
	       NLMSG_HDRLEN + (unsigned int) sizeof(*family) + 4,
	       sprintrc(rc));
}

static void
test_unix_diag_req(const int fd)
{
	struct nlmsghdr *nlh;
	struct unix_diag_req *req;
	uint8_t *family;
	void *const nlh0 = tail_alloc(NLMSG_HDRLEN);
	long rc;

	/* family only */
	nlh = nlh0 - sizeof(*family);
	/* beware of unaligned access to nlh members */
	*nlh = (struct nlmsghdr) {
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*family),
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_REQUEST,
	};
	family = NLMSG_DATA(nlh);
	*family = AF_UNIX;

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*family), MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}, {family=AF_UNIX}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*family),
	       NLMSG_HDRLEN + (unsigned int) sizeof(*family),
	       sprintrc(rc));

	/* family and string */
	nlh = nlh0 - (sizeof(*family) + 4);
	/* beware of unaligned access to nlh members */
	*nlh = (struct nlmsghdr) {
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*family) + 4,
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_REQUEST,
	};
	family = NLMSG_DATA(nlh);
	*family = AF_UNIX;
	memcpy(family + 1, "1234", 4);

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*family) + 4, MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}"
	       ", {sdiag_family=AF_UNIX, ...}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*family) + 4,
	       NLMSG_HDRLEN + (unsigned int) sizeof(*family) + 4,
	       sprintrc(rc));

	/* unix_diag_req */
	nlh = nlh0 - sizeof(*req);
	*nlh = (struct nlmsghdr) {
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*req),
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_REQUEST,
	};
	req = NLMSG_DATA(nlh);
	*req = (struct unix_diag_req) {
		.sdiag_family = AF_UNIX,
		.sdiag_protocol = 253,
		.udiag_states = 1 << TCP_ESTABLISHED | 1 << TCP_LISTEN,
		.udiag_ino = 0xfacefeed,
		.udiag_show = UDIAG_SHOW_NAME,
		.udiag_cookie = { 0xdeadbeef, 0xbadc0ded }
	};

	rc = sendto(fd, nlh, nlh->nlmsg_len, MSG_DONTWAIT, NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}"
	       ", {sdiag_family=AF_UNIX, sdiag_protocol=%u"
	       ", udiag_states=1<<TCP_ESTABLISHED|1<<TCP_LISTEN, udiag_ino=%u"
	       ", udiag_show=UDIAG_SHOW_NAME, udiag_cookie=[%u, %u]}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh->nlmsg_len,
	       253, 0xfacefeed, 0xdeadbeef, 0xbadc0ded,
	       nlh->nlmsg_len, sprintrc(rc));

	/* short read of unix_diag_req */
	nlh = nlh0 - (sizeof(*req) - 1);
	/* beware of unaligned access to nlh members */
	memmove(nlh, nlh0 - sizeof(*req), NLMSG_HDRLEN + sizeof(*req) - 1);

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*req), MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}, {sdiag_family=AF_UNIX, %p}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*req),
	       NLMSG_DATA(nlh) + 1,
	       NLMSG_HDRLEN + (unsigned int) sizeof(*req),
	       sprintrc(rc));
}

static void
test_unix_diag_msg(const int fd)
{
	struct nlmsghdr *nlh;
	struct unix_diag_msg *msg;
	uint8_t *family;
	void *const nlh0 = tail_alloc(NLMSG_HDRLEN);
	long rc;

	/* family only */
	nlh = nlh0 - sizeof(*family);
	/* beware of unaligned access to nlh members */
	*nlh = (struct nlmsghdr) {
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*family),
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_DUMP,
	};
	family = NLMSG_DATA(nlh);
	*family = AF_UNIX;

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*family), MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {family=AF_UNIX}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*family),
	       NLMSG_HDRLEN + (unsigned int) sizeof(*family),
	       sprintrc(rc));

	/* family and string */
	nlh = nlh0 - (sizeof(*family) + 4);
	/* beware of unaligned access to nlh members */
	*nlh = (struct nlmsghdr) {
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*family) + 4,
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_DUMP,
	};
	family = NLMSG_DATA(nlh);
	*family = AF_UNIX;
	memcpy(family + 1, "1234", 4);

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*family) + 4, MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}"
	       ", {udiag_family=AF_UNIX, ...}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*family) + 4,
	       NLMSG_HDRLEN + (unsigned int) sizeof(*family) + 4,
	       sprintrc(rc));

	/* unix_diag_msg */
	nlh = nlh0 - sizeof(*msg);
	*nlh = (struct nlmsghdr) {
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*msg),
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_DUMP,
	};
	msg = NLMSG_DATA(nlh);
	*msg = (struct unix_diag_msg) {
		.udiag_family = AF_UNIX,
		.udiag_type = SOCK_STREAM,
		.udiag_state = TCP_FIN_WAIT1,
		.udiag_ino = 0xfacefeed,
		.udiag_cookie = { 0xdeadbeef, 0xbadc0ded }
	};

	rc = sendto(fd, nlh, nlh->nlmsg_len, MSG_DONTWAIT, NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}"
	       ", {udiag_family=AF_UNIX, udiag_type=SOCK_STREAM"
	       ", udiag_state=TCP_FIN_WAIT1"
	       ", udiag_ino=%u, udiag_cookie=[%u, %u]}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh->nlmsg_len,
	       0xfacefeed, 0xdeadbeef, 0xbadc0ded,
	       nlh->nlmsg_len, sprintrc(rc));

	/* short read of unix_diag_msg */
	nlh = nlh0 - (sizeof(*msg) - 1);
	/* beware of unaligned access to nlh members */
	memmove(nlh, nlh0 - sizeof(*msg), NLMSG_HDRLEN + sizeof(*msg) - 1);

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*msg), MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {udiag_family=AF_UNIX, %p}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*msg),
	       NLMSG_DATA(nlh) + 1,
	       NLMSG_HDRLEN + (unsigned int) sizeof(*msg),
	       sprintrc(rc));
}

static void
test_netlink_diag_req(const int fd)
{
	struct nlmsghdr *nlh;
	struct netlink_diag_req *req;
	uint8_t *family;
	void *const nlh0 = tail_alloc(NLMSG_HDRLEN);
	long rc;

	/* family only */
	nlh = nlh0 - sizeof(*family);
	/* beware of unaligned access to nlh members */
	*nlh = (struct nlmsghdr) {
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*family),
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_REQUEST,
	};
	family = NLMSG_DATA(nlh);
	*family = AF_NETLINK;

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*family), MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}, {family=AF_NETLINK}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*family),
	       NLMSG_HDRLEN + (unsigned int) sizeof(*family),
	       sprintrc(rc));

	/* family and string */
	nlh = nlh0 - (sizeof(*family) + 4);
	/* beware of unaligned access to nlh members */
	*nlh = (struct nlmsghdr) {
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*family) + 4,
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_REQUEST,
	};
	family = NLMSG_DATA(nlh);
	*family = AF_NETLINK;
	memcpy(family + 1, "1234", 4);

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*family) + 4, MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}"
	       ", {sdiag_family=AF_NETLINK, ...}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*family) + 4,
	       NLMSG_HDRLEN + (unsigned int) sizeof(*family) + 4,
	       sprintrc(rc));

	/* netlink_diag_req */
	nlh = nlh0 - sizeof(*req);
	*nlh = (struct nlmsghdr) {
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*req),
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_REQUEST,
	};
	req = NLMSG_DATA(nlh);
	*req = (struct netlink_diag_req) {
		.sdiag_family = AF_NETLINK,
		.sdiag_protocol = NDIAG_PROTO_ALL,
		.ndiag_ino = 0xfacefeed,
		.ndiag_show = NDIAG_SHOW_MEMINFO,
		.ndiag_cookie = { 0xdeadbeef, 0xbadc0ded }
	};


	rc = sendto(fd, nlh, nlh->nlmsg_len, MSG_DONTWAIT, NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}"
	       ", {sdiag_family=AF_NETLINK, sdiag_protocol=NDIAG_PROTO_ALL"
	       ", ndiag_ino=%u, ndiag_show=NDIAG_SHOW_MEMINFO"
	       ", ndiag_cookie=[%u, %u]}}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh->nlmsg_len, 0xfacefeed, 0xdeadbeef,
	       0xbadc0ded, nlh->nlmsg_len, sprintrc(rc));

	req->sdiag_protocol = NETLINK_ROUTE;

	rc = sendto(fd, nlh, nlh->nlmsg_len, MSG_DONTWAIT, NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}"
	       ", {sdiag_family=AF_NETLINK, sdiag_protocol=NETLINK_ROUTE"
	       ", ndiag_ino=%u, ndiag_show=NDIAG_SHOW_MEMINFO"
	       ", ndiag_cookie=[%u, %u]}}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh->nlmsg_len, 0xfacefeed, 0xdeadbeef,
	       0xbadc0ded, nlh->nlmsg_len, sprintrc(rc));

	/* short read of netlink_diag_req */
	nlh = nlh0 - (sizeof(*req) - 1);
	/* beware of unaligned access to nlh members */
	memmove(nlh, nlh0 - sizeof(*req), NLMSG_HDRLEN + sizeof(*req) - 1);

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*req), MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}"
	       ", {sdiag_family=AF_NETLINK, %p}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*req),
	       NLMSG_DATA(nlh) + 1,
	       NLMSG_HDRLEN + (unsigned int) sizeof(*req),
	       sprintrc(rc));
}

static void
test_netlink_diag_msg(const int fd)
{
	struct nlmsghdr *nlh;
	struct netlink_diag_msg *msg;
	uint8_t *family;
	void *const nlh0 = tail_alloc(NLMSG_HDRLEN);
	long rc;

	/* family only */
	nlh = nlh0 - sizeof(*family);
	/* beware of unaligned access to nlh members */
	*nlh = (struct nlmsghdr) {
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*family),
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_DUMP,
	};
	family = NLMSG_DATA(nlh);
	*family = AF_NETLINK;

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*family), MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {family=AF_NETLINK}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*family),
	       NLMSG_HDRLEN + (unsigned int) sizeof(*family),
	       sprintrc(rc));

	/* family and string */
	nlh = nlh0 - (sizeof(*family) + 4);
	/* beware of unaligned access to nlh members */
	*nlh = (struct nlmsghdr) {
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*family) + 4,
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_DUMP,
	};
	family = NLMSG_DATA(nlh);
	*family = AF_NETLINK;
	memcpy(family + 1, "1234", 4);

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*family) + 4, MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}"
	       ", {ndiag_family=AF_NETLINK, ...}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*family) + 4,
	       NLMSG_HDRLEN + (unsigned int) sizeof(*family) + 4,
	       sprintrc(rc));

	/* netlink_diag_msg */
	nlh = nlh0 - sizeof(*msg);
	*nlh = (struct nlmsghdr) {
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*msg),
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_DUMP,
	};
	msg = NLMSG_DATA(nlh);
	*msg = (struct netlink_diag_msg) {
		.ndiag_family = AF_NETLINK,
		.ndiag_type = SOCK_RAW,
		.ndiag_protocol = NETLINK_ROUTE,
		.ndiag_state = NETLINK_CONNECTED,
		.ndiag_portid = 0xbadc0ded,
		.ndiag_dst_portid = 0xdeadbeef,
		.ndiag_dst_group = 0xfacefeed,
		.ndiag_ino = 0xdaeefacd,
		.ndiag_cookie = { 0xbadc0ded, 0xdeadbeef }
	};

	rc = sendto(fd, nlh, nlh->nlmsg_len, MSG_DONTWAIT, NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {ndiag_family=AF_NETLINK"
	       ", ndiag_type=SOCK_RAW, ndiag_protocol=NETLINK_ROUTE"
	       ", ndiag_state=NETLINK_CONNECTED, ndiag_portid=%u"
	       ", ndiag_dst_portid=%u, ndiag_dst_group=%u, ndiag_ino=%u"
	       ", ndiag_cookie=[%u, %u]}}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh->nlmsg_len, 0xbadc0ded, 0xdeadbeef, 0xfacefeed,
	       0xdaeefacd, 0xbadc0ded, 0xdeadbeef,
	       nlh->nlmsg_len, sprintrc(rc));

	/* short read of netlink_diag_msg */
	nlh = nlh0 - (sizeof(*msg) - 1);
	/* beware of unaligned access to nlh members */
	memmove(nlh, nlh0 - sizeof(*msg), NLMSG_HDRLEN + sizeof(*msg) - 1);

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*msg), MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}"
	       ", {ndiag_family=AF_NETLINK, %p}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*msg),
	       NLMSG_DATA(nlh) + 1,
	       NLMSG_HDRLEN + (unsigned int) sizeof(*msg),
	       sprintrc(rc));
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	int fd = create_nl_socket(NETLINK_SOCK_DIAG);

	test_nlmsg_type(fd);
	test_nlmsg_flags(fd);
	test_odd_family_req(fd);
	test_odd_family_msg(fd);
	test_unix_diag_req(fd);
	test_unix_diag_msg(fd);
	test_netlink_diag_req(fd);
	test_netlink_diag_msg(fd);

	printf("+++ exited with 0 +++\n");

	return 0;
}
