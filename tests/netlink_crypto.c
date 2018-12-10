/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#ifdef HAVE_LINUX_CRYPTOUSER_H

# include <stdio.h>
# include <unistd.h>
# include <sys/socket.h>
# include <linux/cryptouser.h>
# include "test_netlink.h"

static void
test_nlmsg_type(const int fd)
{
	long rc;
	struct nlmsghdr nlh = {
		.nlmsg_len = sizeof(nlh),
		.nlmsg_type = CRYPTO_MSG_NEWALG,
		.nlmsg_flags = NLM_F_REQUEST,
	};

	rc = sendto(fd, &nlh, sizeof(nlh), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {len=%u, type=CRYPTO_MSG_NEWALG"
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

	nlh.nlmsg_type = CRYPTO_MSG_GETALG;
	nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	rc = sendto(fd, &nlh, sizeof(nlh), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {len=%u, type=CRYPTO_MSG_GETALG"
	       ", flags=NLM_F_REQUEST|NLM_F_DUMP, seq=0, pid=0}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh.nlmsg_len, (unsigned) sizeof(nlh), sprintrc(rc));

	nlh.nlmsg_type = CRYPTO_MSG_NEWALG;
	nlh.nlmsg_flags = NLM_F_ECHO | NLM_F_REPLACE;
	rc = sendto(fd, &nlh, sizeof(nlh), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {len=%u, type=CRYPTO_MSG_NEWALG"
	       ", flags=NLM_F_ECHO|NLM_F_REPLACE, seq=0, pid=0}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh.nlmsg_len, (unsigned) sizeof(nlh), sprintrc(rc));

	nlh.nlmsg_type = CRYPTO_MSG_DELALG;
	nlh.nlmsg_flags = NLM_F_ECHO | NLM_F_NONREC;
	rc = sendto(fd, &nlh, sizeof(nlh), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {len=%u, type=CRYPTO_MSG_DELALG"
	       ", flags=NLM_F_ECHO|NLM_F_NONREC, seq=0, pid=0}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh.nlmsg_len, (unsigned) sizeof(nlh), sprintrc(rc));

	nlh.nlmsg_type = CRYPTO_MSG_UPDATEALG;
	nlh.nlmsg_flags = NLM_F_REPLACE;
	rc = sendto(fd, &nlh, sizeof(nlh), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {len=%u, type=CRYPTO_MSG_UPDATEALG"
	       ", flags=%#x /* NLM_F_??? */, seq=0, pid=0}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh.nlmsg_len, NLM_F_REPLACE,
	       (unsigned) sizeof(nlh), sprintrc(rc));
}

static void
test_crypto_msg_newalg(const int fd)
{
	struct crypto_user_alg alg = {
		.cru_name = "abcd",
		.cru_driver_name = "efgh",
		.cru_module_name = "dcba",
		.cru_type = 0xabcdfabc,
		.cru_mask = 0xfedabacd,
		.cru_refcnt = 0xbcacfacd,
		.cru_flags = 0xefacdbad
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(alg));

	TEST_NETLINK_OBJECT_EX(fd, nlh0,
			       CRYPTO_MSG_NEWALG, NLM_F_REQUEST,
			       alg, print_quoted_memory,
			       printf("{cru_name=\"abcd\""
				      ", cru_driver_name=\"efgh\""
				      ", cru_module_name=\"dcba\"");
			       PRINT_FIELD_X(", ", alg, cru_type);
			       PRINT_FIELD_X(", ", alg, cru_mask);
			       PRINT_FIELD_U(", ", alg, cru_refcnt);
			       PRINT_FIELD_X(", ", alg, cru_flags);
			       printf("}"));

	fill_memory_ex(alg.cru_name, sizeof(alg.cru_name), '0', 10);
	fill_memory_ex(alg.cru_driver_name, sizeof(alg.cru_driver_name),
		       'a', 'z' - 'a' + 1);
	fill_memory_ex(alg.cru_module_name, sizeof(alg.cru_module_name),
		       'A', 'Z' - 'A' + 1);

	TEST_NETLINK_OBJECT_EX(fd, nlh0,
			       CRYPTO_MSG_NEWALG, NLM_F_REQUEST,
			       alg, print_quoted_memory,
			       printf("{cru_name=");
			       print_quoted_memory(alg.cru_name,
				       sizeof(alg.cru_name) - 1);
			       printf("..., cru_driver_name=");
			       print_quoted_memory(alg.cru_driver_name,
				       sizeof(alg.cru_driver_name) - 1);
			       printf("..., cru_module_name=");
			       print_quoted_memory(alg.cru_module_name,
				       sizeof(alg.cru_module_name) - 1);
			       PRINT_FIELD_X("..., ", alg, cru_type);
			       PRINT_FIELD_X(", ", alg, cru_mask);
			       PRINT_FIELD_U(", ", alg, cru_refcnt);
			       PRINT_FIELD_X(", ", alg, cru_flags);
			       printf("}"));
}

static void
test_crypto_msg_unspec(const int fd)
{
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, 4);

	TEST_NETLINK_(fd, nlh0,
		      0xffff, "0xffff /* CRYPTO_MSG_??? */",
		      NLM_F_REQUEST, "NLM_F_REQUEST",
		      4, "abcd", 4, printf("\"\\x61\\x62\\x63\\x64\""));
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	int fd = create_nl_socket(NETLINK_CRYPTO);

	test_nlmsg_type(fd);
	test_nlmsg_flags(fd);
	test_crypto_msg_newalg(fd);
	test_crypto_msg_unspec(fd);

	printf("+++ exited with 0 +++\n");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_LINUX_CRYPTOUSER_H")

#endif
