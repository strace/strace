/*
 * Check decoding of netlink protocol.
 *
 * Copyright (c) 2014-2017 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016 Fabien Siron <fabien.siron@epita.fr>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#ifdef HAVE_SYS_XATTR_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <sys/xattr.h>
# include <netinet/in.h>
# include "netlink.h"
# include <linux/sock_diag.h>
# include <linux/netlink_diag.h>
# include "xmalloc.h"

static void
send_query(const int fd)
{
	static const struct req {
		struct nlmsghdr nlh;
		const char magic[4];
	} c_req = {
		.nlh = {
			.nlmsg_len = sizeof(struct req),
			.nlmsg_type = NLMSG_NOOP,
			.nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST
		},
		.magic = "abcd"
	};
	struct req *const req = tail_memdup(&c_req, sizeof(c_req));
	long rc;
	const char *errstr;

	/* zero address */
	rc = sendto(fd, NULL, sizeof(*req), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, NULL, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, (unsigned) sizeof(*req), sprintrc(rc));

	/* zero length */
	rc = sendto(fd, req, 0, MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, \"\", 0, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, sprintrc(rc));

	/* zero address and length */
	rc = sendto(fd, NULL, 0, MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, NULL, 0, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, sprintrc(rc));

	/* unfetchable struct nlmsghdr */
	const void *const efault = tail_alloc(sizeof(struct nlmsghdr) - 1);
	rc = sendto(fd, efault, sizeof(struct nlmsghdr), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, %p, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, efault, (unsigned) sizeof(struct nlmsghdr), sprintrc(rc));

	/* whole message length < sizeof(struct nlmsghdr) */
	rc = sendto(fd, req->magic, sizeof(req->magic), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, \"\\x61\\x62\\x63\\x64\""
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, (unsigned) sizeof(req->magic), sprintrc(rc));

	/* a single message with some data */
	rc = sendto(fd, req, sizeof(*req), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, [{nlmsg_len=%u, nlmsg_type=NLMSG_NOOP"
	       ", nlmsg_flags=NLM_F_REQUEST|0x%x"
	       ", nlmsg_seq=0, nlmsg_pid=0}, \"\\x61\\x62\\x63\\x64\"]"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, req->nlh.nlmsg_len, NLM_F_DUMP,
	       (unsigned) sizeof(*req), sprintrc(rc));

	/* a single message without data */
	req->nlh.nlmsg_len = sizeof(req->nlh);
	rc = sendto(fd, &req->nlh, sizeof(req->nlh), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {nlmsg_len=%u, nlmsg_type=NLMSG_NOOP"
	       ", nlmsg_flags=NLM_F_REQUEST|0x%x"
	       ", nlmsg_seq=0, nlmsg_pid=0}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, req->nlh.nlmsg_len, NLM_F_DUMP,
	       (unsigned) sizeof(req->nlh), sprintrc(rc));

	/* nlmsg_len > whole message length */
	req->nlh.nlmsg_len = sizeof(*req) + 8;
	rc = sendto(fd, req, sizeof(*req), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, [{nlmsg_len=%u, nlmsg_type=NLMSG_NOOP"
	       ", nlmsg_flags=NLM_F_REQUEST|0x%x"
	       ", nlmsg_seq=0, nlmsg_pid=0}, \"\\x61\\x62\\x63\\x64\"]"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, req->nlh.nlmsg_len, NLM_F_DUMP,
	       (unsigned) sizeof(*req), sprintrc(rc));

	/* nlmsg_len < sizeof(struct nlmsghdr) */
	req->nlh.nlmsg_len = 8;
	rc = sendto(fd, req, sizeof(*req), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {nlmsg_len=%u, nlmsg_type=NLMSG_NOOP"
	       ", nlmsg_flags=NLM_F_REQUEST|0x%x"
	       ", nlmsg_seq=0, nlmsg_pid=0}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, req->nlh.nlmsg_len, NLM_F_DUMP,
	       (unsigned) sizeof(*req), sprintrc(rc));

	/* a sequence of two nlmsg objects */
	struct reqs {
		struct req req1;
		char padding[NLMSG_ALIGN(sizeof(struct req)) - sizeof(struct req)];
		struct req req2;
	};
	TAIL_ALLOC_OBJECT_CONST_PTR(struct reqs, reqs);
	memcpy(&reqs->req1, &c_req, sizeof(c_req));
	memcpy(&reqs->req2, &c_req, sizeof(c_req));

	rc = sendto(fd, reqs, sizeof(*reqs), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, [[{nlmsg_len=%u, nlmsg_type=NLMSG_NOOP"
	       ", nlmsg_flags=NLM_F_REQUEST|0x%x"
	       ", nlmsg_seq=0, nlmsg_pid=0}, \"\\x61\\x62\\x63\\x64\"]"
	       ", [{nlmsg_len=%u, nlmsg_type=NLMSG_NOOP"
	       ", nlmsg_flags=NLM_F_REQUEST|0x%x"
	       ", nlmsg_seq=0, nlmsg_pid=0}, \"\\x61\\x62\\x63\\x64\"]]"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, reqs->req1.nlh.nlmsg_len, NLM_F_DUMP,
	       reqs->req2.nlh.nlmsg_len, NLM_F_DUMP,
	       (unsigned) sizeof(*reqs), sprintrc(rc));

	/* unfetchable second struct nlmsghdr */
	void *const efault2 = tail_memdup(&reqs->req1, sizeof(reqs->req1));
	rc = sendto(fd, efault2, sizeof(*reqs), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, [[{nlmsg_len=%u, nlmsg_type=NLMSG_NOOP"
	       ", nlmsg_flags=NLM_F_REQUEST|0x%x"
	       ", nlmsg_seq=0, nlmsg_pid=0}, \"\\x61\\x62\\x63\\x64\"]"
	       ", ... /* %p */], %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, reqs->req1.nlh.nlmsg_len, NLM_F_DUMP,
	       &((struct reqs *) efault2)->req2, (unsigned) sizeof(*reqs),
	       sprintrc(rc));

	/* message length is not enough for the second struct nlmsghdr */
	rc = sendto(fd, reqs, sizeof(*reqs) - sizeof(req->nlh), MSG_DONTWAIT,
		    NULL, 0);
	errstr = sprintrc(rc);
	printf("sendto(%d, [[{nlmsg_len=%u, nlmsg_type=NLMSG_NOOP"
	       ", nlmsg_flags=NLM_F_REQUEST|0x%x"
	       ", nlmsg_seq=0, nlmsg_pid=0}, \"\\x61\\x62\\x63\\x64\"], ",
	       fd, reqs->req1.nlh.nlmsg_len, NLM_F_DUMP);
	print_quoted_hex(&reqs->req2.nlh,
			 sizeof(reqs->req2) - sizeof(req->nlh));
	printf("], %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       (unsigned) (sizeof(*reqs) - sizeof(req->nlh)), errstr);

	/* second nlmsg_len < sizeof(struct nlmsghdr) */
	reqs->req2.nlh.nlmsg_len = 4;
	rc = sendto(fd, reqs, sizeof(*reqs), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, [[{nlmsg_len=%u, nlmsg_type=NLMSG_NOOP"
	       ", nlmsg_flags=NLM_F_REQUEST|0x%x"
	       ", nlmsg_seq=0, nlmsg_pid=0}, \"\\x61\\x62\\x63\\x64\"]"
	       ", {nlmsg_len=%u, nlmsg_type=NLMSG_NOOP"
	       ", nlmsg_flags=NLM_F_REQUEST|0x%x"
	       ", nlmsg_seq=0, nlmsg_pid=0}], %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, reqs->req1.nlh.nlmsg_len, NLM_F_DUMP,
	       reqs->req2.nlh.nlmsg_len, NLM_F_DUMP,
	       (unsigned) sizeof(*reqs), sprintrc(rc));

	/* abbreviated output */
# define ABBREV_LEN (DEFAULT_STRLEN + 1)
	const unsigned int msg_len = sizeof(struct nlmsghdr) * ABBREV_LEN;
	struct nlmsghdr *const msgs = tail_alloc(msg_len);
	for (unsigned int i = 0; i < ABBREV_LEN; ++i) {
		msgs[i].nlmsg_len = sizeof(*msgs);
		msgs[i].nlmsg_type = NLMSG_NOOP;
		msgs[i].nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST;
		msgs[i].nlmsg_seq = i;
		msgs[i].nlmsg_pid = 0;
	}

	rc = sendto(fd, msgs, msg_len, MSG_DONTWAIT, NULL, 0);
	errstr = sprintrc(rc);
	printf("sendto(%d, [", fd);
	for (unsigned int i = 0; i < DEFAULT_STRLEN; ++i) {
		if (i)
			printf(", ");
		printf("{nlmsg_len=%u, nlmsg_type=NLMSG_NOOP"
		       ", nlmsg_flags=NLM_F_REQUEST|0x%x"
		       ", nlmsg_seq=%u, nlmsg_pid=0}",
		       msgs[i].nlmsg_len, NLM_F_DUMP, msgs[i].nlmsg_seq);
	}
	printf(", ...], %u, MSG_DONTWAIT, NULL, 0) = %s\n", msg_len, errstr);
}

static void
test_nlmsgerr(const int fd)
{
	struct nlmsgerr *err;
	struct nlmsghdr *nlh;
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(*err) + 4);
	long rc;
	const char *rcstr;

	/* error message with nlmsg_len exceeding the allocated room */
	nlh = nlh0;
	nlh->nlmsg_len = NLMSG_HDRLEN + 4;
	nlh->nlmsg_type = NLMSG_ERROR;
	nlh->nlmsg_flags = NLM_F_REQUEST;
	nlh->nlmsg_seq = 0;
	nlh->nlmsg_pid = 0;

	rc = sendto(fd, nlh, nlh->nlmsg_len, MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, [{nlmsg_len=%u, nlmsg_type=NLMSG_ERROR"
	       ", nlmsg_flags=NLM_F_REQUEST, nlmsg_seq=0, nlmsg_pid=0}"
	       ", %p], %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh->nlmsg_len, nlh0 + NLMSG_HDRLEN,
	       nlh->nlmsg_len, sprintrc(rc));

	/* error message without enough room for the error code */
	nlh->nlmsg_len = NLMSG_HDRLEN + 2;
	nlh = nlh0 - 2;
	memmove(nlh, nlh0, sizeof(*nlh));
	memcpy(NLMSG_DATA(nlh), "42", 2);

	rc = sendto(fd, nlh, NLMSG_HDRLEN + 2, MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, [{nlmsg_len=%u, nlmsg_type=NLMSG_ERROR"
	       ", nlmsg_flags=NLM_F_REQUEST, nlmsg_seq=0, nlmsg_pid=0}"
	       ", \"\\x34\\x32\"], %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + 2, NLMSG_HDRLEN + 2, sprintrc(rc));

	/* error message with room for the error code only */
	nlh = nlh0 - sizeof(err->error);
	nlh->nlmsg_len = NLMSG_HDRLEN + sizeof(err->error);
	nlh->nlmsg_type = NLMSG_ERROR;
	nlh->nlmsg_flags = NLM_F_REQUEST;
	nlh->nlmsg_seq = 0;
	nlh->nlmsg_pid = 0;
	err = NLMSG_DATA(nlh);
	fill_memory(&err->error, sizeof(err->error));

	rc = sendto(fd, nlh, nlh->nlmsg_len, MSG_DONTWAIT, NULL, 0);
	rcstr = sprintrc(rc);
	printf("sendto(%d, [{nlmsg_len=%u, nlmsg_type=NLMSG_ERROR"
	       ", nlmsg_flags=NLM_F_REQUEST, nlmsg_seq=0, nlmsg_pid=0}, ",
	       fd, nlh->nlmsg_len);
	print_quoted_hex(&err->error, sizeof(err->error));
	printf("], %u, MSG_DONTWAIT, NULL, 0) = %s\n", nlh->nlmsg_len, rcstr);

	/* error message without enough room for the whole struct nlmsgerr */
	nlh = nlh0 - (sizeof(*err) - 4);
	nlh->nlmsg_len = NLMSG_HDRLEN + (sizeof(*err) - 4);
	nlh->nlmsg_type = NLMSG_ERROR;
	nlh->nlmsg_flags = NLM_F_REQUEST;
	nlh->nlmsg_seq = 0;
	nlh->nlmsg_pid = 0;
	err = NLMSG_DATA(nlh);
	fill_memory(err, sizeof(*err) - 4);

	rc = sendto(fd, nlh, nlh->nlmsg_len, MSG_DONTWAIT, NULL, 0);
	rcstr = sprintrc(rc);
	printf("sendto(%d, [{nlmsg_len=%u, nlmsg_type=NLMSG_ERROR"
	       ", nlmsg_flags=NLM_F_REQUEST, nlmsg_seq=0, nlmsg_pid=0}, ",
	       fd, nlh->nlmsg_len);
	print_quoted_hex(err, sizeof(*err) - 4);
	printf("], %u, MSG_DONTWAIT, NULL, 0) = %s\n", nlh->nlmsg_len, rcstr);

	/* error message with nlmsg_len exceeding the allocated room */
	nlh->nlmsg_len = NLMSG_HDRLEN + sizeof(*err);

	rc = sendto(fd, nlh, nlh->nlmsg_len, MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, [{nlmsg_len=%u, nlmsg_type=NLMSG_ERROR"
	       ", nlmsg_flags=NLM_F_REQUEST, nlmsg_seq=0, nlmsg_pid=0}"
	       ", %p], %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh->nlmsg_len, err, nlh->nlmsg_len, sprintrc(rc));

	/* error message with room for the error code and a header */
	nlh = nlh0 - sizeof(*err);
	nlh->nlmsg_len = NLMSG_HDRLEN + sizeof(*err);
	nlh->nlmsg_type = NLMSG_ERROR;
	nlh->nlmsg_flags = NLM_F_REQUEST;
	nlh->nlmsg_seq = 0;
	nlh->nlmsg_pid = 0;
	err = NLMSG_DATA(nlh);
	err->error = -13;
	err->msg.nlmsg_len = NLMSG_HDRLEN;
	err->msg.nlmsg_type = NLMSG_NOOP;
	err->msg.nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST;
	err->msg.nlmsg_seq = 42;
	err->msg.nlmsg_pid = 1234;

	rc = sendto(fd, nlh, nlh->nlmsg_len, MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, [{nlmsg_len=%u, nlmsg_type=NLMSG_ERROR"
	       ", nlmsg_flags=NLM_F_REQUEST, nlmsg_seq=0, nlmsg_pid=0}"
	       ", {error=-EACCES, msg={nlmsg_len=%u, nlmsg_type=NLMSG_NOOP"
	       ", nlmsg_flags=NLM_F_REQUEST|0x%x, nlmsg_seq=%u, nlmsg_pid=%d}}]"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh->nlmsg_len, err->msg.nlmsg_len, NLM_F_DUMP,
	       err->msg.nlmsg_seq, err->msg.nlmsg_pid,
	       nlh->nlmsg_len, sprintrc(rc));

	/* err->msg.nlmsg_len < sizeof(err->msg) */
	err->msg.nlmsg_len = sizeof(err->msg.nlmsg_len);

	rc = sendto(fd, nlh, nlh->nlmsg_len, MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, [{nlmsg_len=%u, nlmsg_type=NLMSG_ERROR"
	       ", nlmsg_flags=NLM_F_REQUEST, nlmsg_seq=0, nlmsg_pid=0}"
	       ", {error=-EACCES, msg={nlmsg_len=%u, nlmsg_type=NLMSG_NOOP"
	       ", nlmsg_flags=NLM_F_REQUEST|0x%x, nlmsg_seq=%u, nlmsg_pid=%d}}]"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh->nlmsg_len, err->msg.nlmsg_len, NLM_F_DUMP,
	       err->msg.nlmsg_seq, err->msg.nlmsg_pid,
	       nlh->nlmsg_len, sprintrc(rc));

	/* nlh->nlmsg_len < NLMSG_HDRLEN + err->msg.nlmsg_len */
	err->msg.nlmsg_len = NLMSG_HDRLEN + 4;

	rc = sendto(fd, nlh, nlh->nlmsg_len, MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, [{nlmsg_len=%u, nlmsg_type=NLMSG_ERROR"
	       ", nlmsg_flags=NLM_F_REQUEST, nlmsg_seq=0, nlmsg_pid=0}"
	       ", {error=-EACCES, msg={nlmsg_len=%u, nlmsg_type=NLMSG_NOOP"
	       ", nlmsg_flags=NLM_F_REQUEST|0x%x, nlmsg_seq=%u, nlmsg_pid=%d}}]"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh->nlmsg_len, err->msg.nlmsg_len, NLM_F_DUMP,
	       err->msg.nlmsg_seq, err->msg.nlmsg_pid,
	       nlh->nlmsg_len, sprintrc(rc));

	/* error message with room for the error code, a header, and some data */
	nlh = nlh0 - sizeof(*err) - 4;
	nlh->nlmsg_len = NLMSG_HDRLEN + sizeof(*err) + 4;
	nlh->nlmsg_type = NLMSG_ERROR;
	nlh->nlmsg_flags = NLM_F_REQUEST;
	nlh->nlmsg_seq = 0;
	nlh->nlmsg_pid = 0;
	err = NLMSG_DATA(nlh);
	err->error = -13;
	err->msg.nlmsg_len = NLMSG_HDRLEN + 4;
	err->msg.nlmsg_type = NLMSG_NOOP;
	err->msg.nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST;
	err->msg.nlmsg_seq = 421;
	err->msg.nlmsg_pid = 12345;
	memcpy(NLMSG_DATA(&err->msg), "abcd", 4);

	rc = sendto(fd, nlh, nlh->nlmsg_len, MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, [{nlmsg_len=%u, nlmsg_type=NLMSG_ERROR"
	       ", nlmsg_flags=NLM_F_REQUEST, nlmsg_seq=0, nlmsg_pid=0}"
	       ", {error=-EACCES, msg=[{nlmsg_len=%u, nlmsg_type=NLMSG_NOOP"
	       ", nlmsg_flags=NLM_F_REQUEST|0x%x, nlmsg_seq=%u, nlmsg_pid=%d}"
	       ", \"\\x61\\x62\\x63\\x64\"]}], %u, MSG_DONTWAIT, NULL, 0)"
	       " = %s\n",
	       fd, nlh->nlmsg_len, err->msg.nlmsg_len, NLM_F_DUMP,
	       err->msg.nlmsg_seq, err->msg.nlmsg_pid,
	       nlh->nlmsg_len, sprintrc(rc));

	nlh->nlmsg_len = NLMSG_HDRLEN + sizeof(*err) + 1;
	err->msg.nlmsg_len = NLMSG_HDRLEN + 1;
	err->msg.nlmsg_type = SOCK_DIAG_BY_FAMILY;
	err->msg.nlmsg_flags = NLM_F_REQUEST;
	err->msg.nlmsg_seq = 4213;
	err->msg.nlmsg_pid = 123456;
	memcpy(NLMSG_DATA(&err->msg), "", 1);

	rc = sendto(fd, nlh, nlh->nlmsg_len, MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, [{nlmsg_len=%u, nlmsg_type=NLMSG_ERROR"
	       ", nlmsg_flags=NLM_F_REQUEST, nlmsg_seq=0, nlmsg_pid=0}"
	       ", {error=-EACCES, msg=[{nlmsg_len=%u"
	       ", nlmsg_type=SOCK_DIAG_BY_FAMILY, nlmsg_flags=NLM_F_REQUEST"
	       ", nlmsg_seq=%u, nlmsg_pid=%d}, {family=AF_UNSPEC}]}]"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh->nlmsg_len, err->msg.nlmsg_len,
	       err->msg.nlmsg_seq, err->msg.nlmsg_pid,
	       nlh->nlmsg_len, sprintrc(rc));
}

static void
test_nlmsg_done(const int fd)
{
	struct nlmsghdr *nlh;
	const int num = 0xfacefeed;
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(num));
	long rc;

	/* NLMSG_DONE message without enough room for an integer payload */
	nlh = nlh0;
	*nlh = (struct nlmsghdr) {
		.nlmsg_len = NLMSG_HDRLEN + sizeof(num),
		.nlmsg_type = NLMSG_DONE,
		.nlmsg_flags = NLM_F_MULTI
	};

	rc = sendto(fd, nlh, nlh->nlmsg_len, MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, [{nlmsg_len=%u, nlmsg_type=NLMSG_DONE"
	       ", nlmsg_flags=NLM_F_MULTI, nlmsg_seq=0, nlmsg_pid=0}, %p]"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh->nlmsg_len, nlh0 + NLMSG_HDRLEN,
	       nlh->nlmsg_len, sprintrc(rc));

	/* NLMSG_DONE message with enough room for an oddly short payload */
	nlh->nlmsg_len = NLMSG_HDRLEN + 2;
	nlh = nlh0 - 2;
	/* Beware of unaligned access to nlh members. */
	memmove(nlh, nlh0, sizeof(*nlh));
	memcpy(NLMSG_DATA(nlh), "42", 2);

	rc = sendto(fd, nlh, NLMSG_HDRLEN + 2, MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, [{nlmsg_len=%u, nlmsg_type=NLMSG_DONE"
	       ", nlmsg_flags=NLM_F_MULTI, nlmsg_seq=0, nlmsg_pid=0}"
	       ", \"\\x34\\x32\"], %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, NLMSG_HDRLEN + 2, NLMSG_HDRLEN + 2, sprintrc(rc));

	/* NLMSG_DONE message with enough room for an integer payload */
	nlh = nlh0 - sizeof(num);
	*nlh = (struct nlmsghdr) {
		.nlmsg_len = NLMSG_HDRLEN + sizeof(num),
		.nlmsg_type = NLMSG_DONE,
		.nlmsg_flags = NLM_F_MULTI
	};
	memcpy(NLMSG_DATA(nlh), &num, sizeof(num));

	rc = sendto(fd, nlh, nlh->nlmsg_len, MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, [{nlmsg_len=%u, nlmsg_type=NLMSG_DONE"
	       ", nlmsg_flags=NLM_F_MULTI, nlmsg_seq=0, nlmsg_pid=0}, %d]"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh->nlmsg_len, num, nlh->nlmsg_len, sprintrc(rc));
}

static void
test_ack_flags(const int fd)
{
	long rc;
	struct nlmsghdr nlh = {
		.nlmsg_len = sizeof(nlh),
		.nlmsg_type = NLMSG_ERROR,
	};

	nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_CAPPED,
	rc = sendto(fd, &nlh, sizeof(nlh), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {nlmsg_len=%u, nlmsg_type=NLMSG_ERROR"
	       ", nlmsg_flags=NLM_F_REQUEST|NLM_F_CAPPED, nlmsg_seq=0"
	       ", nlmsg_pid=0}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh.nlmsg_len, (unsigned) sizeof(nlh), sprintrc(rc));

	nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK_TLVS;
	rc = sendto(fd, &nlh, sizeof(nlh), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {nlmsg_len=%u, nlmsg_type=NLMSG_ERROR"
	       ", nlmsg_flags=NLM_F_REQUEST|NLM_F_ACK_TLVS, nlmsg_seq=0"
	       ", nlmsg_pid=0}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh.nlmsg_len, (unsigned) sizeof(nlh), sprintrc(rc));

	nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_CAPPED | NLM_F_ACK_TLVS;
	rc = sendto(fd, &nlh, sizeof(nlh), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {nlmsg_len=%u, nlmsg_type=NLMSG_ERROR"
	       ", nlmsg_flags=NLM_F_REQUEST|NLM_F_CAPPED|NLM_F_ACK_TLVS"
	       ", nlmsg_seq=0, nlmsg_pid=0}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh.nlmsg_len, (unsigned) sizeof(nlh), sprintrc(rc));
}

int main(void)
{
	const int fd = create_nl_socket(NETLINK_SOCK_DIAG);

	char *path = xasprintf("/proc/self/fd/%u", fd);
	char buf[256];
	if (getxattr(path, "system.sockprotoname", buf, sizeof(buf) - 1) < 0)
		perror_msg_and_skip("getxattr");
	free(path);

	send_query(fd);
	test_nlmsgerr(fd);
	test_nlmsg_done(fd);
	test_ack_flags(fd);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_SYS_XATTR_H")

#endif
