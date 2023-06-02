/*
 * Copyright (c) 2017-2021 The strace developers.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/* This test case is based on netlink_selinux.c */

#include "tests.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "netlink.h"
#include <linux/genetlink.h>
#include "test_netlink.h"

struct req {
	const struct nlmsghdr nlh;
	struct genlmsghdr gnlh;
};

struct u32_attr {
	struct nlattr nla;
	__u32 value;
};

#define U32_ATTR(name, type, val)		\
	struct u32_attr name = {		\
	.nla = {				\
		.nla_type = type,		\
		.nla_len = sizeof(name)		\
	},					\
		.value = val			\
	};

struct reqnla {
	struct req req;
	struct u32_attr attr;
};

static void
test_nlmsg_type(const int fd)
{
	/*
	 * Though GENL_ID_CTRL number is statically fixed in this test case,
	 * strace does not have a builtin knowledge that the corresponding
	 * string is "nlctrl".
	 */
	long rc;
	struct req req = {
		.nlh = {
			.nlmsg_len = sizeof(req),
			.nlmsg_type = GENL_ID_CTRL,
			.nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST
		},
		.gnlh = {
			.cmd = CTRL_CMD_GETFAMILY
		}
	};

	rc = sendto(fd, &req, sizeof(req), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, [{nlmsg_len=%u, nlmsg_type=nlctrl"
	       ", nlmsg_flags=NLM_F_REQUEST|NLM_F_DUMP, nlmsg_seq=0, nlmsg_pid=0}"
	       ", {cmd=CTRL_CMD_GETFAMILY, version=0}], %u"
	       ", MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, req.nlh.nlmsg_len,
	       (unsigned int) sizeof(req), sprintrc(rc));
}

static void
test_sendmsg_nlmsg_type(const int fd)
{
	long rc;
	struct req req = {
		.nlh = {
			.nlmsg_len = sizeof(req),
			.nlmsg_type = GENL_ID_CTRL,
			.nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST
		},
		.gnlh = {
			.cmd = CTRL_CMD_GETFAMILY
		}
	};

        struct iovec iov[1] = {
		{ .iov_base = &req, .iov_len = sizeof(req) }
        };
        struct msghdr msg = {
		.msg_iov = iov,
		.msg_iovlen = 1
        };

        rc = sendmsg(fd, &msg, MSG_DONTWAIT);
        printf("sendmsg(%d, {msg_name=NULL, msg_namelen=0"
	       ", msg_iov=[{iov_base=[{nlmsg_len=%u, nlmsg_type=nlctrl"
	       ", nlmsg_flags=NLM_F_REQUEST|NLM_F_DUMP, nlmsg_seq=0, nlmsg_pid=0}"
	       ", {cmd=CTRL_CMD_GETFAMILY, version=0}], iov_len=%u}], msg_iovlen=1"
	       ", msg_controllen=0, msg_flags=0}, MSG_DONTWAIT) = %s\n",
	       fd, req.nlh.nlmsg_len, (unsigned int) iov[0].iov_len,
	       sprintrc(rc));
}

static void
test_missing_type(const int fd)
{
	long rc;
	struct req req = {
		.nlh = {
			.nlmsg_len = sizeof(req),
			.nlmsg_type = UINT16_MAX,
			.nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST
		},
		.gnlh = {
			.cmd = CTRL_CMD_GETFAMILY,
			.version = 1
		}
	};

	rc = sendto(fd, &req, sizeof(req), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, [{nlmsg_len=%u, nlmsg_type=0xffff /* GENERIC_FAMILY_??? */"
	       ", nlmsg_flags=NLM_F_REQUEST|NLM_F_DUMP, nlmsg_seq=0, nlmsg_pid=0}"
	       ", {cmd=0x3 /* ??? */, version=1}], %u"
	       ", MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, req.nlh.nlmsg_len,
	       (unsigned int) sizeof(req), sprintrc(rc));

	struct reqnla reqnla = {
		.req = {
			.nlh = {
				.nlmsg_len = sizeof(reqnla),
				.nlmsg_type = UINT16_MAX,
				.nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST
			},
			.gnlh = {
				.cmd = CTRL_CMD_GETFAMILY,
				.version = 1
			}
		},
		.attr = {
			.nla = {
				.nla_type = CTRL_ATTR_OP,
				.nla_len = sizeof(reqnla.attr),
			},
			.value = 0
		}
	};

	rc = sendto(fd, &reqnla, sizeof(reqnla), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, [{nlmsg_len=%u, nlmsg_type=0xffff /* GENERIC_FAMILY_??? */"
	       ", nlmsg_flags=NLM_F_REQUEST|NLM_F_DUMP, nlmsg_seq=0, nlmsg_pid=0}"
	       ", {cmd=0x3 /* ??? */, version=1"
	       ", data=\"\\x08\\x00\\x0a\\x00\\x00\\x00\\x00\\x00\"}]"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, (unsigned int) sizeof(reqnla),
	       (unsigned int) sizeof(reqnla), sprintrc(rc));
}

static void
test_genlmsg_cmds(const int fd)
{
	long rc;
	struct req req = {
		.nlh = {
			.nlmsg_len = sizeof(req),
			.nlmsg_type = GENL_ID_CTRL,
			.nlmsg_flags = NLM_F_REQUEST
		}
	};

	req.gnlh.cmd = CTRL_CMD_GETFAMILY;
	rc = sendto(fd, &req, sizeof(req), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, [{nlmsg_len=%u, nlmsg_type=nlctrl"
	       ", nlmsg_flags=NLM_F_REQUEST, nlmsg_seq=0, nlmsg_pid=0}"
	       ", {cmd=CTRL_CMD_GETFAMILY, version=0}], %u"
	       ", MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, req.nlh.nlmsg_len,
	       (unsigned int) sizeof(req), sprintrc(rc));

	req.gnlh.cmd = CTRL_CMD_NEWFAMILY;
	rc = sendto(fd, &req, sizeof(req), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, [{nlmsg_len=%u, nlmsg_type=nlctrl"
	       ", nlmsg_flags=NLM_F_REQUEST, nlmsg_seq=0, nlmsg_pid=0}"
	       ", {cmd=CTRL_CMD_NEWFAMILY, version=0}], %u"
	       ", MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, req.nlh.nlmsg_len,
	       (unsigned int) sizeof(req), sprintrc(rc));

	req.gnlh.cmd = CTRL_CMD_DELFAMILY;
	rc = sendto(fd, &req, sizeof(req), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, [{nlmsg_len=%u, nlmsg_type=nlctrl"
	       ", nlmsg_flags=NLM_F_REQUEST, nlmsg_seq=0, nlmsg_pid=0}"
	       ", {cmd=CTRL_CMD_DELFAMILY, version=0}], %u"
	       ", MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, req.nlh.nlmsg_len,
	       (unsigned int) sizeof(req), sprintrc(rc));

	req.gnlh.cmd = CTRL_CMD_GETPOLICY;
	rc = sendto(fd, &req, sizeof(req), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, [{nlmsg_len=%u, nlmsg_type=nlctrl"
	       ", nlmsg_flags=NLM_F_REQUEST, nlmsg_seq=0, nlmsg_pid=0}"
	       ", {cmd=CTRL_CMD_GETPOLICY, version=0}], %u"
	       ", MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, req.nlh.nlmsg_len,
	       (unsigned int) sizeof(req), sprintrc(rc));

	req.gnlh.cmd = __CTRL_CMD_MAX;
	rc = sendto(fd, &req, sizeof(req), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, [{nlmsg_len=%u, nlmsg_type=nlctrl"
	       ", nlmsg_flags=NLM_F_REQUEST, nlmsg_seq=0, nlmsg_pid=0}"
	       ", {cmd=0xb /* CTRL_CMD_??? */, version=0}], %u"
	       ", MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, req.nlh.nlmsg_len,
	       (unsigned int) sizeof(req), sprintrc(rc));

	req.gnlh.cmd = CTRL_CMD_UNSPEC;
	rc = sendto(fd, &req, sizeof(req), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, [{nlmsg_len=%u, nlmsg_type=nlctrl"
	       ", nlmsg_flags=NLM_F_REQUEST, nlmsg_seq=0, nlmsg_pid=0}"
	       ", {cmd=CTRL_CMD_UNSPEC, version=0}], %u"
	       ", MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, req.nlh.nlmsg_len,
	       (unsigned int) sizeof(req), sprintrc(rc));

	req.gnlh.cmd = CTRL_CMD_UNSPEC;
	rc = sendto(fd, &req, sizeof(req), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, [{nlmsg_len=%u, nlmsg_type=nlctrl"
	       ", nlmsg_flags=NLM_F_REQUEST, nlmsg_seq=0, nlmsg_pid=0}"
	       ", {cmd=CTRL_CMD_UNSPEC, version=0}], %u"
	       ", MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, req.nlh.nlmsg_len,
	       (unsigned int) sizeof(req), sprintrc(rc));
}

static void
test_nlctrl_getfamily(const int fd)
{
	struct genlmsghdr genl = {
		.cmd = CTRL_CMD_GETFAMILY,
		.version = 1
	};

	const char family[] = "nlctrl";

	struct nlattr nla = {
		.nla_type = CTRL_ATTR_FAMILY_NAME,
		.nla_len = sizeof(nla) + sizeof(family)
	};

	char buf[NLMSG_ALIGN(sizeof(genl) + nla.nla_len)];
	memcpy(buf, &genl, sizeof(genl));
	size_t offset = NLMSG_ALIGN(sizeof(genl));
	memcpy(buf + offset, &nla, sizeof(nla));
	offset += sizeof(nla);
	memcpy(buf + offset, &family, sizeof(family));

	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(buf));

        TEST_NETLINK_(fd, nlh0, GENL_ID_CTRL, "nlctrl", NLM_F_REQUEST,
                      "NLM_F_REQUEST", sizeof(buf), &buf, sizeof(buf),
                      printf("{cmd=CTRL_CMD_GETFAMILY"), printf(", version=1"),
                      printf(", [{nla_len=11, nla_type=CTRL_ATTR_FAMILY_NAME}"),
                      printf(", \"nlctrl\"]}"));
}

static void
test_nlctrl_newfamily(const int fd)
{
	struct genlmsghdr genl = {
		.cmd = CTRL_CMD_NEWFAMILY,
		.version = 2
	};

	const char name[] = "strace";

	struct nlattr family = {
		.nla_type = CTRL_ATTR_FAMILY_NAME,
		.nla_len = sizeof(family) + sizeof(name)
	};

	U32_ATTR(version, CTRL_ATTR_VERSION, 2);
	U32_ATTR(hdrsize, CTRL_ATTR_HDRSIZE, 0);
	U32_ATTR(maxattr, CTRL_ATTR_MAXATTR, 0);

	U32_ATTR(op_id, CTRL_ATTR_OP_ID, 1);
	U32_ATTR(op_flags, CTRL_ATTR_OP_FLAGS, GENL_CMD_CAP_DO | GENL_CMD_CAP_DUMP);

	struct nlattr op_array = {
		.nla_type = 1,
		.nla_len = sizeof(op_array) + sizeof(op_id) + sizeof(op_flags)
	};

	struct nlattr ops = {
		.nla_type = CTRL_ATTR_OPS,
		.nla_len = sizeof(ops) + op_array.nla_len
	};

	U32_ATTR(group_id, CTRL_ATTR_MCAST_GRP_ID, 1);

	struct nlattr group_array = {
		.nla_type = 1,
		.nla_len = sizeof(group_array) + sizeof(group_id)
	};

	struct nlattr mcast_groups = {
		.nla_type = CTRL_ATTR_MCAST_GROUPS,
		.nla_len = sizeof(mcast_groups) + group_array.nla_len
	};

	char buf[NLMSG_ALIGN(sizeof(genl))
		 + NLMSG_ALIGN(family.nla_len)
		 + NLMSG_ALIGN(sizeof(version))
		 + NLMSG_ALIGN(sizeof(hdrsize))
		 + NLMSG_ALIGN(sizeof(maxattr))
		 + NLMSG_ALIGN(ops.nla_len)
		 + NLMSG_ALIGN(mcast_groups.nla_len)
		 ];
	memcpy(buf, &genl, sizeof(genl));
	size_t offset = NLMSG_ALIGN(sizeof(genl));

	memcpy(buf + offset, &family, sizeof(family));
	memcpy(buf + offset + sizeof(family), name, sizeof(name));
	offset += NLMSG_ALIGN(family.nla_len);

	memcpy(buf + offset, &version, sizeof(version));
	offset += NLMSG_ALIGN(sizeof(version));
	memcpy(buf + offset, &hdrsize, sizeof(hdrsize));
	offset += NLMSG_ALIGN(sizeof(hdrsize));
	memcpy(buf + offset, &maxattr, sizeof(maxattr));
	offset += NLMSG_ALIGN(sizeof(maxattr));

	memcpy(buf + offset, &ops, sizeof(ops));
	offset += NLMSG_ALIGN(sizeof(ops));
	memcpy(buf + offset, &op_array, sizeof(op_array));
	offset += NLMSG_ALIGN(sizeof(op_array));
	memcpy(buf + offset, &op_id, sizeof(op_id));
	offset += NLMSG_ALIGN(sizeof(op_id));
	memcpy(buf + offset, &op_flags, sizeof(op_flags));
	offset += NLMSG_ALIGN(sizeof(op_flags));

	memcpy(buf + offset, &mcast_groups, sizeof(mcast_groups));
	offset += NLMSG_ALIGN(sizeof(mcast_groups));
	memcpy(buf + offset, &group_array, sizeof(group_array));
	offset += NLMSG_ALIGN(sizeof(group_array));
	memcpy(buf + offset, &group_id, sizeof(group_id));
	offset += NLMSG_ALIGN(sizeof(group_id));

	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(buf));

        TEST_NETLINK_(fd, nlh0, GENL_ID_CTRL, "nlctrl", NLM_F_REQUEST,
                      "NLM_F_REQUEST", offset, &buf, offset,
		      printf("{cmd=CTRL_CMD_NEWFAMILY, version=2, [["
			     "{nla_len=11, nla_type=CTRL_ATTR_FAMILY_NAME}, "
			     "\"strace\"], "
			     "[{nla_len=8, nla_type=CTRL_ATTR_VERSION}, 2], "
			     "[{nla_len=8, nla_type=CTRL_ATTR_HDRSIZE}, 0], "
			     "[{nla_len=8, nla_type=CTRL_ATTR_MAXATTR}, 0], "
			     "[{nla_len=24, nla_type=CTRL_ATTR_OPS}, "
			     "[{nla_len=20, nla_type=0x1}, "
			     "[[{nla_len=8, nla_type=CTRL_ATTR_OP_ID}, 1], "
			     "[{nla_len=8, nla_type=CTRL_ATTR_OP_FLAGS}, "
			     "GENL_CMD_CAP_DO|GENL_CMD_CAP_DUMP]]]], "
			     "[{nla_len=16, nla_type=CTRL_ATTR_MCAST_GROUPS}, "
			     "[{nla_len=12, nla_type=0x1}, "
			     "[{nla_len=8, nla_type=CTRL_ATTR_MCAST_GRP_ID}, "
			     "1]]]]}"));
}

int main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	int fd = create_nl_socket(NETLINK_GENERIC);

	test_nlmsg_type(fd);
	test_sendmsg_nlmsg_type(fd);
	test_missing_type(fd);
	test_genlmsg_cmds(fd);
	test_nlctrl_getfamily(fd);
	test_nlctrl_newfamily(fd);

	printf("+++ exited with 0 +++\n");

	return 0;
}
