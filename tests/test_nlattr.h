/*
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
#include "print_fields.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include "netlink.h"
#include <linux/rtnetlink.h>

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
print_nlattr(const unsigned int nla_len, const char *const nla_type)
{
	printf(", {{nla_len=%u, nla_type=%s}, ", nla_len, nla_type);
}

#define TEST_NLATTR_(fd_, nlh0_, hdrlen_,				\
		     init_msg_, print_msg_,				\
		     nla_type_, nla_type_str_,				\
		     nla_data_len_, src_, slen_, ...)			\
	do {								\
		struct nlmsghdr *const nlh =				\
			(nlh0_) - (NLA_HDRLEN + (slen_));		\
		struct nlattr *const TEST_NLATTR_nla =			\
			NLMSG_ATTR(nlh, (hdrlen_));			\
		const unsigned int nla_len =				\
			NLA_HDRLEN + (nla_data_len_);			\
		const unsigned int msg_len =				\
			NLMSG_SPACE(hdrlen_) + nla_len;			\
									\
		(init_msg_)(nlh, msg_len);				\
		init_nlattr(TEST_NLATTR_nla, nla_len, (nla_type_),	\
			   (src_), (slen_));				\
									\
		const char *const errstr =				\
			sprintrc(sendto((fd_), nlh, msg_len,		\
					MSG_DONTWAIT, NULL, 0));	\
									\
		printf("sendto(%d, {", (fd_));				\
		(print_msg_)(msg_len);					\
		print_nlattr(nla_len, (nla_type_str_));			\
									\
		{ __VA_ARGS__; }					\
									\
		printf("}}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",		\
		       msg_len, errstr);				\
	} while (0)

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
			       pattern_, obj_, fallback_func, ...)	\
	do {								\
		const unsigned int plen =				\
			sizeof(obj_) - 1 > DEFAULT_STRLEN		\
			? DEFAULT_STRLEN : (int) sizeof(obj_) - 1;	\
		/* len < sizeof(obj_) */				\
		TEST_NLATTR_((fd_), (nlh0_), (hdrlen_),			\
			(init_msg_), (print_msg_),			\
			(nla_type_), (nla_type_str_),			\
			plen, (pattern_), plen,				\
			(fallback_func)((pattern_), plen));		\
		/* short read of sizeof(obj_) */			\
		TEST_NLATTR_((fd_), (nlh0_), (hdrlen_),			\
			(init_msg_), (print_msg_),			\
			(nla_type_), (nla_type_str_),			\
			sizeof(obj_),					\
			(pattern_), sizeof(obj_) - 1,			\
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
			      pattern_, obj_, fallback_func, ...)	\
	TEST_NLATTR_OBJECT_EX_((fd_), (nlh0_), (hdrlen_),		\
			       (init_msg_), (print_msg_),		\
			       (nla_type_), #nla_type_,			\
			       (pattern_), (obj_), (fallback_func),	\
			       __VA_ARGS__)

#define TEST_NLATTR_OBJECT(fd_, nlh0_, hdrlen_,				\
			   init_msg_, print_msg_,			\
			   nla_type_, pattern_, obj_, ...)		\
	TEST_NLATTR_OBJECT_EX_((fd_), (nlh0_), (hdrlen_),		\
			       (init_msg_), (print_msg_),		\
			       (nla_type_), #nla_type_,			\
			       (pattern_), (obj_), print_quoted_hex,	\
			       __VA_ARGS__)

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
				(print_elem_)(&(obj_)[i]);		\
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
				(print_elem_)(&(obj_)[i]);		\
			}						\
			printf(", %p]",					\
			       RTA_DATA(NLMSG_ATTR(nlh, (hdrlen_)))	\
			        + sizeof((obj_)[0])));			\
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
				(print_elem_)(&(obj_)[i]);		\
			}						\
			printf("]"));					\
	} while (0)

#define TEST_NESTED_NLATTR_OBJECT_EX_(fd_, nlh0_, hdrlen_,		\
				      init_msg_, print_msg_,		\
				      nla_type_, nla_type_str_,		\
				      pattern_, obj_, depth_, ...)	\
	do {								\
		const unsigned int plen =				\
			sizeof(obj_) - 1 > DEFAULT_STRLEN		\
			? DEFAULT_STRLEN : (int) sizeof(obj_) - 1;	\
		/* len < sizeof(obj_) */				\
		TEST_NLATTR_((fd_), (nlh0_) - NLA_HDRLEN * depth_,	\
			(hdrlen_) + NLA_HDRLEN * depth_,		\
			(init_msg_), (print_msg_),			\
			(nla_type_), (nla_type_str_),			\
			plen, (pattern_), plen,				\
			print_quoted_hex((pattern_), plen);		\
			size_t i;					\
			for (i = 0; i < depth_; ++i)			\
				printf("}"));				\
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
				      (pattern_), (obj_), (depth_),	\
				      __VA_ARGS__)

#define TEST_NESTED_NLATTR_OBJECT(fd_, nlh0_, hdrlen_,			\
				  init_msg_, print_msg_,		\
				  nla_type_, pattern_, obj_, ...)	\
	TEST_NESTED_NLATTR_OBJECT_EX_((fd_), (nlh0_), (hdrlen_),	\
				      (init_msg_), (print_msg_),	\
				      (nla_type_), #nla_type_,		\
				      (pattern_), (obj_), 1,		\
				      __VA_ARGS__)

#define TEST_NESTED_NLATTR_ARRAY(fd_, nlh0_, hdrlen_,			\
				 init_msg_, print_msg_,			\
				 nla_type_, pattern_, obj_, print_elem_)\
	do {								\
		const unsigned int plen =				\
			sizeof((obj_)[0]) - 1 > DEFAULT_STRLEN		\
			? DEFAULT_STRLEN : (int) sizeof((obj_)[0]) - 1;	\
		/* len < sizeof((obj_)[0]) */				\
		TEST_NLATTR_((fd_), (nlh0_) - NLA_HDRLEN,		\
			(hdrlen_) + NLA_HDRLEN,				\
			(init_msg_), (print_msg_),			\
			(nla_type_), #nla_type_,			\
			plen, (pattern_), plen,				\
			print_quoted_hex((pattern_), plen);		\
			printf("}"));					\
		/* sizeof((obj_)[0]) < len < sizeof(obj_) */		\
		TEST_NLATTR_((fd_), (nlh0_) - NLA_HDRLEN,		\
			(hdrlen_) + NLA_HDRLEN,				\
			(init_msg_), (print_msg_),			\
			(nla_type_), #nla_type_,			\
			sizeof(obj_) - 1,				\
			&(obj_), sizeof(obj_) - 1,			\
			printf("[");					\
			size_t i;					\
			for (i = 0; i < ARRAY_SIZE(obj_) - 1; ++i) {	\
				if (i) printf(", ");			\
				(print_elem_)(&(obj_)[i]);		\
			}						\
			printf("]}"));					\
		/* short read of sizeof(obj_) */			\
		TEST_NLATTR_((fd_), (nlh0_) - NLA_HDRLEN,		\
			(hdrlen_) + NLA_HDRLEN,				\
			(init_msg_), (print_msg_),			\
			(nla_type_), #nla_type_,			\
			sizeof(obj_),					\
			&(obj_), sizeof(obj_) - 1,			\
			printf("[");					\
			size_t i;					\
			for (i = 0; i < ARRAY_SIZE(obj_) - 1; ++i) {	\
				if (i) printf(", ");			\
				(print_elem_)(&(obj_)[i]);		\
			}						\
			printf(", %p]}",				\
			       RTA_DATA(TEST_NLATTR_nla)		\
			        + sizeof((obj_)[0])));			\
		/* sizeof(obj_) */					\
		TEST_NLATTR_((fd_), (nlh0_) - NLA_HDRLEN,		\
			(hdrlen_) + NLA_HDRLEN,				\
			(init_msg_), (print_msg_),			\
			(nla_type_), #nla_type_,			\
			sizeof(obj_),					\
			&(obj_), sizeof(obj_),				\
			printf("[");					\
			size_t i;					\
			for (i = 0; i < ARRAY_SIZE(obj_); ++i) {	\
				if (i) printf(", ");			\
				(print_elem_)(&(obj_)[i]);		\
			}						\
			printf("]}"));					\
	} while (0)
