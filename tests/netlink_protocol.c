/*
 * Check decoding of netlink protocol.
 *
 * Copyright (c) 2014-2017 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016 Fabien Siron <fabien.siron@epita.fr>
 * Copyright (c) 2016-2026 The strace developers.
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
print_nlmsgerr_nesting_header(const struct nlmsghdr *const nlh)
{
	printf("{nlmsg_len=%u, nlmsg_type=%s, nlmsg_flags=0"
	       ", nlmsg_seq=0, nlmsg_pid=0}",
	       nlh->nlmsg_len,
	       nlh->nlmsg_type == NLMSG_ERROR ? "NLMSG_ERROR" : "NLMSG_NOOP");
}

static void
test_nlmsgerr_nesting(const int fd)
{
	enum {
		MAX_NESTING = 8,
		DEEP_NESTING = 0x4000,
		NESTING_STRIDE = NLMSG_HDRLEN + sizeof(int)
	};
	static const unsigned int depths[] = {
		1, MAX_NESTING - 1, MAX_NESTING,
		MAX_NESTING + 1, MAX_NESTING + 2, DEEP_NESTING
	};

	for (unsigned int i = 0; i < ARRAY_SIZE(depths); ++i) {
		const unsigned int depth = depths[i];
		const unsigned int len = depth * NESTING_STRIDE + NLMSG_HDRLEN;
		struct nlmsghdr *const nlh = xcalloc(1, len);

		for (unsigned int j = 0; j < depth; ++j) {
			struct nlmsghdr *const err_nlh =
				(void *) nlh + j * NESTING_STRIDE;
			err_nlh->nlmsg_len = len - j * NESTING_STRIDE;
			err_nlh->nlmsg_type = NLMSG_ERROR;
			const int error = -13;
			memcpy(NLMSG_DATA(err_nlh), &error, sizeof(error));
		}

		struct nlmsghdr *const terminal =
			(void *) nlh + depth * NESTING_STRIDE;
		terminal->nlmsg_len = NLMSG_HDRLEN;
		terminal->nlmsg_type = NLMSG_NOOP;

		const long rc = sendto(fd, nlh, len, MSG_DONTWAIT, NULL, 0);
		const char *const rcstr = sprintrc(rc);

		printf("sendto(%d, ", fd);
		const unsigned int decoded = MIN(depth, MAX_NESTING);
		for (unsigned int j = 0; j < decoded; ++j) {
			printf("[");
			print_nlmsgerr_nesting_header((void *) nlh
						      + j * NESTING_STRIDE);
			printf(", {error=-EACCES, msg=");
		}

		if (depth > MAX_NESTING)
			printf("[");
		print_nlmsgerr_nesting_header((void *) nlh
					      + decoded * NESTING_STRIDE);
		if (depth > MAX_NESTING)
			printf(", ...]");

		for (unsigned int j = 0; j < decoded; ++j)
			printf("}]");

		printf(", %u, MSG_DONTWAIT, NULL, 0) = %s\n", len, rcstr);
		free(nlh);
	}
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

static void
test_split_iov(const int fd)
{
	long rc;

	/*
	 * Case 1: A single 48-byte Netlink message split across 2 iovecs.
	 * iov[0] has 20 bytes (16-byte header + 4-byte payload).
	 * iov[1] has 28 bytes of remaining payload, with a fake header embedded.
	 */
	struct {
		struct nlmsghdr nlh;
		char data[4];
	} msg1_iov0 = {
		.nlh = {
			.nlmsg_len = 48,
			.nlmsg_type = NLMSG_NOOP,
			.nlmsg_flags = 0,
			.nlmsg_seq = 1,
			.nlmsg_pid = 0,
		},
		.data = { '1', '2', '3', '4' },
	};
	struct {
		struct nlmsghdr fake_nlh;
		char data[12];
	} msg1_iov1 = {
		.fake_nlh = {
			.nlmsg_len = 16,
			.nlmsg_type = NLMSG_DONE,
			.nlmsg_flags = 0,
			.nlmsg_seq = 2,
			.nlmsg_pid = 0,
		},
		.data = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L' },
	};
	struct iovec iov1[2] = {
		{ .iov_base = &msg1_iov0, .iov_len = sizeof(msg1_iov0) },
		{ .iov_base = &msg1_iov1, .iov_len = sizeof(msg1_iov1) },
	};
	struct msghdr mh1 = {
		.msg_iov = iov1,
		.msg_iovlen = 2,
	};

	rc = sendmsg(fd, &mh1, MSG_DONTWAIT);
	printf("sendmsg(%d, {msg_name=NULL, msg_namelen=0, msg_iov=[{iov_base="
	       "[{nlmsg_len=48, nlmsg_type=NLMSG_NOOP, nlmsg_flags=0"
	       ", nlmsg_seq=1, nlmsg_pid=0}, \"\\x31\\x32\\x33\\x34\"]"
	       ", iov_len=20}, {iov_base=", fd);
	print_quoted_hex(&msg1_iov1, sizeof(msg1_iov1));
	printf(", iov_len=28}], msg_iovlen=2, msg_controllen=0, msg_flags=0}"
	       ", MSG_DONTWAIT) = %s\n", sprintrc(rc));

	/*
	 * Case 2: A 32-byte message split across 2 iovecs, followed by
	 * a complete 16-byte NLMSG_DONE message in the remainder of iov[1].
	 */
	struct {
		struct nlmsghdr nlh;
		char data[4];
	} msg2_iov0 = {
		.nlh = {
			.nlmsg_len = 32,
			.nlmsg_type = NLMSG_NOOP,
			.nlmsg_flags = 0,
			.nlmsg_seq = 3,
			.nlmsg_pid = 0,
		},
		.data = { '1', '2', '3', '4' },
	};
	struct {
		char tail[12];
		struct nlmsghdr nlh2;
	} msg2_iov1 = {
		.tail = { '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6' },
		.nlh2 = {
			.nlmsg_len = 16,
			.nlmsg_type = NLMSG_DONE,
			.nlmsg_flags = 0,
			.nlmsg_seq = 4,
			.nlmsg_pid = 0,
		},
	};
	struct iovec iov2[2] = {
		{ .iov_base = &msg2_iov0, .iov_len = sizeof(msg2_iov0) },
		{ .iov_base = &msg2_iov1, .iov_len = sizeof(msg2_iov1) },
	};
	struct msghdr mh2 = {
		.msg_iov = iov2,
		.msg_iovlen = 2,
	};

	rc = sendmsg(fd, &mh2, MSG_DONTWAIT);
	printf("sendmsg(%d, {msg_name=NULL, msg_namelen=0, msg_iov=[{iov_base="
	       "[{nlmsg_len=32, nlmsg_type=NLMSG_NOOP, nlmsg_flags=0"
	       ", nlmsg_seq=3, nlmsg_pid=0}, \"\\x31\\x32\\x33\\x34\"]"
	       ", iov_len=20}, {iov_base=[\"\\x35\\x36\\x37\\x38\\x39\\x30"
	       "\\x31\\x32\\x33\\x34\\x35\\x36\", {nlmsg_len=16"
	       ", nlmsg_type=NLMSG_DONE, nlmsg_flags=0, nlmsg_seq=4"
	       ", nlmsg_pid=0}], iov_len=28}], msg_iovlen=2, msg_controllen=0"
	       ", msg_flags=0}, MSG_DONTWAIT) = %s\n",
	       fd, sprintrc(rc));

	/*
	 * Case 3: A 40-byte message split across 3 iovecs (20 + 12 + 8),
	 * followed by a complete 16-byte NLMSG_DONE message in a 4th iovec
	 * to verify decoding resumes when the continuation ends exactly at
	 * the previous iovec boundary.
	 */
	struct {
		struct nlmsghdr nlh;
		char data[4];
	} msg3_iov0 = {
		.nlh = {
			.nlmsg_len = 40,
			.nlmsg_type = NLMSG_NOOP,
			.nlmsg_flags = 0,
			.nlmsg_seq = 5,
			.nlmsg_pid = 0,
		},
		.data = { '1', '2', '3', '4' },
	};
	char msg3_iov1[12] = { '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6' };
	char msg3_iov2[8] = { '7', '8', '9', '0', '1', '2', '3', '4' };
	struct nlmsghdr msg3_iov3 = {
		.nlmsg_len = 16,
		.nlmsg_type = NLMSG_DONE,
		.nlmsg_flags = 0,
		.nlmsg_seq = 6,
		.nlmsg_pid = 0,
	};
	struct iovec iov3[4] = {
		{ .iov_base = &msg3_iov0, .iov_len = sizeof(msg3_iov0) },
		{ .iov_base = msg3_iov1, .iov_len = sizeof(msg3_iov1) },
		{ .iov_base = msg3_iov2, .iov_len = sizeof(msg3_iov2) },
		{ .iov_base = &msg3_iov3, .iov_len = sizeof(msg3_iov3) },
	};
	struct msghdr mh3 = {
		.msg_iov = iov3,
		.msg_iovlen = 4,
	};

	rc = sendmsg(fd, &mh3, MSG_DONTWAIT);
	printf("sendmsg(%d, {msg_name=NULL, msg_namelen=0, msg_iov=[{iov_base="
	       "[{nlmsg_len=40, nlmsg_type=NLMSG_NOOP, nlmsg_flags=0"
	       ", nlmsg_seq=5, nlmsg_pid=0}, \"\\x31\\x32\\x33\\x34\"]"
	       ", iov_len=20}, {iov_base=\"\\x35\\x36\\x37\\x38\\x39\\x30"
	       "\\x31\\x32\\x33\\x34\\x35\\x36\", iov_len=12}, {iov_base="
	       "\"\\x37\\x38\\x39\\x30\\x31\\x32\\x33\\x34\", iov_len=8}"
	       ", {iov_base={nlmsg_len=16, nlmsg_type=NLMSG_DONE, nlmsg_flags=0"
	       ", nlmsg_seq=6, nlmsg_pid=0}, iov_len=16}]"
	       ", msg_iovlen=4, msg_controllen=0, msg_flags=0}, MSG_DONTWAIT) = %s\n",
	       fd, sprintrc(rc));

	/*
	 * Case 4: A Netlink header itself split across 2 iovecs.
	 * iov[0] has only 8 bytes (< sizeof(struct nlmsghdr)).
	 * iov[1] starts with bytes matching a valid struct nlmsghdr; it must
	 * be printed as raw hex rather than misinterpreted as a top-level header.
	 */
	struct nlmsghdr msg4_hdr = {
		.nlmsg_len = 32,
		.nlmsg_type = NLMSG_NOOP,
		.nlmsg_flags = 0,
		.nlmsg_seq = 6,
		.nlmsg_pid = 0,
	};
	struct {
		struct nlmsghdr fake_nlh;
		char data[8];
	} msg4_iov1 = {
		.fake_nlh = {
			.nlmsg_len = 16,
			.nlmsg_type = NLMSG_DONE,
			.nlmsg_flags = 0,
			.nlmsg_seq = 7,
			.nlmsg_pid = 0,
		},
		.data = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H' },
	};
	struct iovec iov4[2] = {
		{ .iov_base = &msg4_hdr, .iov_len = 8 },
		{ .iov_base = &msg4_iov1, .iov_len = sizeof(msg4_iov1) },
	};
	struct msghdr mh4 = {
		.msg_iov = iov4,
		.msg_iovlen = 2,
	};

	rc = sendmsg(fd, &mh4, MSG_DONTWAIT);
	printf("sendmsg(%d, {msg_name=NULL, msg_namelen=0, msg_iov=[{iov_base=",
	       fd);
	print_quoted_hex(&msg4_hdr, 8);
	printf(", iov_len=8}, {iov_base=");
	print_quoted_hex(&msg4_iov1, sizeof(msg4_iov1));
	printf(", iov_len=24}], msg_iovlen=2, msg_controllen=0, msg_flags=0}"
	       ", MSG_DONTWAIT) = %s\n", sprintrc(rc));

	/*
	 * Case 5: NLMSG_ALIGN padding crossing an iovec boundary.
	 * iov[0] has an unaligned 21-byte message (NLMSG_ALIGN(21) == 24).
	 * iov[1] starts with the 3 bytes of alignment padding followed by
	 * a complete 16-byte NLMSG_DONE message.
	 */
	struct {
		struct nlmsghdr nlh;
		char data[5];
	} msg5_iov0 = {
		.nlh = {
			.nlmsg_len = 21,
			.nlmsg_type = NLMSG_NOOP,
			.nlmsg_flags = 0,
			.nlmsg_seq = 8,
			.nlmsg_pid = 0,
		},
		.data = { '1', '2', '3', '4', '5' },
	};
	struct nlmsghdr msg5_nlh2 = {
		.nlmsg_len = 16,
		.nlmsg_type = NLMSG_DONE,
		.nlmsg_flags = 0,
		.nlmsg_seq = 9,
		.nlmsg_pid = 0,
	};
	unsigned char msg5_iov1[3 + sizeof(msg5_nlh2)];
	msg5_iov1[0] = 0xaa;
	msg5_iov1[1] = 0xbb;
	msg5_iov1[2] = 0xcc;
	memcpy(msg5_iov1 + 3, &msg5_nlh2, sizeof(msg5_nlh2));
	struct iovec iov5[2] = {
		{ .iov_base = &msg5_iov0, .iov_len = 21 },
		{ .iov_base = msg5_iov1, .iov_len = sizeof(msg5_iov1) },
	};
	struct msghdr mh5 = {
		.msg_iov = iov5,
		.msg_iovlen = 2,
	};

	rc = sendmsg(fd, &mh5, MSG_DONTWAIT);
	printf("sendmsg(%d, {msg_name=NULL, msg_namelen=0, msg_iov=[{iov_base="
	       "[{nlmsg_len=21, nlmsg_type=NLMSG_NOOP, nlmsg_flags=0"
	       ", nlmsg_seq=8, nlmsg_pid=0}, \"\\x31\\x32\\x33\\x34\\x35\"]"
	       ", iov_len=21}, {iov_base=[\"\\xaa\\xbb\\xcc\", {nlmsg_len=16"
	       ", nlmsg_type=NLMSG_DONE, nlmsg_flags=0, nlmsg_seq=9"
	       ", nlmsg_pid=0}], iov_len=19}], msg_iovlen=2, msg_controllen=0"
	       ", msg_flags=0}, MSG_DONTWAIT) = %s\n",
	       fd, sprintrc(rc));

	/*
	 * Case 6: A split message whose continuation iovec has a NULL base
	 * with iov_len > pending_len.  strace must not advance past NULL or
	 * resume decoding on subsequent iovecs.
	 */
	struct {
		struct nlmsghdr nlh;
		char data[4];
	} msg6_iov0 = {
		.nlh = {
			.nlmsg_len = 32,
			.nlmsg_type = NLMSG_NOOP,
			.nlmsg_flags = 0,
			.nlmsg_seq = 10,
			.nlmsg_pid = 0,
		},
		.data = { '1', '2', '3', '4' },
	};
	struct nlmsghdr msg6_iov2 = {
		.nlmsg_len = 16,
		.nlmsg_type = NLMSG_DONE,
		.nlmsg_flags = 0,
		.nlmsg_seq = 11,
		.nlmsg_pid = 0,
	};
	struct iovec iov6[3] = {
		{ .iov_base = &msg6_iov0, .iov_len = sizeof(msg6_iov0) },
		{ .iov_base = NULL, .iov_len = 28 },
		{ .iov_base = &msg6_iov2, .iov_len = sizeof(msg6_iov2) },
	};
	struct msghdr mh6 = {
		.msg_iov = iov6,
		.msg_iovlen = 3,
	};

	rc = sendmsg(fd, &mh6, MSG_DONTWAIT);
	printf("sendmsg(%d, {msg_name=NULL, msg_namelen=0, msg_iov=[{iov_base="
	       "[{nlmsg_len=32, nlmsg_type=NLMSG_NOOP, nlmsg_flags=0"
	       ", nlmsg_seq=10, nlmsg_pid=0}, \"\\x31\\x32\\x33\\x34\"]"
	       ", iov_len=20}, {iov_base=NULL, iov_len=28}, {iov_base=",
	       fd);
	print_quoted_hex(&msg6_iov2, sizeof(msg6_iov2));
	printf(", iov_len=16}], msg_iovlen=3, msg_controllen=0, msg_flags=0}"
	       ", MSG_DONTWAIT) = %s\n", sprintrc(rc));
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
	test_nlmsgerr_nesting(fd);
	test_nlmsg_done(fd);
	test_ack_flags(fd);
	test_split_iov(fd);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_SYS_XATTR_H")

#endif
