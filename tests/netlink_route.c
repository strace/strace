/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
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
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "test_netlink.h"
#include <linux/if_arp.h>
#include <linux/rtnetlink.h>

#ifdef HAVE_IF_INDEXTONAME
/* <linux/if.h> used to conflict with <net/if.h> */
extern unsigned int if_nametoindex(const char *);
# define IFINDEX_LO	(if_nametoindex("lo"))
#else
# define IFINDEX_LO	1
#endif

#define TEST_NL_ROUTE(fd_, nlh0_, type_, obj_, print_family_, ...)	\
	do {								\
		/* family and string */					\
		TEST_NETLINK((fd_), (nlh0_),				\
			     type_, NLM_F_REQUEST,			\
			     sizeof(obj_) - 1,				\
			     &(obj_), sizeof(obj_) - 1,			\
			     (print_family_);				\
			     printf(", ...}"));				\
									\
		/* sizeof(obj_) */					\
		TEST_NETLINK((fd_), (nlh0_),				\
			     type_, NLM_F_REQUEST,			\
			     sizeof(obj_), &(obj_), sizeof(obj_),	\
			     (print_family_);				\
			      __VA_ARGS__);				\
									\
		/* short read of sizeof(obj_) */			\
		TEST_NETLINK((fd_), (nlh0_),				\
			     type_, NLM_F_REQUEST,			\
			     sizeof(obj_), &(obj_), sizeof(obj_) - 1,	\
			     (print_family_);				\
			     printf(", %p}",				\
				    NLMSG_DATA(TEST_NETLINK_nlh) + 1));	\
	} while (0)

static void
test_nlmsg_type(const int fd)
{
	long rc;
	struct nlmsghdr nlh = {
		.nlmsg_len = sizeof(nlh),
		.nlmsg_type = RTM_GETLINK,
		.nlmsg_flags = NLM_F_REQUEST,
	};

	rc = sendto(fd, &nlh, sizeof(nlh), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {len=%u, type=RTM_GETLINK"
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
	};

	nlh.nlmsg_type = RTM_GETLINK;
	nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	rc = sendto(fd, &nlh, sizeof(nlh), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {len=%u, type=RTM_GETLINK"
	       ", flags=NLM_F_REQUEST|NLM_F_DUMP, seq=0, pid=0}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh.nlmsg_len, (unsigned) sizeof(nlh), sprintrc(rc));

	nlh.nlmsg_type = RTM_DELACTION;
	nlh.nlmsg_flags = NLM_F_ROOT;
	rc = sendto(fd, &nlh, sizeof(nlh), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {len=%u, type=RTM_DELACTION"
	       ", flags=NLM_F_ROOT, seq=0, pid=0}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh.nlmsg_len, (unsigned) sizeof(nlh), sprintrc(rc));

	nlh.nlmsg_type = RTM_NEWLINK;
	nlh.nlmsg_flags = NLM_F_ECHO | NLM_F_REPLACE;
	rc = sendto(fd, &nlh, sizeof(nlh), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {len=%u, type=RTM_NEWLINK"
	       ", flags=NLM_F_ECHO|NLM_F_REPLACE, seq=0, pid=0}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh.nlmsg_len, (unsigned) sizeof(nlh), sprintrc(rc));

	nlh.nlmsg_type = RTM_DELLINK;
	nlh.nlmsg_flags = NLM_F_REPLACE;
	rc = sendto(fd, &nlh, sizeof(nlh), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {len=%u, type=RTM_DELLINK"
	       ", flags=%#x /* NLM_F_??? */, seq=0, pid=0}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh.nlmsg_len, NLM_F_REPLACE,
	       (unsigned) sizeof(nlh), sprintrc(rc));
}

static void
test_rtnl_unspec(const int fd)
{
	void *const nlh0 = tail_alloc(NLMSG_HDRLEN);

	/* unspecified family only */
	uint8_t family = 0;
	TEST_NETLINK_(fd, nlh0,
		      0xffff, "0xffff /* RTM_??? */",
		      NLM_F_REQUEST, "NLM_F_REQUEST",
		      sizeof(family), &family, sizeof(family),
		      printf("{family=AF_UNSPEC}"));

	/* unknown family only */
	family = 0xff;
	TEST_NETLINK_(fd, nlh0,
		      0xffff, "0xffff /* RTM_??? */",
		      NLM_F_REQUEST, "NLM_F_REQUEST",
		      sizeof(family), &family, sizeof(family),
		      printf("{family=0xff /* AF_??? */}"));

	/* short read of family */
	TEST_NETLINK_(fd, nlh0,
		      0xffff, "0xffff /* RTM_??? */",
		      NLM_F_REQUEST, "NLM_F_REQUEST",
		      sizeof(family), &family, sizeof(family) - 1,
		      printf("%p", NLMSG_DATA(TEST_NETLINK_nlh)));

	/* unspecified family and string */
	char buf[sizeof(family) + 4];
	family = 0;
	memcpy(buf, &family, sizeof(family));
	memcpy(buf + sizeof(family), "1234", 4);
	TEST_NETLINK_(fd, nlh0,
		      0xffff, "0xffff /* RTM_??? */",
		      NLM_F_REQUEST, "NLM_F_REQUEST",
		      sizeof(buf), buf, sizeof(buf),
		      printf("{family=AF_UNSPEC, \"\\x31\\x32\\x33\\x34\"}"));

	/* unknown family and string */
	family = 0xfd;
	memcpy(buf, &family, sizeof(family));
	TEST_NETLINK_(fd, nlh0,
		      0xffff, "0xffff /* RTM_??? */",
		      NLM_F_REQUEST, "NLM_F_REQUEST",
		      sizeof(buf), buf, sizeof(buf),
		      printf("{family=%#x /* AF_??? */"
			     ", \"\\x31\\x32\\x33\\x34\"}", family));
}

static void
test_rtnl_link(const int fd)
{
	void *const nlh0 = tail_alloc(NLMSG_HDRLEN);
	const struct ifinfomsg ifinfo = {
		.ifi_family = AF_UNIX,
		.ifi_type = ARPHRD_LOOPBACK,
		.ifi_index = IFINDEX_LO,
		.ifi_flags = IFF_UP,
		.ifi_change = 0xfabcdeba
	};

	TEST_NL_ROUTE(fd, nlh0, RTM_GETLINK, ifinfo,
		      printf("{ifi_family=AF_UNIX"),
		      printf(", ifi_type=ARPHRD_LOOPBACK"
			     ", ifi_index=if_nametoindex(\"lo\")"
			     ", ifi_flags=IFF_UP");
		      PRINT_FIELD_X(", ", ifinfo, ifi_change);
		      printf("}"));
}

int main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	int fd = create_nl_socket(NETLINK_ROUTE);

	test_nlmsg_type(fd);
	test_nlmsg_flags(fd);
	test_rtnl_unspec(fd);
	test_rtnl_link(fd);

	printf("+++ exited with 0 +++\n");

	return 0;
}
