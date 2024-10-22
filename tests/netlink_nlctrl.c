/*
 * Copyright (c) 2017-2024 The strace developers.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "xmalloc.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "netlink.h"
#include "test_netlink.h"
#include "test_nlattr.h"
#include <linux/genetlink.h>
#include <linux/cgroupstats.h>
#include <linux/devlink.h>
#include <linux/ethtool_netlink.h>
#include <linux/ioam6_genl.h>
#include <linux/mptcp_pm.h>
#include <linux/netdev.h>
#include <linux/nl80211.h>
#include <linux/seg6_genl.h>
#include <linux/tcp_metrics.h>
#include <linux/thermal.h>

static void
test_hdr(const int fd)
{
	static const struct strval32 cmds[] = {
		{ ARG_STR(CTRL_CMD_UNSPEC) },
		{ ARG_STR(CTRL_CMD_NEWFAMILY) },
		{ ARG_STR(CTRL_CMD_DELFAMILY) },
		{ ARG_STR(CTRL_CMD_GETFAMILY) },
		{ ARG_STR(CTRL_CMD_NEWOPS) },
		{ ARG_STR(CTRL_CMD_DELOPS) },
		{ ARG_STR(CTRL_CMD_GETOPS) },
		{ ARG_STR(CTRL_CMD_NEWMCAST_GRP) },
		{ ARG_STR(CTRL_CMD_DELMCAST_GRP) },
		{ ARG_STR(CTRL_CMD_GETMCAST_GRP) },
		{ ARG_STR(CTRL_CMD_GETPOLICY) },
	};
	struct genlmsghdr hdr = {
		.version = 1
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(hdr));

	for (size_t i = 0; i < ARRAY_SIZE(cmds); ++i) {
		hdr.cmd = cmds[i].val;
		TEST_NETLINK_OBJECT_EX_(fd, nlh0,
					GENL_ID_CTRL, "nlctrl",
					NLM_F_REQUEST | NLM_F_DUMP,
					"NLM_F_REQUEST|NLM_F_DUMP",
					hdr, print_quoted_hex,
					printf("{cmd=%s, version=1}",
					       cmds[i].str);
				       );
	}
}

static void
init_genlmsghdr(struct nlmsghdr *const nlh, const unsigned int msg_len)
{
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = msg_len,
		.nlmsg_type = GENL_ID_CTRL,
		.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP
	);

	struct genlmsghdr *const hdr = NLMSG_DATA(nlh);
	SET_STRUCT(struct genlmsghdr, hdr,
		.cmd = CTRL_CMD_GETFAMILY,
		.version = 2,
		.reserved = 0xfeed
	);
}

static void
print_genlmsghdr(const unsigned int msg_len)
{
	printf("{nlmsg_len=%u, nlmsg_type=%s, nlmsg_flags=%s, nlmsg_seq=0"
	       ", nlmsg_pid=0}, {cmd=%s, version=2, reserved=%#x}",
	       msg_len, "nlctrl", "NLM_F_REQUEST|NLM_F_DUMP",
	       "CTRL_CMD_GETFAMILY", 0xfeed);
}

static void
test_nla_unknown(const int fd, void *const nlh0)
{
	static char pattern[DEFAULT_STRLEN];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);
	const char *nla_type_str = xasprintf("%#x /* CTRL_ATTR_??? */",
					     CTRL_ATTR_OP + 1);

	TEST_NLATTR_(fd, nlh0, sizeof(struct genlmsghdr),
		     init_genlmsghdr, print_genlmsghdr,
		     CTRL_ATTR_OP + 1, nla_type_str,
		     sizeof(pattern), pattern, sizeof(pattern),
		     print_quoted_hex(pattern, sizeof(pattern)));
}

static void
test_nla_x16(const int fd, void *const nlh0)
{
	static const struct strval16 attrs[] = {
		{ ARG_STR(CTRL_ATTR_FAMILY_ID) },
	};
	char pattern[sizeof(uint16_t)];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);

	for (size_t i = 0; i < ARRAY_SIZE(attrs); ++i) {
		check_x16_nlattr(fd, nlh0, sizeof(struct genlmsghdr),
				 init_genlmsghdr, print_genlmsghdr,
				 attrs[i].val, attrs[i].str, pattern, 0);
	}
}

static void
test_nla_u32(const int fd, void *const nlh0)
{
	static const struct strval16 attrs[] = {
		{ ARG_STR(CTRL_ATTR_VERSION) },
		{ ARG_STR(CTRL_ATTR_HDRSIZE) },
		{ ARG_STR(CTRL_ATTR_MAXATTR) },
		{ ARG_STR(CTRL_ATTR_OP) },
	};
	char pattern[sizeof(uint32_t)];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);

	for (size_t i = 0; i < ARRAY_SIZE(attrs); ++i) {
		check_u32_nlattr(fd, nlh0, sizeof(struct genlmsghdr),
				 init_genlmsghdr, print_genlmsghdr,
				 attrs[i].val, attrs[i].str, pattern, 0);
	}
}

static void
test_nla_str(const int fd, void *const nlh0)
{
	static const struct strval16 attrs[] = {
		{ ARG_STR(CTRL_ATTR_FAMILY_NAME) },
	};

	for (size_t i = 0; i < ARRAY_SIZE(attrs); ++i) {
		char str[DEFAULT_STRLEN];

		fill_memory_ex(str, sizeof(str), '0', 10);
	        TEST_NLATTR_(fd, nlh0, sizeof(struct genlmsghdr),
			     init_genlmsghdr, print_genlmsghdr,
			     attrs[i].val, attrs[i].str,
			     sizeof(str), str, sizeof(str),
			     printf("\"%.*s\"...", (int) sizeof(str), str)
			    );

		str[sizeof(str) - 1] = '\0';
	        TEST_NLATTR_(fd, nlh0, sizeof(struct genlmsghdr),
			     init_genlmsghdr, print_genlmsghdr,
			     attrs[i].val, attrs[i].str,
			     sizeof(str), str, sizeof(str),
			     printf("\"%s\"", str)
			    );
	}

}

static void
test_nla_ops(const int fd)
{
	static const struct strval16 attrs[] = {
		{ ARG_STR(CTRL_ATTR_OP_ID) },
		{ ARG_STR(CTRL_ATTR_OP_FLAGS) },
	};
	static const struct strval32 cmds[] = {
		{ ARG_STR(CTRL_CMD_GETFAMILY) },
		{ ARG_STR(CTRL_CMD_GETPOLICY) },
	};
	static const struct strval32 flags[] = {
		{ ARG_STR(GENL_CMD_CAP_DO|GENL_CMD_CAP_DUMP|GENL_CMD_CAP_HASPOL) },
		{ ARG_STR(GENL_ADMIN_PERM|GENL_UNS_ADMIN_PERM) },
	};

	const struct {
		struct nlattr h;
		struct {
			struct nlattr h;
			uint32_t v;
		} a[2];
	} src[] = {
		{
			{ sizeof(src[0]), 1 },
			{
				{
					{ sizeof(src[0].a[0]), attrs[0].val },
					cmds[0].val
				}, {
					{ sizeof(src[0].a[1]), attrs[1].val },
					flags[0].val
				}
			}
		}, {
			{ sizeof(src[1]), 2 },
			{
				{
					{ sizeof(src[1].a[0]), attrs[0].val },
					cmds[1].val
				}, {
					{ sizeof(src[1].a[1]), attrs[1].val },
					flags[1].val
				}
			}
		}
	};
	void *const nlh0 = midtail_alloc(NLMSG_SPACE(sizeof(struct genlmsghdr)),
					 NLA_HDRLEN + sizeof(src));

	TEST_NLATTR(fd, nlh0, sizeof(struct genlmsghdr),
		    init_genlmsghdr, print_genlmsghdr, CTRL_ATTR_OPS,
		    sizeof(src), src, sizeof(src),
		    printf("["
			    "[{nla_len=%u, nla_type=%#x}, "
			     "["
			      "[{nla_len=%u, nla_type=%s}, %s], "
			      "[{nla_len=%u, nla_type=%s}, %s]"
			     "]"
			    "], "
			    "[{nla_len=%u, nla_type=%#x}, "
			     "["
			      "[{nla_len=%u, nla_type=%s}, %s], "
			      "[{nla_len=%u, nla_type=%s}, %s]"
			     "]"
			    "]"
			   "]",
			   src[0].h.nla_len, src[0].h.nla_type,
			   src[0].a[0].h.nla_len, attrs[0].str, cmds[0].str,
			   src[0].a[1].h.nla_len, attrs[1].str, flags[0].str,
			   src[1].h.nla_len, src[1].h.nla_type,
			   src[1].a[0].h.nla_len, attrs[0].str, cmds[1].str,
			   src[1].a[1].h.nla_len, attrs[1].str, flags[1].str)
		    );
}

static void
test_nla_mcast(const int fd)
{
	static const struct strval16 attrs[] = {
		{ ARG_STR(CTRL_ATTR_MCAST_GRP_ID) },
		{ ARG_STR(CTRL_ATTR_MCAST_GRP_NAME) },
	};

	struct {
		struct nlattr h;
		struct {
			struct nlattr h;
			uint32_t v;
		} a0;
		struct {
			struct nlattr h;
			char s[DEFAULT_STRLEN];
		} a1;
	} src[] = {
		{
			{ sizeof(src[0]), 1 },
			{
				{ sizeof(src[0].a0), attrs[0].val },
				0xcafef00d
			}, {
				{ sizeof(src[0].a1), attrs[1].val },
				""
			}
		}, {
			{ sizeof(src[1]), 2 },
			{
				{ sizeof(src[1].a0), attrs[0].val },
				0xdeadface
			}, {
				{ sizeof(src[1].a1), attrs[1].val },
				""
			}
		}
	};

	char strs[2][DEFAULT_STRLEN];
	fill_memory_ex(strs, sizeof(strs), '0', 10);
	strs[0][sizeof(strs[0]) - 1] = '\0';
	strs[1][sizeof(strs[1]) - 1] = '\0';
	memcpy(src[0].a1.s, strs[0], DEFAULT_STRLEN);
	memcpy(src[1].a1.s, strs[1], DEFAULT_STRLEN);

	void *const nlh0 = midtail_alloc(NLMSG_SPACE(sizeof(struct genlmsghdr)),
					 NLA_HDRLEN + sizeof(src));

	TEST_NLATTR(fd, nlh0, sizeof(struct genlmsghdr),
		    init_genlmsghdr, print_genlmsghdr, CTRL_ATTR_MCAST_GROUPS,
		    sizeof(src), src, sizeof(src),
		    printf("["
			    "[{nla_len=%u, nla_type=%#x}, "
			     "["
			      "[{nla_len=%u, nla_type=%s}, %#x], "
			      "[{nla_len=%u, nla_type=%s}, \"%s\"]"
			     "]"
			    "], "
			    "[{nla_len=%u, nla_type=%#x}, "
			     "["
			      "[{nla_len=%u, nla_type=%s}, %#x], "
			      "[{nla_len=%u, nla_type=%s}, \"%s\"]"
			     "]"
			    "]"
			   "]",
			   src[0].h.nla_len, src[0].h.nla_type,
			   src[0].a0.h.nla_len, attrs[0].str, src[0].a0.v,
			   src[0].a1.h.nla_len, attrs[1].str, src[0].a1.s,
			   src[1].h.nla_len, src[1].h.nla_type,
			   src[1].a0.h.nla_len, attrs[0].str, src[1].a0.v,
			   src[1].a1.h.nla_len, attrs[1].str, src[1].a1.s)
		    );
}

static void
test_nla_policy(const int fd)
{
	static const struct strval16 type_attr_type =
		{ ARG_STR(NL_POLICY_TYPE_ATTR_TYPE) };
	static const struct strval32 attr_types[] = {
		{ ARG_STR(NL_ATTR_TYPE_INVALID) },
		{ ARG_STR(NL_ATTR_TYPE_FLAG) },
		{ ARG_STR(NL_ATTR_TYPE_U8) },
		{ ARG_STR(NL_ATTR_TYPE_U16) },
		{ ARG_STR(NL_ATTR_TYPE_U32) },
		{ ARG_STR(NL_ATTR_TYPE_U64) },
		{ ARG_STR(NL_ATTR_TYPE_S8) },
		{ ARG_STR(NL_ATTR_TYPE_S16) },
		{ ARG_STR(NL_ATTR_TYPE_S32) },
		{ ARG_STR(NL_ATTR_TYPE_S64) },
		{ ARG_STR(NL_ATTR_TYPE_BINARY) },
		{ ARG_STR(NL_ATTR_TYPE_STRING) },
		{ ARG_STR(NL_ATTR_TYPE_NUL_STRING) },
		{ ARG_STR(NL_ATTR_TYPE_NESTED) },
		{ ARG_STR(NL_ATTR_TYPE_NESTED_ARRAY) },
		{ ARG_STR(NL_ATTR_TYPE_BITFIELD32) },
		{ ARG_STR(NL_ATTR_TYPE_SINT) },
		{ ARG_STR(NL_ATTR_TYPE_UINT) },
	};
	static const struct {
		struct strval16 name;
		struct strval64 value;
	} type_attrs_64[] = {
		{ { ARG_STR(NL_POLICY_TYPE_ATTR_MIN_VALUE_S) }, { ARG_STR(-2401053088871166255) }, },
		{ { ARG_STR(NL_POLICY_TYPE_ATTR_MAX_VALUE_S) }, { ARG_STR(-2401053088871166254) }, },
		{ { ARG_STR(NL_POLICY_TYPE_ATTR_MIN_VALUE_U) }, { ARG_ULL_STR(16045690984838385363) }, },
		{ { ARG_STR(NL_POLICY_TYPE_ATTR_MAX_VALUE_U) }, { ARG_ULL_STR(16045690984838385364) }, },
		{ { ARG_STR(NL_POLICY_TYPE_ATTR_MASK) }, { ARG_STR(0xdeadbeefdefaced5) }, },
	};
	static const struct {
		struct strval16 name;
		struct strval32 value;
	} type_attrs_32[] = {
		{ { ARG_STR(NL_POLICY_TYPE_ATTR_MIN_LENGTH) }, { ARG_STR(3740978897) }, },
		{ { ARG_STR(NL_POLICY_TYPE_ATTR_MAX_LENGTH) }, { ARG_STR(3740978898) }, },
		{ { ARG_STR(NL_POLICY_TYPE_ATTR_POLICY_IDX) }, { ARG_STR(0xdefaced3) }, },
		{ { ARG_STR(NL_POLICY_TYPE_ATTR_POLICY_MAXTYPE) }, { ARG_STR(0xdefaced4) }, },
		{ { ARG_STR(NL_POLICY_TYPE_ATTR_BITFIELD32_MASK) }, { ARG_STR(0xdefaced5) }, },
	};
	static_assert(ARRAY_SIZE(type_attrs_64) == ARRAY_SIZE(type_attrs_32),
		      "type attrs size mismatch");
	struct {
		struct nlattr h;
		struct {
			struct nlattr h;
			struct {
				struct nlattr h;
				uint64_t v;
			} ATTRIBUTE_PACKED a64;
			struct {
				struct nlattr h;
				uint32_t v;
			} a32;
			struct {
				struct nlattr h;
				uint32_t v;
			} at;
		} a[ARRAY_SIZE(type_attrs_64)];
	} src[ARRAY_SIZE(attr_types)];

	for (unsigned int i = 0; i < ARRAY_SIZE(src); ++i) {
		src[i].h.nla_len = sizeof(src[i]);
		src[i].h.nla_type = NLA_F_NESTED | i;
		for (unsigned int j = 0; j < ARRAY_SIZE(src[i].a); ++j) {
			src[i].a[j].h.nla_len = sizeof(src[i].a[j]);
			src[i].a[j].h.nla_type = NLA_F_NESTED | 0x100 | j;
			src[i].a[j].a64.h.nla_len = sizeof(src[i].a[j].a64);
			src[i].a[j].a64.h.nla_type = type_attrs_64[j].name.val;
			src[i].a[j].a64.v = type_attrs_64[j].value.val;
			src[i].a[j].a32.h.nla_len = sizeof(src[i].a[j].a32);
			src[i].a[j].a32.h.nla_type = type_attrs_32[j].name.val;
			src[i].a[j].a32.v = type_attrs_32[j].value.val;
			src[i].a[j].at.h.nla_len = sizeof(src[i].a[j].at);
			src[i].a[j].at.h.nla_type = type_attr_type.val;
			src[i].a[j].at.v =
				attr_types[(i + j) % ARRAY_SIZE(attr_types)].val;
		}
	}

	void *const nlh0 = midtail_alloc(NLMSG_SPACE(sizeof(struct genlmsghdr)),
					 NLA_HDRLEN + sizeof(src));

	TEST_NLATTR(fd, nlh0, sizeof(struct genlmsghdr),
		    init_genlmsghdr, print_genlmsghdr,
		    NLA_F_NESTED|CTRL_ATTR_POLICY,
		    sizeof(src), src, sizeof(src),
		    for (unsigned int i = 0; i < ARRAY_SIZE(src); ++i) {
			printf("%s", i ? ", " : "[");
			printf("[{nla_len=%u, nla_type=NLA_F_NESTED|%#x}, ",
			       src[i].h.nla_len, i);
			for (unsigned int j = 0; j < ARRAY_SIZE(src[i].a); ++j) {
			    printf("%s", j ? ", " : "[");
			    printf("[{nla_len=%u, nla_type=NLA_F_NESTED|%#x}, "
				    "["
				     "[{nla_len=%u, nla_type=%s}, %s], "
				     "[{nla_len=%u, nla_type=%s}, %s], "
				     "[{nla_len=%u, nla_type=%s}, %s]"
				    "]"
				   "]",
				   src[i].a[j].h.nla_len, 0x100 | j,
				   src[i].a[j].a64.h.nla_len,
				   type_attrs_64[j].name.str,
				   type_attrs_64[j].value.str,
				   src[i].a[j].a32.h.nla_len,
				   type_attrs_32[j].name.str,
				   type_attrs_32[j].value.str,
				   src[i].a[j].at.h.nla_len,
				   type_attr_type.str,
				   attr_types[(i + j) % ARRAY_SIZE(attr_types)].str
				   );
			}
			printf("]]");
		    }
		    printf("]");
		   );
}

static void
test_nla_op_policy(const int fd)
{
	static const struct strval16 attrs[] = {
		{ ARG_STR(CTRL_ATTR_POLICY_DO) },
		{ ARG_STR(CTRL_ATTR_POLICY_DUMP) },
	};
	static const struct strval16 types[] = {
		{ ARG_STR(NLA_F_NESTED|0xfe) },
		{ ARG_STR(NLA_F_NESTED|0xfd) },
	};
	const struct {
		struct nlattr h;
		struct {
			struct nlattr h;
			uint32_t v;
		} a[2];
	} src[] = {
		{
			{ sizeof(src[0]), types[0].val },
			{
				{
					{ sizeof(src[0].a[0]), attrs[0].val },
					0xdefaced1
				}, {
					{ sizeof(src[0].a[1]), attrs[1].val },
					0xdefaced2
				}
			}
		}, {
			{ sizeof(src[1]), types[1].val },
			{
				{
					{ sizeof(src[1].a[0]), attrs[0].val },
					0xdefaced3
				}, {
					{ sizeof(src[1].a[1]), attrs[1].val },
					0xdefaced4
				}
			}
		}
	};
	void *const nlh0 = midtail_alloc(NLMSG_SPACE(sizeof(struct genlmsghdr)),
					 NLA_HDRLEN + sizeof(src));

	TEST_NLATTR(fd, nlh0, sizeof(struct genlmsghdr),
		    init_genlmsghdr, print_genlmsghdr,
		    NLA_F_NESTED|CTRL_ATTR_OP_POLICY,
		    sizeof(src), src, sizeof(src),
		    printf("["
			    "[{nla_len=%u, nla_type=%s}, "
			     "["
			      "[{nla_len=%u, nla_type=%s}, %u], "
			      "[{nla_len=%u, nla_type=%s}, %u]"
			     "]"
			    "], "
			    "[{nla_len=%u, nla_type=%s}, "
			     "["
			      "[{nla_len=%u, nla_type=%s}, %u], "
			      "[{nla_len=%u, nla_type=%s}, %u]"
			     "]"
			    "]"
			   "]",
			   src[0].h.nla_len, types[0].str,
			   src[0].a[0].h.nla_len, attrs[0].str, src[0].a[0].v,
			   src[0].a[1].h.nla_len, attrs[1].str, src[0].a[1].v,
			   src[1].h.nla_len, types[1].str,
			   src[1].a[0].h.nla_len, attrs[0].str, src[1].a[0].v,
			   src[1].a[1].h.nla_len, attrs[1].str, src[1].a[1].v)
		    );
}

static void
test_nla_ops_family(const int fd)
{
	struct ops {
		struct nlattr h;
		struct {
			struct nlattr h;
			struct {
				struct nlattr h;
				uint32_t v;
			} id;
			struct {
				struct nlattr h;
				uint32_t v;
			} flags;
		} a[3];
	};
	static const struct strval16 ctrl_attr_family_name =
		{ ARG_STR(CTRL_ATTR_FAMILY_NAME) };
	static const struct strval16 ctrl_attr_ops =
		{ ARG_STR(CTRL_ATTR_OPS) };
	static const struct strval16 attrs[] = {
		{ ARG_STR(CTRL_ATTR_OP_ID) },
		{ ARG_STR(CTRL_ATTR_OP_FLAGS) },
	};
	static const struct strval32 flags[] = {
		{ ARG_STR(GENL_CMD_CAP_DO|GENL_CMD_CAP_DUMP|GENL_CMD_CAP_HASPOL) },
		{ ARG_STR(GENL_ADMIN_PERM|GENL_CMD_CAP_DO|GENL_CMD_CAP_HASPOL) },
		{ 0xffffffe0, "0xffffffe0 /* GENL_??? */" },
	};

	void *const nlh0 = midtail_alloc(NLMSG_SPACE(sizeof(struct genlmsghdr)),
					 NLA_HDRLEN +
					 sizeof(struct ops) + GENL_NAMSIZ);

#define TEST_NLA_OPS_FAMILY(family_name, cmds)	\
	do {	\
		const struct {	\
			union {	\
				char family[sizeof(family_name)];	\
				char family_a[NLA_ALIGN(sizeof(family_name))];	\
			};	\
			struct ops ops;	\
		} src = {	\
			{	\
				family_name	\
			}, {	\
				{ sizeof(src.ops), ctrl_attr_ops.val }, {	\
					{	\
						{ sizeof(src.ops.a[0]), 1 }, {	\
							{	\
								sizeof(src.ops.a[0].id),	\
								attrs[0].val }	\
							,	\
							cmds[0].val	\
						}, {	\
							{	\
								sizeof(src.ops.a[0].flags),	\
								attrs[1].val	\
							},	\
							flags[0].val	\
						}	\
					}, {	\
						{ sizeof(src.ops.a[1]), 2 }, {	\
							{	\
								sizeof(src.ops.a[1].id),	\
								attrs[0].val	\
							},	\
							cmds[1].val	\
						}, {	\
							{	\
								sizeof(src.ops.a[1].flags),	\
								attrs[1].val	\
							},	\
							flags[1].val	\
						}	\
					}, {	\
						{ sizeof(src.ops.a[2]), 3 }, {	\
							{	\
								sizeof(src.ops.a[2].id),	\
								attrs[0].val	\
							},	\
							cmds[2].val	\
						}, {	\
							{	\
								sizeof(src.ops.a[2].flags),	\
								attrs[1].val	\
							},	\
							flags[2].val	\
						}	\
					}	\
				}	\
			}	\
		};	\
		\
		TEST_NLATTR_EX_(fd, nlh0, sizeof(struct genlmsghdr),	\
				init_genlmsghdr, print_genlmsghdr,	\
				ctrl_attr_family_name.val,	\
				ctrl_attr_family_name.str,	\
				sizeof(src.family), sizeof(src),	\
				&src, sizeof(src),	\
				printf("\"%s\"], "	\
				       "[{nla_len=%u, nla_type=%s}, "	\
				       "["	\
					"[{nla_len=%u, nla_type=%#x}, "	\
					 "["	\
					  "[{nla_len=%u, nla_type=%s}, %s], "	\
					  "[{nla_len=%u, nla_type=%s}, %s]"	\
					 "]"	\
					"], "	\
					"[{nla_len=%u, nla_type=%#x}, "	\
					 "["	\
					  "[{nla_len=%u, nla_type=%s}, %s], "	\
					  "[{nla_len=%u, nla_type=%s}, %s]"	\
					 "]"	\
					"], "	\
					"[{nla_len=%u, nla_type=%#x}, "	\
					 "["	\
					  "[{nla_len=%u, nla_type=%s}, %#x /* %s */], "	\
					  "[{nla_len=%u, nla_type=%s}, %s]"	\
					 "]"	\
					"]"	\
				       "]",	\
				       src.family,	\
				       src.ops.h.nla_len, ctrl_attr_ops.str,	\
				       src.ops.a[0].h.nla_len, src.ops.a[0].h.nla_type,	\
				       src.ops.a[0].id.h.nla_len, attrs[0].str,	\
					cmds[0].str,	\
				       src.ops.a[0].flags.h.nla_len, attrs[1].str,	\
					flags[0].str,	\
				       src.ops.a[1].h.nla_len, src.ops.a[1].h.nla_type,	\
				       src.ops.a[1].id.h.nla_len, attrs[0].str,	\
					cmds[1].str,	\
				       src.ops.a[1].flags.h.nla_len, attrs[1].str,	\
					flags[1].str,	\
				       src.ops.a[2].h.nla_len, src.ops.a[2].h.nla_type,	\
				       src.ops.a[2].id.h.nla_len, attrs[0].str,	\
					cmds[2].val, cmds[2].str,	\
				       src.ops.a[2].flags.h.nla_len, attrs[1].str,	\
					flags[2].str)	\
			       );	\
	} while (0)

	static const struct strval32 devlink_cmds[] = {
		{ ARG_STR(DEVLINK_CMD_GET) },
		{ ARG_STR(DEVLINK_CMD_NOTIFY_FILTER_SET) },
		{ DEVLINK_CMD_NOTIFY_FILTER_SET + 1, "DEVLINK_CMD_???" },
	};
	TEST_NLA_OPS_FAMILY(DEVLINK_GENL_NAME, devlink_cmds);

	static const struct strval32 ethtool_cmds[] = {
		{ ARG_STR(ETHTOOL_MSG_STRSET_GET) },
		{ ARG_STR(ETHTOOL_MSG_PHY_GET) },
		{ ETHTOOL_MSG_PHY_GET + 1, "ETHTOOL_MSG_???" },
	};
	TEST_NLA_OPS_FAMILY(ETHTOOL_GENL_NAME, ethtool_cmds);

	static const struct strval32 ioam6_cmds[] = {
		{ ARG_STR(IOAM6_CMD_ADD_NAMESPACE) },
		{ ARG_STR(IOAM6_CMD_NS_SET_SCHEMA) },
		{ IOAM6_CMD_NS_SET_SCHEMA + 1, "IOAM6_CMD_???" },
	};
	TEST_NLA_OPS_FAMILY(IOAM6_GENL_NAME, ioam6_cmds);

	static const struct strval32 mptcp_pm_cmds[] = {
		{ ARG_STR(MPTCP_PM_CMD_ADD_ADDR) },
		{ ARG_STR(MPTCP_PM_CMD_SUBFLOW_DESTROY) },
		{ MPTCP_PM_CMD_SUBFLOW_DESTROY + 1, "MPTCP_PM_CMD_???" },
	};
	TEST_NLA_OPS_FAMILY(MPTCP_PM_NAME, mptcp_pm_cmds);

	static const struct strval32 netdev_cmds[] = {
		{ ARG_STR(NETDEV_CMD_DEV_GET) },
		{ ARG_STR(NETDEV_CMD_QSTATS_GET) },
		{ NETDEV_CMD_QSTATS_GET + 1, "NETDEV_CMD_???" },
	};
	TEST_NLA_OPS_FAMILY(NETDEV_FAMILY_NAME, netdev_cmds);

	static const struct strval32 nl80211_cmds[] = {
		{ ARG_STR(NL80211_CMD_GET_WIPHY) },
		{ ARG_STR(NL80211_CMD_SET_TID_TO_LINK_MAPPING) },
		{ NL80211_CMD_SET_TID_TO_LINK_MAPPING + 1, "NL80211_CMD_???" },
	};
	TEST_NLA_OPS_FAMILY(NL80211_GENL_NAME, nl80211_cmds);

	static const struct strval32 seg6_cmds[] = {
		{ ARG_STR(SEG6_CMD_SETHMAC) },
		{ ARG_STR(SEG6_CMD_GET_TUNSRC) },
		{ SEG6_CMD_GET_TUNSRC + 1, "SEG6_CMD_???" },
	};
	TEST_NLA_OPS_FAMILY(SEG6_GENL_NAME, seg6_cmds);

	static const struct strval32 taskstats_cmds[] = {
		{ ARG_STR(TASKSTATS_CMD_GET) },
		{ ARG_STR(CGROUPSTATS_CMD_NEW) },
		{ CGROUPSTATS_CMD_NEW + 1, "TASKSTATS_CMD_???" },
	};
	TEST_NLA_OPS_FAMILY(TASKSTATS_GENL_NAME, taskstats_cmds);

	static const struct strval32 tcp_metrics_cmds[] = {
		{ ARG_STR(TCP_METRICS_CMD_GET) },
		{ ARG_STR(TCP_METRICS_CMD_DEL) },
		{ TCP_METRICS_CMD_DEL + 1, "TCP_METRICS_CMD_???" },
	};
	TEST_NLA_OPS_FAMILY(TCP_METRICS_GENL_NAME, tcp_metrics_cmds);

	static const struct strval32 thermal_cmds[] = {
		{ ARG_STR(THERMAL_GENL_CMD_TZ_GET_ID) },
		{ ARG_STR(THERMAL_GENL_CMD_CDEV_GET) },
		{ THERMAL_GENL_CMD_CDEV_GET + 1, "THERMAL_GENL_CMD_???" },
	};
	TEST_NLA_OPS_FAMILY(THERMAL_GENL_FAMILY_NAME, thermal_cmds);
}

static void
test_nlmsg_done(const int fd)
{
	const int num = 0xabcdefad;
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(num));

	TEST_NETLINK(fd, nlh0, NLMSG_DONE, NLM_F_REQUEST,
		     sizeof(num), &num, sizeof(num),
		     printf("%d", num));
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	int fd = create_nl_socket(NETLINK_GENERIC);
	void *const nlh0 = midtail_alloc(NLMSG_SPACE(sizeof(struct genlmsghdr)),
					 NLA_HDRLEN + DEFAULT_STRLEN);

	test_hdr(fd);
	test_nla_unknown(fd, nlh0);
	test_nla_x16(fd, nlh0);
	test_nla_u32(fd, nlh0);
	test_nla_str(fd, nlh0);
	test_nla_ops(fd);
	test_nla_mcast(fd);
	test_nla_policy(fd);
	test_nla_op_policy(fd);
	test_nla_ops_family(fd);
	test_nlmsg_done(fd);

	printf("+++ exited with 0 +++\n");

	return 0;
}
