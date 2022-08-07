/*
 * Copyright (c) 2017-2022 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "print_fields.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
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
	printf(", %s[{nla_len=%u, nla_type=%s}, ",
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
		print_sockfd((fd_), "sendto(", ", [");			\
		(print_msg_)(msg_len);					\
		print_nlattr(nla_len, (nla_type_str_),			\
			     (nla_total_len_) > (nla_data_len_));	\
									\
		{ __VA_ARGS__; }					\
									\
		if ((nla_total_len_) > (nla_data_len_))			\
			printf("]");					\
									\
		printf("]], %u, " XLAT_FMT ", NULL, 0) = %s\n",		\
		       msg_len, XLAT_ARGS(MSG_DONTWAIT), errstr);	\
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

#define TEST_NLATTR_OBJECT_(fd_, nlh0_, hdrlen_,			\
			    init_msg_, print_msg_,			\
			    nla_type_, nla_type_str_, pattern_, obj_, ...) \
	TEST_NLATTR_OBJECT_EX_((fd_), (nlh0_), (hdrlen_),		\
			       (init_msg_), (print_msg_),		\
			       (nla_type_), (nla_type_str_),		\
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

#define TEST_NLATTR_ARRAY_(fd_, nlh0_, hdrlen_,				\
			   init_msg_, print_msg_,			\
			   nla_type_, nla_type_str_,			\
			   pattern_, obj_, print_elem_)			\
	do {								\
		const unsigned int plen =				\
			sizeof((obj_)[0]) - 1 > DEFAULT_STRLEN		\
			? DEFAULT_STRLEN : (int) sizeof((obj_)[0]) - 1;	\
		/* len < sizeof((obj_)[0]) */				\
		TEST_NLATTR_((fd_), (nlh0_), (hdrlen_),			\
			(init_msg_), (print_msg_),			\
			(nla_type_), (nla_type_str_),			\
			plen, (pattern_), plen,				\
			print_quoted_hex((pattern_), plen));		\
		/* sizeof((obj_)[0]) < len < sizeof(obj_) */		\
		TEST_NLATTR_((fd_), (nlh0_), (hdrlen_),			\
			(init_msg_), (print_msg_),			\
			(nla_type_), (nla_type_str_),			\
			sizeof(obj_) - 1,				\
			&(obj_), sizeof(obj_) - 1,			\
			printf("[");					\
			for (size_t i = 0;				\
			    i < ARRAY_SIZE(obj_) - 1; ++i) {		\
				if (i) printf(", ");			\
				(print_elem_)(&(obj_)[i], i);		\
			}						\
			printf("]"));					\
		/* short read of sizeof(obj_) */			\
		TEST_NLATTR_((fd_), (nlh0_), (hdrlen_),			\
			(init_msg_), (print_msg_),			\
			(nla_type_), (nla_type_str_),			\
			sizeof(obj_),					\
			&(obj_), sizeof(obj_) - 1,			\
			printf("[");					\
			for (size_t i = 0;				\
			     i < ARRAY_SIZE(obj_) - 1; ++i) {		\
				if (i) printf(", ");			\
				(print_elem_)(&(obj_)[i], i);		\
			}						\
			printf(", ... /* %p */]",			\
			       RTA_DATA(NLMSG_ATTR(nlh, (hdrlen_)))	\
			        + sizeof(obj_) - sizeof((obj_)[0])));	\
		/* sizeof(obj_) */					\
		TEST_NLATTR_((fd_), (nlh0_), (hdrlen_),			\
			(init_msg_), (print_msg_),			\
			(nla_type_), (nla_type_str_),			\
			sizeof(obj_),					\
			&(obj_), sizeof(obj_),				\
			printf("[");					\
			for (size_t i = 0; i < ARRAY_SIZE(obj_); ++i) {	\
				if (i) printf(", ");			\
				(print_elem_)(&(obj_)[i], i);		\
			}						\
			printf("]"));					\
	} while (0)

#define TEST_NLATTR_ARRAY(fd_, nlh0_, hdrlen_,				\
			  init_msg_, print_msg_,			\
			  nla_type_, pattern_, obj_, print_elem_)	\
	TEST_NLATTR_ARRAY_((fd_), (nlh0_), (hdrlen_),			\
			   (init_msg_), (print_msg_),			\
			   (nla_type_), #nla_type_,			\
			   (pattern_), (obj_), (print_elem_))

#define TEST_NESTED_NLATTR_(fd_, nlh0_, hdrlen_,			\
			    init_msg_, print_msg_,			\
			    nla_type_, nla_type_str_,			\
			    nla_data_len_, src_, slen_, depth_, ...)	\
	TEST_NLATTR_((fd_), (nlh0_) - NLA_HDRLEN * (depth_),		\
		(hdrlen_) + NLA_HDRLEN * (depth_),			\
		(init_msg_), (print_msg_),				\
		(nla_type_), (nla_type_str_),				\
		(nla_data_len_), (src_), (slen_),			\
		__VA_ARGS__;						\
		for (size_t i = 0; i < (depth_); ++i)			\
			printf("]"))

#define TEST_NESTED_NLATTR_OBJECT_EX_MINSZ_(fd_, nlh0_, hdrlen_,	\
					    init_msg_, print_msg_,	\
					    nla_type_, nla_type_str_,	\
					    pattern_, obj_, minsz_,	\
					    fallback_func, depth_, ...)	\
	do {								\
		const unsigned int plen =				\
			sizeof(obj_) - 1 > DEFAULT_STRLEN		\
			? DEFAULT_STRLEN : (int) sizeof(obj_) - 1;	\
		/* len < sizeof(obj_) */				\
		if (plen > 0)						\
			TEST_NESTED_NLATTR_((fd_), (nlh0_), (hdrlen_),	\
				(init_msg_), (print_msg_),		\
				(nla_type_), (nla_type_str_),		\
				plen, (pattern_), plen, (depth_),	\
				(fallback_func)((pattern_), plen));	\
		/* short read of sizeof(obj_) */			\
		TEST_NESTED_NLATTR_((fd_), (nlh0_), (hdrlen_),		\
			(init_msg_), (print_msg_),			\
			(nla_type_), (nla_type_str_),			\
			sizeof(obj_), (pattern_), (minsz_) - 1, (depth_), \
			printf("%p", RTA_DATA(TEST_NLATTR_nla)));	\
		/* sizeof(obj_) */					\
		TEST_NESTED_NLATTR_((fd_), (nlh0_), (hdrlen_),		\
			(init_msg_), (print_msg_),			\
			(nla_type_), (nla_type_str_),			\
			sizeof(obj_), &(obj_), sizeof(obj_), (depth_),	\
			__VA_ARGS__);					\
	} while (0)

#define TEST_NESTED_NLATTR_OBJECT_EX_(fd_, nlh0_, hdrlen_,		\
				      init_msg_, print_msg_,		\
				      nla_type_, nla_type_str_,		\
				      pattern_, obj_,			\
				      fallback_func, depth_, ...)	\
	TEST_NESTED_NLATTR_OBJECT_EX_MINSZ_((fd_), (nlh0_), (hdrlen_),	\
					    (init_msg_), (print_msg_),	\
					    (nla_type_), (nla_type_str_), \
					    (pattern_), (obj_), sizeof(obj_), \
					    (fallback_func), (depth_),	\
					    __VA_ARGS__)

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

#define TEST_NESTED_NLATTR_ARRAY_EX_(fd_, nlh0_, hdrlen_,		\
				     init_msg_, print_msg_,		\
				     nla_type_, nla_type_str_,		\
				     pattern_, obj_, depth_,		\
				     print_elem_)			\
	do {								\
		const unsigned int plen =				\
			sizeof((obj_)[0]) - 1 > DEFAULT_STRLEN		\
			? DEFAULT_STRLEN : (int) sizeof((obj_)[0]) - 1;	\
		/* len < sizeof((obj_)[0]) */				\
		TEST_NESTED_NLATTR_((fd_), (nlh0_), (hdrlen_),		\
			(init_msg_), (print_msg_),			\
			(nla_type_), (nla_type_str_),			\
			plen, (pattern_), plen,	(depth_),		\
			print_quoted_hex((pattern_), plen));		\
		/* sizeof((obj_)[0]) < len < sizeof(obj_) */		\
		TEST_NESTED_NLATTR_((fd_), (nlh0_), (hdrlen_),		\
			(init_msg_), (print_msg_),			\
			(nla_type_), (nla_type_str_),			\
			sizeof(obj_) - 1,				\
			&(obj_), sizeof(obj_) - 1, (depth_),		\
			printf("[");					\
			for (size_t i = 0;				\
			     i < ARRAY_SIZE(obj_) - 1; ++i) {		\
				if (i) printf(", ");			\
				(print_elem_)(&(obj_)[i], i);		\
			}						\
			printf("]"));					\
		/* short read of sizeof(obj_) */			\
		TEST_NESTED_NLATTR_((fd_), (nlh0_), (hdrlen_),		\
			(init_msg_), (print_msg_),			\
			(nla_type_), (nla_type_str_),			\
			sizeof(obj_),					\
			&(obj_), sizeof(obj_) - 1, (depth_),		\
			printf("[");					\
			for (size_t i = 0;				\
			     i < ARRAY_SIZE(obj_) - 1; ++i) {		\
				if (i) printf(", ");			\
				(print_elem_)(&(obj_)[i], i);		\
			}						\
			printf(", ... /* %p */]",			\
			       RTA_DATA(TEST_NLATTR_nla)		\
			        + sizeof(obj_) - sizeof((obj_)[0])));	\
		/* sizeof(obj_) */					\
		TEST_NESTED_NLATTR_((fd_), (nlh0_), (hdrlen_),		\
			(init_msg_), (print_msg_),			\
			(nla_type_), (nla_type_str_),			\
			sizeof(obj_), &(obj_), sizeof(obj_), (depth_),	\
			printf("[");					\
			for (size_t i = 0; i < ARRAY_SIZE(obj_); ++i) {	\
				if (i) printf(", ");			\
				(print_elem_)(&(obj_)[i], i);		\
			}						\
			printf("]"));					\
	} while (0)

#define TEST_NESTED_NLATTR_ARRAY_EX(fd_, nlh0_, hdrlen_,		\
				    init_msg_, print_msg_,		\
				    nla_type_, pattern_, obj_, depth_,	\
				    print_elem_)			\
	TEST_NESTED_NLATTR_ARRAY_EX_(fd_, nlh0_, hdrlen_,		\
				     init_msg_, print_msg_,		\
				     nla_type_, #nla_type_, pattern_,	\
				     obj_, depth_, print_elem_)

#define TEST_NESTED_NLATTR_ARRAY(fd_, nlh0_, hdrlen_,			\
				 init_msg_, print_msg_,			\
				 nla_type_, pattern_, obj_, print_elem_)\
	TEST_NESTED_NLATTR_ARRAY_EX((fd_), (nlh0_), (hdrlen_),		\
				    (init_msg_), (print_msg_),		\
				    nla_type_, (pattern_), (obj_), 1,	\
				    (print_elem_))


/* Checks for specific typical decoders */
#define DEF_NLATTR_INTEGER_CHECK_(nla_data_name_, nla_data_type_, fmt_)	\
	static inline void						\
	check_##nla_data_name_##_nlattr(int fd, void *nlh0, size_t hdrlen, \
					void (*init_msg)(struct nlmsghdr *, \
							 unsigned int),	\
					void (*print_msg)(unsigned int), \
					unsigned int nla_type,		\
					const char *nla_type_str,	\
					void *pattern, size_t depth)	\
	{								\
		static const nla_data_type_ vecs[] = {			\
			(nla_data_type_) 0,				\
			(nla_data_type_) 1,				\
			(nla_data_type_) 0xdeadfacebeeffeedULL,		\
		};							\
		static char buf[sizeof(nla_data_type_) + 8];		\
		for (size_t i = 0; i < ARRAY_SIZE(vecs); i++) {		\
			TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,	\
						      init_msg, print_msg, \
						      nla_type, nla_type_str, \
						      pattern, vecs[i],	\
						      print_quoted_hex,	depth, \
						      printf(fmt_, vecs[i])); \
			fill_memory(buf, sizeof(buf));			\
			memcpy(buf, vecs + i, sizeof(vecs[i]));		\
			TEST_NLATTR_(fd, nlh0 - NLA_HDRLEN * depth,	\
				    hdrlen + NLA_HDRLEN * depth,	\
				    init_msg, print_msg,		\
				    nla_type, nla_type_str,		\
				    sizeof(vecs[i]) + 8,		\
				    buf, sizeof(vecs[i]) + 8,		\
				    printf(fmt_, vecs[i]);		\
				    for (size_t i = 0; i < depth; i++)	\
					    printf("]"));		\
		}							\
	}

DEF_NLATTR_INTEGER_CHECK_(u8, uint8_t, "%hhu")
DEF_NLATTR_INTEGER_CHECK_(u16, uint16_t, "%hu")
DEF_NLATTR_INTEGER_CHECK_(u32, uint32_t, "%u")
DEF_NLATTR_INTEGER_CHECK_(u64, uint64_t, "%" PRIu64)

DEF_NLATTR_INTEGER_CHECK_(x16, uint16_t, "%#hx")
DEF_NLATTR_INTEGER_CHECK_(x32, uint32_t, "%#x")

#define TEST_NLATTR_VAL(type_, fd_, nlh0_, hdrlen_,			\
			init_msg_, print_msg_,				\
			nla_type_, pattern_, depth_)			\
	check_##type_##_nlattr((fd_), (nlh0_), (hdrlen_),		\
			       (init_msg_), (print_msg_),		\
			       (nla_type_), #nla_type_, (pattern_), (depth_))

static inline void
check_clock_t_nlattr(int fd, void *nlh0, size_t hdrlen,
		     void (*init_msg)(struct nlmsghdr *, unsigned int),
		     void (*print_msg)(unsigned int),
		     unsigned int nla_type, const char *nla_type_str,
		     size_t depth)
{
	static const uint64_t vecs[] = { 0, 1, 9, 10, 99, 100, 249, 250, 999,
					 1000, 1023, 1024, 0xdefacebeeffedULL };
	static char buf[sizeof(uint64_t) + 1];
#if !XLAT_RAW
	static long clk_tck;
	static int precision;

	if (!clk_tck) {
		clk_tck = sysconf(_SC_CLK_TCK);
		precision = clk_tck > 100000000 ? 9
				: clk_tck > 10000000 ? 8
				: clk_tck > 1000000 ? 7
				: clk_tck > 100000 ? 6
				: clk_tck > 10000 ? 5
				: clk_tck > 1000 ? 4
				: clk_tck > 100 ? 3
				: clk_tck > 10 ? 2
				: clk_tck > 1 ? 1 : 0;
	}
#endif

	fill_memory(buf, sizeof(buf));
	TEST_NLATTR_(fd, nlh0 - NLA_HDRLEN * depth, hdrlen + NLA_HDRLEN * depth,
		     init_msg, print_msg, nla_type, nla_type_str,
		     sizeof(vecs[0]) + 1, buf, sizeof(vecs[0]) + 1,
		     print_quoted_hex(buf, sizeof(vecs[0]) + 1);
		     for (size_t i = 0; i < depth; i++)
			    printf("]"));

	for (size_t i = 0; i < ARRAY_SIZE(vecs); i++) {
		memcpy(buf, vecs + i, sizeof(vecs[i]));
		TEST_NLATTR_(fd, nlh0 - NLA_HDRLEN * depth,
			     hdrlen + NLA_HDRLEN * depth,
			     init_msg, print_msg, nla_type, nla_type_str,
			     sizeof(vecs[i]),
			     buf, sizeof(vecs[i]),
			     printf("%" PRIu64, vecs[i]);
#if !XLAT_RAW
			     if (i)
				     printf(" /* %.*f s */", precision,
					    (double) vecs[i] / clk_tck);
#endif
			     for (size_t i = 0; i < depth; i++)
				    printf("]"));
	}
	for (size_t i = 1; i < sizeof(vecs[0]); i++) {
		uint64_t val = vecs[ARRAY_SIZE(vecs) - 1] & MASK64(i * 8);
		TEST_NLATTR_(fd, nlh0 - NLA_HDRLEN * depth,
			     hdrlen + NLA_HDRLEN * depth,
			     init_msg, print_msg, nla_type, nla_type_str,
			     i, buf + BE_LE(sizeof(vecs[0]) - i, 0), i,
			     printf("%" PRIu64, val);
#if !XLAT_RAW
			     printf(" /* %.*f s */", precision,
				    (double) val / clk_tck);
#endif
			     for (size_t i = 0; i < depth; i++)
				    printf("]"));
	}
}
