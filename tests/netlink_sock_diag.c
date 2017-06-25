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
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include "netlink.h"
#include <linux/if_ether.h>
#include <linux/inet_diag.h>
#include <linux/netlink_diag.h>
#include <linux/packet_diag.h>
#ifdef AF_SMC
# include <linux/smc_diag.h>
#endif
#include <linux/sock_diag.h>
#include <linux/unix_diag.h>

#define SMC_ACTIVE 1

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
	printf("sendto(%d, {len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}"
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
	printf("sendto(%d, {len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_REQUEST|NLM_F_DUMP, seq=0, pid=0}"
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
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*family),
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_REQUEST
	);
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
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*family) + 4,
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_REQUEST
	);
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
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*family),
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_DUMP
	);
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
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*family) + 4,
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_DUMP
	);
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
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*family),
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_REQUEST
	);
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
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*family) + 4,
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_REQUEST
	);
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
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*req),
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_REQUEST
	);
	req = NLMSG_DATA(nlh);
	*req = (struct unix_diag_req) {
		.sdiag_family = AF_UNIX,
		.sdiag_protocol = 253,
		.udiag_states = 1 << TCP_ESTABLISHED | 1 << TCP_LISTEN,
		.udiag_ino = 0xfacefeed,
		.udiag_show = UDIAG_SHOW_NAME,
		.udiag_cookie = { 0xdeadbeef, 0xbadc0ded }
	};

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*req), MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}"
	       ", {sdiag_family=AF_UNIX, sdiag_protocol=%u"
	       ", udiag_states=1<<TCP_ESTABLISHED|1<<TCP_LISTEN, udiag_ino=%u"
	       ", udiag_show=UDIAG_SHOW_NAME, udiag_cookie=[%u, %u]}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*req),
	       253, 0xfacefeed, 0xdeadbeef, 0xbadc0ded,
	       NLMSG_HDRLEN + (unsigned int) sizeof(*req),
	       sprintrc(rc));

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
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*family),
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_DUMP
	);
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
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*family) + 4,
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_DUMP
	);
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
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*msg),
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_DUMP
	);
	msg = NLMSG_DATA(nlh);
	*msg = (struct unix_diag_msg) {
		.udiag_family = AF_UNIX,
		.udiag_type = SOCK_STREAM,
		.udiag_state = TCP_FIN_WAIT1,
		.udiag_ino = 0xfacefeed,
		.udiag_cookie = { 0xdeadbeef, 0xbadc0ded }
	};

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*msg), MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}"
	       ", {udiag_family=AF_UNIX, udiag_type=SOCK_STREAM"
	       ", udiag_state=TCP_FIN_WAIT1"
	       ", udiag_ino=%u, udiag_cookie=[%u, %u]}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*msg),
	       0xfacefeed, 0xdeadbeef, 0xbadc0ded,
	       NLMSG_HDRLEN + (unsigned int) sizeof(*msg),
	       sprintrc(rc));

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
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*family),
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_REQUEST
	);
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
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*family) + 4,
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_REQUEST
	);
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
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*req),
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_REQUEST
	);
	req = NLMSG_DATA(nlh);
	*req = (struct netlink_diag_req) {
		.sdiag_family = AF_NETLINK,
		.sdiag_protocol = NDIAG_PROTO_ALL,
		.ndiag_ino = 0xfacefeed,
		.ndiag_show = NDIAG_SHOW_MEMINFO,
		.ndiag_cookie = { 0xdeadbeef, 0xbadc0ded }
	};


	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*req), MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}"
	       ", {sdiag_family=AF_NETLINK, sdiag_protocol=NDIAG_PROTO_ALL"
	       ", ndiag_ino=%u, ndiag_show=NDIAG_SHOW_MEMINFO"
	       ", ndiag_cookie=[%u, %u]}}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*req),
	       0xfacefeed, 0xdeadbeef, 0xbadc0ded,
	       NLMSG_HDRLEN + (unsigned int) sizeof(*req),
	       sprintrc(rc));

	req->sdiag_protocol = NETLINK_ROUTE;

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*req), MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}"
	       ", {sdiag_family=AF_NETLINK, sdiag_protocol=NETLINK_ROUTE"
	       ", ndiag_ino=%u, ndiag_show=NDIAG_SHOW_MEMINFO"
	       ", ndiag_cookie=[%u, %u]}}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*req),
	       0xfacefeed, 0xdeadbeef, 0xbadc0ded,
	       NLMSG_HDRLEN + (unsigned int) sizeof(*req),
	       sprintrc(rc));

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
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*family),
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_DUMP
	);
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
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*family) + 4,
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_DUMP
	);
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
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*msg),
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_DUMP
	);
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

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*msg), MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {ndiag_family=AF_NETLINK"
	       ", ndiag_type=SOCK_RAW, ndiag_protocol=NETLINK_ROUTE"
	       ", ndiag_state=NETLINK_CONNECTED, ndiag_portid=%u"
	       ", ndiag_dst_portid=%u, ndiag_dst_group=%u, ndiag_ino=%u"
	       ", ndiag_cookie=[%u, %u]}}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*msg),
	       0xbadc0ded, 0xdeadbeef, 0xfacefeed,
	       0xdaeefacd, 0xbadc0ded, 0xdeadbeef,
	       NLMSG_HDRLEN + (unsigned int) sizeof(*msg),
	       sprintrc(rc));

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

static void
test_packet_diag_req(const int fd)
{
	struct nlmsghdr *nlh;
	struct packet_diag_req *req;
	uint8_t *family;
	void *const nlh0 = tail_alloc(NLMSG_HDRLEN);
	long rc;

	/* family only */
	nlh = nlh0 - sizeof(*family);
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*family),
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_REQUEST
	);
	family = NLMSG_DATA(nlh);
	*family = AF_PACKET;

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*family), MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}, {family=AF_PACKET}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*family),
	       NLMSG_HDRLEN + (unsigned int) sizeof(*family),
	       sprintrc(rc));

	/* family and string */
	nlh = nlh0 - (sizeof(*family) + 4);
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*family) + 4,
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_REQUEST
	);
	family = NLMSG_DATA(nlh);
	*family = AF_PACKET;
	memcpy(family + 1, "1234", 4);

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*family) + 4, MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}"
	       ", {sdiag_family=AF_PACKET, ...}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*family) + 4,
	       NLMSG_HDRLEN + (unsigned int) sizeof(*family) + 4,
	       sprintrc(rc));

	/* packet_diag_req */
	nlh = nlh0 - sizeof(*req);
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*req),
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_REQUEST
	);
	req = NLMSG_DATA(nlh);
	*req = (struct packet_diag_req) {
		.sdiag_family = AF_PACKET,
		.sdiag_protocol = ETH_P_LOOP,
		.pdiag_ino = 0xfacefeed,
		.pdiag_show = PACKET_SHOW_INFO,
		.pdiag_cookie = { 0xdeadbeef, 0xbadc0ded }
	};

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*req), MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}"
	       ", {sdiag_family=AF_PACKET, sdiag_protocol=ETH_P_LOOP"
	       ", pdiag_ino=%u, pdiag_show=PACKET_SHOW_INFO"
	       ", pdiag_cookie=[%u, %u]}}, %u"
	       ", MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*req),
	       0xfacefeed, 0xdeadbeef, 0xbadc0ded,
	       NLMSG_HDRLEN + (unsigned int) sizeof(*req),
	       sprintrc(rc));

	/* short read of packet_diag_req */
	nlh = nlh0 - (sizeof(*req) - 1);
	memmove(nlh, nlh0 - sizeof(*req), NLMSG_HDRLEN + sizeof(*req) - 1);

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*req), MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}"
	       ", {sdiag_family=AF_PACKET, %p}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*req),
	       NLMSG_DATA(nlh) + 1,
	       NLMSG_HDRLEN + (unsigned int) sizeof(*req),
	       sprintrc(rc));
}

static void
test_packet_diag_msg(const int fd)
{
	struct nlmsghdr *nlh;
	struct packet_diag_msg *msg;
	uint8_t *family;
	void *const nlh0 = tail_alloc(NLMSG_HDRLEN);
	long rc;

	/* family only */
	nlh = nlh0 - sizeof(*family);
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*family),
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_DUMP
	);
	family = NLMSG_DATA(nlh);
	*family = AF_PACKET;

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*family), MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {family=AF_PACKET}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*family),
	       NLMSG_HDRLEN + (unsigned int) sizeof(*family),
	       sprintrc(rc));

	/* family and string */
	nlh = nlh0 - (sizeof(*family) + 4);
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*family) + 4,
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_DUMP
	);
	family = NLMSG_DATA(nlh);
	*family = AF_PACKET;
	memcpy(family + 1, "1234", 4);

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*family) + 4, MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}"
	       ", {pdiag_family=AF_PACKET, ...}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*family) + 4,
	       NLMSG_HDRLEN + (unsigned int) sizeof(*family) + 4,
	       sprintrc(rc));

	/* packet_diag_msg */
	nlh = nlh0 - sizeof(*msg);
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*msg),
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_DUMP
	);
	msg = NLMSG_DATA(nlh);
	*msg = (struct packet_diag_msg) {
		.pdiag_family = AF_PACKET,
		.pdiag_type = SOCK_STREAM,
		.pdiag_num = 0xbadc,
		.pdiag_ino = 0xfacefeed,
		.pdiag_cookie = { 0xdeadbeef, 0xbadc0ded }
	};

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*msg), MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}"
	       ", {pdiag_family=AF_PACKET, pdiag_type=SOCK_STREAM"
	       ", pdiag_num=%u, pdiag_ino=%u, pdiag_cookie=[%u, %u]}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*msg),
	       0xbadc, 0xfacefeed, 0xdeadbeef, 0xbadc0ded,
	       NLMSG_HDRLEN + (unsigned int) sizeof(*msg),
	       sprintrc(rc));

	/* short read of packet_diag_msg */
	nlh = nlh0 - (sizeof(*msg) - 1);
	memmove(nlh, nlh0 - sizeof(*msg), NLMSG_HDRLEN + sizeof(*msg) - 1);

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*msg), MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}"
	       ", {pdiag_family=AF_PACKET, %p}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*msg),
	       NLMSG_DATA(nlh) + 1,
	       NLMSG_HDRLEN + (unsigned int) sizeof(*msg),
	       sprintrc(rc));
}

static void
test_inet_diag_sockid(const int fd)
{
	const char address[] = "12.34.56.78";
	const char address6[] = "12:34:56:78:90:ab:cd:ef";
	struct nlmsghdr *nlh;
	struct inet_diag_req_v2 *req;
	void *const nlh0 = tail_alloc(NLMSG_HDRLEN);
	long rc;

	nlh = nlh0 - sizeof(*req);
	/* beware of unaligned access to nlh members */
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*req),
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_REQUEST
	);

	req = NLMSG_DATA(nlh);
	*req = (struct inet_diag_req_v2) {
		.sdiag_family = AF_INET,
		.idiag_ext = 1 << (INET_DIAG_CONG - 1),
		.sdiag_protocol = IPPROTO_TCP,
		.idiag_states = 1 << TCP_CLOSE,
		.id = {
			.idiag_sport = 0xfacd,
			.idiag_dport = 0xdead,
			.idiag_if = 0xadcdfafc,
			.idiag_cookie = { 0xdeadbeef, 0xbadc0ded }
		},
	};

	if (!inet_pton(AF_INET, address, &req->id.idiag_src))
		perror_msg_and_skip("sendto");
	if (!inet_pton(AF_INET, address, &req->id.idiag_dst))
		perror_msg_and_skip("sendto");

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*req), MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}"
	       ", {sdiag_family=AF_INET, sdiag_protocol=IPPROTO_TCP"
	       ", idiag_ext=1<<(INET_DIAG_CONG-1)"
	       ", idiag_states=1<<TCP_CLOSE, id={idiag_sport=htons(%u)"
	       ", idiag_dport=htons(%u), inet_pton(AF_INET, \"%s\", &idiag_src)"
	       ", inet_pton(AF_INET, \"%s\", &idiag_dst), idiag_if=%u"
	       ", idiag_cookie=[%u, %u]}}}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*req),
	       ntohs(0xfacd), ntohs(0xdead), address, address,
	       0xadcdfafc, 0xdeadbeef, 0xbadc0ded,
	       NLMSG_HDRLEN + (unsigned int) sizeof(*req),
	       sprintrc(rc));

	req->sdiag_family = AF_INET6;
	if (!inet_pton(AF_INET6, address6, &req->id.idiag_src))
		perror_msg_and_skip("sendto");
	if (!inet_pton(AF_INET6, address6, &req->id.idiag_dst))
		perror_msg_and_skip("sendto");

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*req), MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}"
	       ", {sdiag_family=AF_INET6, sdiag_protocol=IPPROTO_TCP"
	       ", idiag_ext=1<<(INET_DIAG_CONG-1)"
	       ", idiag_states=1<<TCP_CLOSE, id={idiag_sport=htons(%u)"
	       ", idiag_dport=htons(%u), inet_pton(AF_INET6, \"%s\", &idiag_src)"
	       ", inet_pton(AF_INET6, \"%s\", &idiag_dst), idiag_if=%u"
	       ", idiag_cookie=[%u, %u]}}}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*req),
	       ntohs(0xfacd), ntohs(0xdead), address6, address6,
	       0xadcdfafc, 0xdeadbeef, 0xbadc0ded,
	       NLMSG_HDRLEN + (unsigned int) sizeof(*req),
	       sprintrc(rc));
}

static void
test_inet_diag_req(const int fd)
{
	const char address[] = "12.34.56.78";
	struct nlmsghdr *nlh;
	struct inet_diag_req *req;
	uint8_t *family;
	void *const nlh0 = tail_alloc(NLMSG_HDRLEN);
	long rc;

	/* family only */
	nlh = nlh0 - sizeof(*family);
	/* beware of unaligned access to nlh members */
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*family),
		.nlmsg_type = TCPDIAG_GETSOCK,
		.nlmsg_flags = NLM_F_REQUEST
	);

	family = NLMSG_DATA(nlh);
	*family = AF_INET;

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*family), MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=TCPDIAG_GETSOCK"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}, {family=AF_INET}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*family),
	       NLMSG_HDRLEN + (unsigned int) sizeof(*family),
	       sprintrc(rc));

	/* family and string */
	nlh = nlh0 - (sizeof(*family) + 4);
	/* beware of unaligned access to nlh members */
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*family) + 4,
		.nlmsg_type = TCPDIAG_GETSOCK,
		.nlmsg_flags = NLM_F_REQUEST
	);

	family = NLMSG_DATA(nlh);
	*family = AF_INET;
	memcpy(family + 1, "1234", 4);

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*family) + 4, MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=TCPDIAG_GETSOCK"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}"
	       ", {idiag_family=AF_INET, ...}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*family) + 4,
	       NLMSG_HDRLEN + (unsigned int) sizeof(*family) + 4,
	       sprintrc(rc));

	/* inet_diag_req */
	nlh = nlh0 - sizeof(*req);
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*req),
		.nlmsg_type = TCPDIAG_GETSOCK,
		.nlmsg_flags = NLM_F_REQUEST
	);

	req = NLMSG_DATA(nlh);
	*req = (struct inet_diag_req) {
		.idiag_family = AF_INET,
		.idiag_ext = 1 << (INET_DIAG_TOS - 1),
		.idiag_src_len = 0xde,
		.idiag_dst_len = 0xba,
		.id = {
			.idiag_sport = 0xdead,
			.idiag_dport = 0xadcd,
			.idiag_if = 0xadcdfafc,
			.idiag_cookie = { 0xdeadbeef, 0xbadc0ded }
		},
		.idiag_states = 1 << TCP_LAST_ACK,
		.idiag_dbs = 0xfacefeed,
	};

	if (!inet_pton(AF_INET, address, &req->id.idiag_src))
		perror_msg_and_skip("sendto");
	if (!inet_pton(AF_INET, address, &req->id.idiag_dst))
		perror_msg_and_skip("sendto");

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*req), MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=TCPDIAG_GETSOCK"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}"
	       ", {idiag_family=AF_INET, idiag_src_len=%u"
	       ", idiag_dst_len=%u, idiag_ext=1<<(INET_DIAG_TOS-1)"
	       ", id={idiag_sport=htons(%u), idiag_dport=htons(%u)"
	       ", inet_pton(AF_INET, \"%s\", &idiag_src)"
	       ", inet_pton(AF_INET, \"%s\", &idiag_dst)"
	       ", idiag_if=%u, idiag_cookie=[%u, %u]}"
	       ", idiag_states=1<<TCP_LAST_ACK, idiag_dbs=%u}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*req),
	       0xde, 0xba, ntohs(0xdead), ntohs(0xadcd), address, address,
	       0xadcdfafc, 0xdeadbeef, 0xbadc0ded, 0xfacefeed,
	       NLMSG_HDRLEN + (unsigned int) sizeof(*req),
	       sprintrc(rc));

	/* short read of inet_diag_req */
	nlh = nlh0 - (sizeof(*req) - 1);
	/* beware of unaligned access to nlh members */
	memmove(nlh, nlh0 - sizeof(*req), NLMSG_HDRLEN + sizeof(*req) - 1);

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*req), MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=TCPDIAG_GETSOCK"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}"
	       ", {idiag_family=AF_INET, %p}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*req),
	       NLMSG_DATA(nlh) + 1,
	       NLMSG_HDRLEN + (unsigned int) sizeof(*req),
	       sprintrc(rc));
}

static void
test_inet_diag_req_v2(const int fd)
{
	const char address[] = "87.65.43.21";
	struct nlmsghdr *nlh;
	struct inet_diag_req_v2 *req;
	uint8_t *family;
	void *const nlh0 = tail_alloc(NLMSG_HDRLEN);
	long rc;

	/* family only */
	nlh = nlh0 - sizeof(*family);
	/* beware of unaligned access to nlh members */
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*family),
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_REQUEST
	);

	family = NLMSG_DATA(nlh);
	*family = AF_INET;

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*family), MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}, {family=AF_INET}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*family),
	       NLMSG_HDRLEN + (unsigned int) sizeof(*family),
	       sprintrc(rc));

	/* family and string */
	nlh = nlh0 - sizeof(*family) - 4;
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*family) + 4,
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_REQUEST
	);

	family = NLMSG_DATA(nlh);
	*family = AF_INET;
	memcpy(family + 1, "1234", 4);

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*family) + 4, MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}"
	       ", {sdiag_family=AF_INET, ...}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*family) + 4,
	       NLMSG_HDRLEN + (unsigned int) sizeof(*family) + 4,
	       sprintrc(rc));

	/* inet_diag_req_v2 */
	nlh = nlh0 - sizeof(*req);
	/* beware of unaligned access to nlh members */
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*req),
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_REQUEST
	);

	req = NLMSG_DATA(nlh);
	*req = (struct inet_diag_req_v2) {
		.sdiag_family = AF_INET,
		.idiag_ext = 1 << (INET_DIAG_CONG - 1),
		.sdiag_protocol = IPPROTO_TCP,
		.idiag_states = 1 << TCP_CLOSE,
		.id = {
			.idiag_sport = 0xfacd,
			.idiag_dport = 0xdead,
			.idiag_if = 0xadcdfafc,
			.idiag_cookie = { 0xdeadbeef, 0xbadc0ded }
		},
	};

	if (!inet_pton(AF_INET, address, &req->id.idiag_src))
		perror_msg_and_skip("sendto");
	if (!inet_pton(AF_INET, address, &req->id.idiag_dst))
		perror_msg_and_skip("sendto");

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*req), MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}"
	       ", {sdiag_family=AF_INET, sdiag_protocol=IPPROTO_TCP"
	       ", idiag_ext=1<<(INET_DIAG_CONG-1)"
	       ", idiag_states=1<<TCP_CLOSE, id={idiag_sport=htons(%u)"
	       ", idiag_dport=htons(%u), inet_pton(AF_INET, \"%s\", &idiag_src)"
	       ", inet_pton(AF_INET, \"%s\", &idiag_dst), idiag_if=%u"
	       ", idiag_cookie=[%u, %u]}}}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*req),
	       ntohs(0xfacd), ntohs(0xdead), address, address,
	       0xadcdfafc, 0xdeadbeef, 0xbadc0ded,
	       NLMSG_HDRLEN + (unsigned int) sizeof(*req),
	       sprintrc(rc));

	/* short read of inet_diag_req_v2 */
	nlh = nlh0 - (sizeof(*req) - 1);
	/* beware of unaligned access to nlh members */
	memmove(nlh, nlh0 - sizeof(*req), NLMSG_HDRLEN + sizeof(*req) - 1);

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*req), MSG_DONTWAIT,
		    NULL, 0);
	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}"
	       ", {sdiag_family=AF_INET, %p}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*req),
	       NLMSG_DATA(nlh) + 1,
	       NLMSG_HDRLEN + (unsigned int) sizeof(*req),
	       sprintrc(rc));
}

static void
test_inet_diag_msg(const int fd)
{
	const char address[] = "11.22.33.44";
	struct nlmsghdr *nlh;
	struct inet_diag_msg *msg;
	uint8_t *family;
	void *const nlh0 = tail_alloc(NLMSG_HDRLEN);
	long rc;

	/* family only */
	nlh = nlh0 - sizeof(*family);
	/* beware of unaligned access to nlh members */
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*family),
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_DUMP
	);

	family = NLMSG_DATA(nlh);
	*family = AF_INET;

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*family), MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {family=AF_INET}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*family),
	       NLMSG_HDRLEN + (unsigned int) sizeof(*family),
	       sprintrc(rc));

	/* family and string */
	nlh = nlh0 - sizeof(*family) - 4;
	/* beware of unaligned access to nlh members */
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*family) + 4,
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_DUMP
	);

	family = NLMSG_DATA(nlh);
	*family = AF_INET;
	memcpy(family + 1, "1234", 4);

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*family) + 4, MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}"
	       ", {idiag_family=AF_INET, ...}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*family) + 4,
	       NLMSG_HDRLEN + (unsigned int) sizeof(*family) + 4,
	       sprintrc(rc));

	/* inet_diag_msg */
	nlh = nlh0 - sizeof(*msg);
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*msg),
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_DUMP
	);

	msg = NLMSG_DATA(nlh);
	*msg = (struct inet_diag_msg) {
		.idiag_family = AF_INET,
		.idiag_state = TCP_LISTEN,
		.idiag_timer = 0xfa,
		.idiag_retrans = 0xde,
		.id = {
			.idiag_sport = 0xfacf,
			.idiag_dport = 0xdead,
			.idiag_if = 0xadcdfafc,
			.idiag_cookie = { 0xdeadbeef, 0xbadc0ded }
		},
		.idiag_expires = 0xfacefeed,
		.idiag_rqueue = 0xdeadbeef,
		.idiag_wqueue = 0xadcdfafc,
		.idiag_uid = 0xdecefaeb,
		.idiag_inode = 0xbadc0ded,
	};

	if (!inet_pton(AF_INET, address, &msg->id.idiag_src))
		perror_msg_and_skip("sendto");
	if (!inet_pton(AF_INET, address, &msg->id.idiag_dst))
		perror_msg_and_skip("sendto");

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*msg), MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}"
	       ", {idiag_family=AF_INET, idiag_state=TCP_LISTEN"
	       ", idiag_timer=%u, idiag_retrans=%u"
	       ", id={idiag_sport=htons(%u), idiag_dport=htons(%u)"
	       ", inet_pton(AF_INET, \"%s\", &idiag_src)"
	       ", inet_pton(AF_INET, \"%s\", &idiag_dst)"
	       ", idiag_if=%u, idiag_cookie=[%u, %u]}"
	       ", idiag_expires=%u, idiag_rqueue=%u, idiag_wqueue=%u"
	       ", idiag_uid=%u, idiag_inode=%u}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*msg),
	       0xfa, 0xde, ntohs(0xfacf), ntohs(0xdead),
	       address, address, 0xadcdfafc, 0xdeadbeef, 0xbadc0ded,
	       0xfacefeed, 0xdeadbeef, 0xadcdfafc, 0xdecefaeb, 0xbadc0ded,
	       NLMSG_HDRLEN + (unsigned int) sizeof(*msg),
	       sprintrc(rc));

	/* short read of inet_diag_msg */
	nlh = nlh0 - (sizeof(*msg) - 1);
	/* beware of unaligned access to nlh members */
	memmove(nlh, nlh0 - sizeof(*msg), NLMSG_HDRLEN + sizeof(*msg) - 1);

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*msg), MSG_DONTWAIT,
		    NULL, 0);
	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}"
	       ", {idiag_family=AF_INET, %p}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*msg),
	       NLMSG_DATA(nlh) + 1,
	       NLMSG_HDRLEN + (unsigned int) sizeof(*msg),
	       sprintrc(rc));
}

#ifdef AF_SMC
static void
test_smc_diag_req(const int fd)
{
	const char address[] = "43.21.56.78";
	struct nlmsghdr *nlh;
	struct smc_diag_req *req;
	uint8_t *family;
	void *const nlh0 = tail_alloc(NLMSG_HDRLEN);
	long rc;

	/* family only */
	nlh = nlh0 - sizeof(*family);
	/* beware of unaligned access to nlh members */
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*family),
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_REQUEST
	);

	family = NLMSG_DATA(nlh);
	*family = AF_SMC;

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*family), MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}, {family=AF_SMC}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*family),
	       NLMSG_HDRLEN + (unsigned int) sizeof(*family),
	       sprintrc(rc));

	/* family and string */
	nlh = nlh0 - sizeof(*family) - 4;
	/* beware of unaligned access to nlh members */
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*family) + 4,
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_REQUEST
	);

	family = NLMSG_DATA(nlh);
	*family = AF_SMC;
	memcpy(family + 1, "1234", 4);

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*family) + 4, MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}"
	       ", {diag_family=AF_SMC, ...}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*family) + 4,
	       NLMSG_HDRLEN + (unsigned int) sizeof(*family) + 4,
	       sprintrc(rc));

	/* smc_diag_req */
	nlh = nlh0 - sizeof(*req);
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*req),
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_REQUEST
	);

	req = NLMSG_DATA(nlh);
	*req = (struct smc_diag_req) {
		.diag_family = AF_SMC,
		.diag_ext = 1 << (SMC_DIAG_CONNINFO - 1),
		.id = {
			.idiag_sport = 0xdead,
			.idiag_dport = 0xadcd,
			.idiag_if = 0xadcdfafc,
			.idiag_cookie = { 0xdeadbeef, 0xbadc0ded },
		},
	};

	if (!inet_pton(AF_INET, address, &req->id.idiag_src))
		perror_msg_and_skip("sendto");
	if (!inet_pton(AF_INET, address, &req->id.idiag_dst))
		perror_msg_and_skip("sendto");

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*req), MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}, {diag_family=AF_SMC"
	       ", diag_ext=1<<(SMC_DIAG_CONNINFO-1)"
	       ", id={idiag_sport=htons(%u), idiag_dport=htons(%u)"
	       ", inet_pton(AF_INET, \"%s\", &idiag_src)"
	       ", inet_pton(AF_INET, \"%s\", &idiag_dst)"
	       ", idiag_if=%u, idiag_cookie=[%u, %u]}}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*req),
	       htons(0xdead), htons(0xadcd), address, address,
	       0xadcdfafc, 0xdeadbeef, 0xbadc0ded,
	       NLMSG_HDRLEN + (unsigned int) sizeof(*req),
	       sprintrc(rc));

	/* short read of smc_diag_req */
	nlh = nlh0 - (sizeof(*req) - 1);
	/* beware of unaligned access to nlh members */
	memmove(nlh, nlh0 - sizeof(*req), NLMSG_HDRLEN + sizeof(*req) - 1);

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*req), MSG_DONTWAIT,
		    NULL, 0);
	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}"
	       ", {diag_family=AF_SMC, %p}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*req),
	       NLMSG_DATA(nlh) + 1,
	       NLMSG_HDRLEN + (unsigned int) sizeof(*req),
	       sprintrc(rc));
}

static void
test_smc_diag_msg(const int fd)
{
	const char address[] = "34.87.12.90";
	struct nlmsghdr *nlh;
	struct smc_diag_msg *msg;
	uint8_t *family;
	void *const nlh0 = tail_alloc(NLMSG_HDRLEN);
	long rc;

	/* family only */
	nlh = nlh0 - sizeof(*family);
	/* beware of unaligned access to nlh members */
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*family),
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_DUMP
	);

	family = NLMSG_DATA(nlh);
	*family = AF_SMC;

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*family), MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {family=AF_SMC}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*family),
	       NLMSG_HDRLEN + (unsigned int) sizeof(*family),
	       sprintrc(rc));

	/* family and string */
	nlh = nlh0 - sizeof(*family) - 4;
	/* beware of unaligned access to nlh members */
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*family) + 4,
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_DUMP
	);

	family = NLMSG_DATA(nlh);
	*family = AF_SMC;
	memcpy(family + 1, "1234", 4);

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*family) + 4, MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}"
	       ", {diag_family=AF_SMC, ...}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*family) + 4,
	       NLMSG_HDRLEN + (unsigned int) sizeof(*family) + 4,
	       sprintrc(rc));

	/* smc_diag_msg */
	nlh = nlh0 - sizeof(*msg);
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = NLMSG_HDRLEN + sizeof(*msg),
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_DUMP
	);

	msg = NLMSG_DATA(nlh);
	*msg = (struct smc_diag_msg) {
		.diag_family = AF_SMC,
		.diag_state = SMC_ACTIVE,
		.diag_fallback = 0xde,
		.diag_shutdown = 0xba,
		.id = {
			.idiag_sport = 0xdead,
			.idiag_dport = 0xadcd,
			.idiag_if = 0xadcdfafc,
			.idiag_cookie = { 0xdeadbeef, 0xbadc0ded },
		},
		.diag_uid = 0xadcdfafc,
		.diag_inode = 0xbadc0ded,
	};

	if (!inet_pton(AF_INET, address, &msg->id.idiag_src))
		perror_msg_and_skip("sendto");
	if (!inet_pton(AF_INET, address, &msg->id.idiag_dst))
		perror_msg_and_skip("sendto");

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*msg), MSG_DONTWAIT,
		    NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {diag_family=AF_SMC"
	       ", diag_state=SMC_ACTIVE, diag_fallback=%u, diag_shutdown=%u"
	       ", id={idiag_sport=htons(%u), idiag_dport=htons(%u)"
	       ", inet_pton(AF_INET, \"%s\", &idiag_src)"
	       ", inet_pton(AF_INET, \"%s\", &idiag_dst)"
	       ", idiag_if=%u, idiag_cookie=[%u, %u]}"
	       ", diag_uid=%u, diag_inode=%u}}, %u"
	       ", MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*msg),
	       0xde, 0xba, htons(0xdead), htons(0xadcd), address, address,
	       0xadcdfafc, 0xdeadbeef, 0xbadc0ded, 0xadcdfafc, 0xbadc0ded,
	       NLMSG_HDRLEN + (unsigned int) sizeof(*msg),
	       sprintrc(rc));

	/* short read of smc_diag_msg */
	nlh = nlh0 - (sizeof(*msg) - 1);
	/* beware of unaligned access to nlh members */
	memmove(nlh, nlh0 - sizeof(*msg), NLMSG_HDRLEN + sizeof(*msg) - 1);

	rc = sendto(fd, nlh, NLMSG_HDRLEN + sizeof(*msg), MSG_DONTWAIT,
		    NULL, 0);
	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}"
	       ", {diag_family=AF_SMC, %p}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + (unsigned int) sizeof(*msg),
	       NLMSG_DATA(nlh) + 1,
	       NLMSG_HDRLEN + (unsigned int) sizeof(*msg),
	       sprintrc(rc));
}
#endif

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
	test_packet_diag_req(fd);
	test_packet_diag_msg(fd);
	test_inet_diag_sockid(fd);
	test_inet_diag_req(fd);
	test_inet_diag_req_v2(fd);
	test_inet_diag_msg(fd);
#ifdef AF_SMC
	test_smc_diag_req(fd);
	test_smc_diag_msg(fd);
#endif

	printf("+++ exited with 0 +++\n");

	return 0;
}
