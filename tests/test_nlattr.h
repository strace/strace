/*
 * Copyright (c) 2017-2019 The strace developers.
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
#include <linux/rtnetlink.h>

#ifndef PRINT_SOCK
# define PRINT_SOCK 0
#endif

static void
init_nlattr(struct nlattr *const nla,
	    const uint16_t nla_len,
	    const uint16_t nla_type,
	    const void *const src,
	    const size_t n)
{
	SET_STRUCT(struct nlattr, nla,
		.nla_len = nla_len,
		.nla_type = nla_type,
	);

	memcpy(RTA_DATA(nla), src, n);
}

static void
print_nlattr(const unsigned int nla_len, const char *const nla_type, bool add_data)
{
	printf(", %s{{nla_len=%u, nla_type=%s}, ",
	       add_data ? "[" : "", nla_len, nla_type);
}

static void
print_sockfd(int sockfd, const char *pfx, const char *sfx)
{
#if PRINT_SOCK
	static int fd = -1;
	static unsigned long inode;

	if (sockfd < 0) {
		printf("%s%d%s", pfx, sockfd, sfx);
		return;
	}

	if (sockfd != fd) {
		fd = sockfd;
		inode = inode_of_sockfd(fd);
	}

	printf("%s%d<socket:[%lu]>%s", pfx, sockfd, inode, sfx);
#else
	printf("%s%d%s", pfx, sockfd, sfx);
#endif
}

#define TEST_NLATTR_EX_(fd_, nlh0_, hdrlen_,				\
		     init_msg_, print_msg_,				\
		     nla_type_, nla_type_str_,				\
		     nla_data_len_, nla_total_len_,			\
		     src_, slen_, ...)					\
	do {								\
		struct nlmsghdr *const nlh =				\
			(nlh0_) - (NLA_HDRLEN + (slen_));		\
		struct nlattr *const TEST_NLATTR_nla =			\
			NLMSG_ATTR(nlh, (hdrlen_));			\
		const unsigned int nla_len =				\
			NLA_HDRLEN + (nla_data_len_);			\
		const unsigned int msg_len =				\
			NLMSG_SPACE(hdrlen_) + NLA_HDRLEN + (nla_total_len_); \
									\
		(init_msg_)(nlh, msg_len);				\
		init_nlattr(TEST_NLATTR_nla, nla_len, (nla_type_),	\
			   (src_), (slen_));				\
									\
		const char *const errstr =				\
			sprintrc(sendto((fd_), nlh, msg_len,		\
					MSG_DONTWAIT, NULL, 0));	\
									\
		print_sockfd((fd_), "sendto(", ", {");			\
		(print_msg_)(msg_len);					\
		print_nlattr(nla_len, (nla_type_str_),			\
			     (nla_total_len_) > (nla_data_len_));	\
									\
		{ __VA_ARGS__; }					\
									\
		if ((nla_total_len_) > (nla_data_len_))			\
			printf("]");					\
									\
		printf("}}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",		\
		       msg_len, errstr);				\
	} while (0)

#define TEST_NLATTR_(fd_, nlh0_, hdrlen_,				\
		     init_msg_, print_msg_,				\
		     nla_type_, nla_type_str_,				\
		     nla_data_len_, src_, slen_, ...)			\
	TEST_NLATTR_EX_((fd_), (nlh0_), (hdrlen_),			\
			(init_msg_), (print_msg_),			\
			(nla_type_), (nla_type_str_),			\
			(nla_data_len_), (nla_data_len_),		\
			(src_), (slen_), __VA_ARGS__)

#define TEST_NLATTR(fd_, nlh0_, hdrlen_,				\
		    init_msg_, print_msg_,				\
		    nla_type_,						\
		    nla_data_len_, src_, slen_, ...)			\
	TEST_NLATTR_((fd_), (nlh0_), (hdrlen_),				\
		(init_msg_), (print_msg_),				\
		(nla_type_), #nla_type_,				\
		(nla_data_len_), (src_), (slen_), __VA_ARGS__)

#define TEST_NLATTR_OBJECT_EX_(fd_, nlh0_, hdrlen_,			\
			       init_msg_, print_msg_,			\
			       nla_type_, nla_type_str_,		\
			       pattern_, obj_, minsz_, fallback_func, ...) \
	do {								\
		const unsigned int plen = MIN((minsz_) - 1, DEFAULT_STRLEN); \
		/* len < sizeof(obj_) */				\
		if (plen > 0)						\
			TEST_NLATTR_((fd_), (nlh0_), (hdrlen_),		\
				(init_msg_), (print_msg_),		\
				(nla_type_), (nla_type_str_),		\
				plen, (pattern_), plen,			\
				(fallback_func)((pattern_), plen));	\
		/* short read of sizeof(obj_) */			\
		TEST_NLATTR_((fd_), (nlh0_), (hdrlen_),			\
			(init_msg_), (print_msg_),			\
			(nla_type_), (nla_type_str_),			\
			sizeof(obj_),					\
			(pattern_), (minsz_) - 1,			\
			printf("%p",					\
			       RTA_DATA(NLMSG_ATTR(nlh, (hdrlen_)))));	\
		/* sizeof(obj_) */					\
		TEST_NLATTR_((fd_), (nlh0_), (hdrlen_),			\
			(init_msg_), (print_msg_),			\
			(nla_type_), (nla_type_str_),			\
			sizeof(obj_),					\
			&(obj_), sizeof(obj_),				\
			__VA_ARGS__);					\
	} while (0)

#define TEST_NLATTR_OBJECT_EX(fd_, nlh0_, hdrlen_,			\
			      init_msg_, print_msg_,			\
			      nla_type_,				\
			      pattern_, obj_, minsz_, fallback_func, ...) \
	TEST_NLATTR_OBJECT_EX_((fd_), (nlh0_), (hdrlen_),		\
			       (init_msg_), (print_msg_),		\
			       (nla_type_), #nla_type_,			\
			       (pattern_), (obj_), (minsz_),		\
			       (fallback_func),	__VA_ARGS__)

#define TEST_NLATTR_OBJECT(fd_, nlh0_, hdrlen_,				\
			   init_msg_, print_msg_,			\
			   nla_type_, pattern_, obj_, ...)		\
	TEST_NLATTR_OBJECT_EX_((fd_), (nlh0_), (hdrlen_),		\
			       (init_msg_), (print_msg_),		\
			       (nla_type_), #nla_type_,			\
			       (pattern_), (obj_), sizeof(obj_),	\
			       print_quoted_hex, __VA_ARGS__)

#define TEST_NLATTR_OBJECT_MINSZ(fd_, nlh0_, hdrlen_,			\
			   init_msg_, print_msg_,			\
			   nla_type_, pattern_, obj_, minsz_, ...)	\
	TEST_NLATTR_OBJECT_EX_((fd_), (nlh0_), (hdrlen_),		\
			       (init_msg_), (print_msg_),		\
			       (nla_type_), #nla_type_,			\
			       (pattern_), (obj_), (minsz_),		\
			       print_quoted_hex, __VA_ARGS__)

#define TEST_NLATTR_ARRAY(fd_, nlh0_, hdrlen_,				\
			  init_msg_, print_msg_,			\
			  nla_type_, pattern_, obj_, print_elem_)	\
	do {								\
		const unsigned int plen =				\
			sizeof((obj_)[0]) - 1 > DEFAULT_STRLEN		\
			? DEFAULT_STRLEN : (int) sizeof((obj_)[0]) - 1;	\
		/* len < sizeof((obj_)[0]) */				\
		TEST_NLATTR_((fd_), (nlh0_), (hdrlen_),			\
			(init_msg_), (print_msg_),			\
			(nla_type_), #nla_type_,			\
			plen, (pattern_), plen,				\
			print_quoted_hex((pattern_), plen));		\
		/* sizeof((obj_)[0]) < len < sizeof(obj_) */		\
		TEST_NLATTR_((fd_), (nlh0_), (hdrlen_),			\
			(init_msg_), (print_msg_),			\
			(nla_type_), #nla_type_,			\
			sizeof(obj_) - 1,				\
			&(obj_), sizeof(obj_) - 1,			\
			printf("[");					\
			size_t i;					\
			for (i = 0; i < ARRAY_SIZE(obj_) - 1; ++i) {	\
				if (i) printf(", ");			\
				(print_elem_)(&(obj_)[i], i);		\
			}						\
			printf("]"));					\
		/* short read of sizeof(obj_) */			\
		TEST_NLATTR_((fd_), (nlh0_), (hdrlen_),			\
			(init_msg_), (print_msg_),			\
			(nla_type_), #nla_type_,			\
			sizeof(obj_),					\
			&(obj_), sizeof(obj_) - 1,			\
			printf("[");					\
			size_t i;					\
			for (i = 0; i < ARRAY_SIZE(obj_) - 1; ++i) {	\
				if (i) printf(", ");			\
				(print_elem_)(&(obj_)[i], i);		\
			}						\
			printf(", ... /* %p */]",			\
			       RTA_DATA(NLMSG_ATTR(nlh, (hdrlen_)))	\
			        + sizeof(obj_) - sizeof((obj_)[0])));	\
		/* sizeof(obj_) */					\
		TEST_NLATTR_((fd_), (nlh0_), (hdrlen_),			\
			(init_msg_), (print_msg_),			\
			(nla_type_), #nla_type_,			\
			sizeof(obj_),					\
			&(obj_), sizeof(obj_),				\
			printf("[");					\
			size_t i;					\
			for (i = 0; i < ARRAY_SIZE(obj_); ++i) {	\
				if (i) printf(", ");			\
				(print_elem_)(&(obj_)[i], i);		\
			}						\
			printf("]"));					\
	} while (0)

#define TEST_NESTED_NLATTR_OBJECT_EX_(fd_, nlh0_, hdrlen_,		\
				      init_msg_, print_msg_,		\
				      nla_type_, nla_type_str_,		\
				      pattern_, obj_, fallback_func,	\
				      depth_, ...)	\
	do {								\
		const unsigned int plen =				\
			sizeof(obj_) - 1 > DEFAULT_STRLEN		\
			? DEFAULT_STRLEN : (int) sizeof(obj_) - 1;	\
		/* len < sizeof(obj_) */				\
		if (plen > 0)						\
			TEST_NLATTR_((fd_), (nlh0_) - NLA_HDRLEN * depth_, \
				(hdrlen_) + NLA_HDRLEN * depth_,	\
				(init_msg_), (print_msg_),		\
				(nla_type_), (nla_type_str_),		\
				plen, (pattern_), plen,			\
				(fallback_func)((pattern_), plen);	\
				size_t i;				\
				for (i = 0; i < depth_; ++i)		\
					printf("}"));			\
		/* short read of sizeof(obj_) */			\
		TEST_NLATTR_((fd_), (nlh0_) - NLA_HDRLEN * depth_,	\
			(hdrlen_) + NLA_HDRLEN * depth_,		\
			(init_msg_), (print_msg_),			\
			(nla_type_), (nla_type_str_),			\
			sizeof(obj_),					\
			(pattern_), sizeof(obj_) - 1,			\
			printf("%p", RTA_DATA(TEST_NLATTR_nla));	\
			size_t i;					\
			for (i = 0; i < depth_; ++i)			\
				printf("}"));				\
		/* sizeof(obj_) */					\
		TEST_NLATTR_((fd_), (nlh0_) - NLA_HDRLEN * depth_,	\
			(hdrlen_) + NLA_HDRLEN * depth_,		\
			(init_msg_), (print_msg_),			\
			(nla_type_), (nla_type_str_),			\
			sizeof(obj_),					\
			&(obj_), sizeof(obj_),				\
			__VA_ARGS__;					\
			size_t i;					\
			for (i = 0; i < depth_; ++i)			\
				printf("}"));				\
	} while (0)

#define TEST_NESTED_NLATTR_OBJECT_EX(fd_, nlh0_, hdrlen_,		\
				     init_msg_, print_msg_,		\
				     nla_type_, pattern_, obj_,		\
				     depth_, ...)			\
	TEST_NESTED_NLATTR_OBJECT_EX_((fd_), (nlh0_), (hdrlen_),	\
				      (init_msg_), (print_msg_),	\
				      (nla_type_), #nla_type_,		\
				      (pattern_), (obj_),		\
				      print_quoted_hex, (depth_),	\
				      __VA_ARGS__)

#define TEST_NESTED_NLATTR_OBJECT(fd_, nlh0_, hdrlen_,			\
				  init_msg_, print_msg_,		\
				  nla_type_, pattern_, obj_, ...)	\
	TEST_NESTED_NLATTR_OBJECT_EX_((fd_), (nlh0_), (hdrlen_),	\
				      (init_msg_), (print_msg_),	\
				      (nla_type_), #nla_type_,		\
				      (pattern_), (obj_),		\
				      print_quoted_hex, 1,		\
				      __VA_ARGS__)

#define TEST_NESTED_NLATTR_ARRAY_EX(fd_, nlh0_, hdrlen_,		\
				 init_msg_, print_msg_,			\
				 nla_type_, pattern_, obj_, depth_,	\
				 print_elem_)				\
	do {								\
		const unsigned int plen =				\
			sizeof((obj_)[0]) - 1 > DEFAULT_STRLEN		\
			? DEFAULT_STRLEN : (int) sizeof((obj_)[0]) - 1;	\
		/* len < sizeof((obj_)[0]) */				\
		TEST_NLATTR_((fd_), (nlh0_) - NLA_HDRLEN * depth_,	\
			(hdrlen_) + NLA_HDRLEN * depth_,		\
			(init_msg_), (print_msg_),			\
			(nla_type_), #nla_type_,			\
			plen, (pattern_), plen,				\
			print_quoted_hex((pattern_), plen);		\
			for (size_t i = 0; i < depth_; ++i)		\
				printf("}"));				\
		/* sizeof((obj_)[0]) < len < sizeof(obj_) */		\
		TEST_NLATTR_((fd_), (nlh0_) - NLA_HDRLEN * depth_,	\
			(hdrlen_) + NLA_HDRLEN * depth_,		\
			(init_msg_), (print_msg_),			\
			(nla_type_), #nla_type_,			\
			sizeof(obj_) - 1,				\
			&(obj_), sizeof(obj_) - 1,			\
			printf("[");					\
			size_t i;					\
			for (i = 0; i < ARRAY_SIZE(obj_) - 1; ++i) {	\
				if (i) printf(", ");			\
				(print_elem_)(&(obj_)[i], i);		\
			}						\
			printf("]");					\
			for (i = 0; i < depth_; ++i)			\
				printf("}"));				\
		/* short read of sizeof(obj_) */			\
		TEST_NLATTR_((fd_), (nlh0_) - NLA_HDRLEN * depth_,	\
			(hdrlen_) + NLA_HDRLEN * depth_,		\
			(init_msg_), (print_msg_),			\
			(nla_type_), #nla_type_,			\
			sizeof(obj_),					\
			&(obj_), sizeof(obj_) - 1,			\
			printf("[");					\
			size_t i;					\
			for (i = 0; i < ARRAY_SIZE(obj_) - 1; ++i) {	\
				if (i) printf(", ");			\
				(print_elem_)(&(obj_)[i], i);		\
			}						\
			printf(", ... /* %p */]",			\
			       RTA_DATA(TEST_NLATTR_nla)		\
			        + sizeof(obj_) - sizeof((obj_)[0]));	\
			for (i = 0; i < depth_; ++i)			\
				printf("}"));				\
		/* sizeof(obj_) */					\
		TEST_NLATTR_((fd_), (nlh0_) - NLA_HDRLEN * depth_,	\
			(hdrlen_) + NLA_HDRLEN * depth_,		\
			(init_msg_), (print_msg_),			\
			(nla_type_), #nla_type_,			\
			sizeof(obj_),					\
			&(obj_), sizeof(obj_),				\
			printf("[");					\
			size_t i;					\
			for (i = 0; i < ARRAY_SIZE(obj_); ++i) {	\
				if (i) printf(", ");			\
				(print_elem_)(&(obj_)[i], i);		\
			}						\
			printf("]");					\
			for (i = 0; i < depth_; ++i)			\
				printf("}"));				\
	} while (0)

#define TEST_NESTED_NLATTR_ARRAY(fd_, nlh0_, hdrlen_,			\
				 init_msg_, print_msg_,			\
				 nla_type_, pattern_, obj_, print_elem_)\
	TEST_NESTED_NLATTR_ARRAY_EX((fd_), (nlh0_), (hdrlen_),		\
				    (init_msg_), (print_msg_),		\
				    nla_type_, (pattern_), (obj_), 1,	\
				    (print_elem_))
