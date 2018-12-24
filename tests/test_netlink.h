/*
 * Copyright (c) 2017-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "print_fields.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include "netlink.h"

#define TEST_NETLINK_(fd_, nlh0_,					\
		      type_, type_str_,					\
		      flags_, flags_str_,				\
		      data_len_, src_, slen_, ...)			\
	do {								\
		struct nlmsghdr *const TEST_NETLINK_nlh =		\
			(nlh0_) - (slen_);				\
		const unsigned int msg_len =				\
			NLMSG_HDRLEN + (data_len_);			\
									\
		SET_STRUCT(struct nlmsghdr, TEST_NETLINK_nlh,		\
			.nlmsg_len = msg_len,				\
			.nlmsg_type = (type_),				\
			.nlmsg_flags = (flags_)				\
		);							\
		memcpy(NLMSG_DATA(TEST_NETLINK_nlh), (src_), (slen_));	\
									\
		const char *const errstr =				\
			sprintrc(sendto((fd_), TEST_NETLINK_nlh,	\
					msg_len, MSG_DONTWAIT,		\
					NULL, 0));			\
									\
		printf("sendto(%d, {{len=%u, type=%s"			\
		       ", flags=%s, seq=0, pid=0}, ",			\
		       (fd_), msg_len, (type_str_), (flags_str_));	\
									\
		{ __VA_ARGS__; }					\
									\
		printf("}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",		\
		       msg_len, errstr);				\
	} while (0)

#define TEST_NETLINK(fd_, nlh0_, type_, flags_,				\
		     data_len_, src_, slen_, ...)			\
	TEST_NETLINK_((fd_), (nlh0_),					\
		      (type_), #type_,					\
		      (flags_), #flags_,				\
		      (data_len_), (src_), (slen_), __VA_ARGS__)

#define TEST_NETLINK_OBJECT_EX_(fd_, nlh0_,				\
				type_, type_str_,			\
				flags_, flags_str_,			\
				obj_, fallback_func, ...)		\
	do {								\
		char pattern[DEFAULT_STRLEN];				\
		fill_memory_ex(pattern, sizeof(pattern),		\
			       'a', 'z' - 'a' + 1);			\
		const unsigned int plen =				\
			sizeof(obj_) - 1 > DEFAULT_STRLEN		\
			? DEFAULT_STRLEN : (int) sizeof(obj_) - 1;	\
		/* len < sizeof(obj_) */				\
		TEST_NETLINK_((fd_), (nlh0_),				\
			      (type_), (type_str_),			\
			      (flags_), (flags_str_),			\
			      plen, pattern, plen,			\
			      (fallback_func)(pattern, plen));		\
		/* short read of sizeof(obj_) */			\
		TEST_NETLINK_((fd_), (nlh0_),				\
			      (type_), (type_str_),			\
			      (flags_), (flags_str_),			\
			      sizeof(obj_),				\
			      pattern, plen,				\
			      printf("%p",				\
				     NLMSG_DATA(TEST_NETLINK_nlh)));	\
		/* sizeof(obj_) */					\
		TEST_NETLINK_((fd_), (nlh0_),				\
			      (type_), (type_str_),			\
			      (flags_), (flags_str_),			\
			      sizeof(obj_),				\
			      &(obj_), sizeof(obj_),			\
			      __VA_ARGS__);				\
	} while (0)

#define TEST_NETLINK_OBJECT_EX(fd_, nlh0_,				\
			       type_, flags_,				\
			       obj_, fallback_func, ...)		\
	TEST_NETLINK_OBJECT_EX_((fd_), (nlh0),				\
				(type_), #type_,			\
				(flags_), #flags_,			\
				(obj_), (fallback_func), __VA_ARGS__)

#define TEST_NETLINK_OBJECT(fd_, nlh0_,					\
			    type_, flags_,				\
			    obj_, ...)					\
	TEST_NETLINK_OBJECT_EX_((fd_), (nlh0),				\
				(type_), #type_,			\
				(flags_), #flags_,			\
				(obj_), print_quoted_hex, __VA_ARGS__)
