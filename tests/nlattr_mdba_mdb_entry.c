/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#ifdef HAVE_STRUCT_BR_PORT_MSG

# include <stdio.h>
# include "test_nlattr.h"
# include <arpa/inet.h>
# include <linux/if_bridge.h>
# include <linux/rtnetlink.h>

# ifndef MDB_TEMPORARY
#  define MDB_TEMPORARY 0
# endif
# ifndef MDBA_MDB_ENTRY_INFO
#  define MDBA_MDB_ENTRY_INFO 1
# endif
# ifndef MDBA_MDB_EATTR_TIMER
#  define MDBA_MDB_EATTR_TIMER 1
# endif

const unsigned int hdrlen = sizeof(struct br_port_msg);

static void
init_br_port_msg(struct nlmsghdr *const nlh, const unsigned int msg_len)
{
	unsigned int len = msg_len;

	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = len,
		.nlmsg_type = RTM_GETMDB,
		.nlmsg_flags = NLM_F_DUMP
	);

	struct br_port_msg *const msg = NLMSG_DATA(nlh);
	SET_STRUCT(struct br_port_msg, msg,
		.family = AF_UNIX,
		.ifindex = ifindex_lo()
	);

	struct nlattr *nla = NLMSG_ATTR(nlh, sizeof(*msg));
	len -= NLMSG_SPACE(hdrlen);
	SET_STRUCT(struct nlattr, nla,
		.nla_len = len,
		.nla_type = MDBA_MDB
	);

	nla = nla + 1;
	len -= NLA_HDRLEN;
	SET_STRUCT(struct nlattr, nla,
		.nla_len = len,
		.nla_type = MDBA_MDB_ENTRY
	);
}

static void
print_br_port_msg(const unsigned int msg_len)
{
	printf("{len=%u, type=RTM_GETMDB, flags=NLM_F_DUMP"
	       ", seq=0, pid=0}, {family=AF_UNIX"
	       ", ifindex=" IFINDEX_LO_STR "}"
	       ", {{nla_len=%u, nla_type=MDBA_MDB}"
	       ", {{nla_len=%u, nla_type=MDBA_MDB_ENTRY}",
	       msg_len, msg_len - NLMSG_SPACE(hdrlen),
	       msg_len - NLMSG_SPACE(hdrlen) - NLA_HDRLEN);
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	const int fd = create_nl_socket(NETLINK_ROUTE);

	void *nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen), NLA_HDRLEN + 4
# ifdef HAVE_STRUCT_BR_MDB_ENTRY
			- 4 + NLA_HDRLEN * 2 + sizeof(struct nlattr)
			+ sizeof(struct br_mdb_entry)
# endif
			);

	static char pattern[4096];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);

	const unsigned int nla_type = 0xffff & NLA_TYPE_MASK;
	char nla_type_str[256];
	sprintf(nla_type_str, "%#x /* MDBA_MDB_ENTRY_??? */", nla_type);
	TEST_NLATTR_(fd, nlh0 - NLA_HDRLEN * 2, hdrlen + NLA_HDRLEN * 2,
		     init_br_port_msg, print_br_port_msg,
		     nla_type, nla_type_str,
		     4, pattern, 4,
		     print_quoted_hex(pattern, 4);
		     printf("}}"));

# ifdef HAVE_STRUCT_BR_MDB_ENTRY
	struct br_mdb_entry entry = {
		.ifindex = ifindex_lo(),
		.state = MDB_TEMPORARY,
#  ifdef HAVE_STRUCT_BR_MDB_ENTRY_FLAGS
		.flags = MDB_FLAGS_OFFLOAD,
#  endif
#  ifdef HAVE_STRUCT_BR_MDB_ENTRY_VID
		.vid = 0xcdef,
#  endif
		.addr = {
			.proto = htons(AF_UNSPEC)
		}
	};

	memcpy(&entry.addr.u, pattern, sizeof(entry.addr.u));
	TEST_NESTED_NLATTR_OBJECT_EX(fd, nlh0, hdrlen,
				     init_br_port_msg, print_br_port_msg,
				     MDBA_MDB_ENTRY_INFO, pattern, entry, 2,
				     printf("{ifindex=" IFINDEX_LO_STR);
				     printf(", state=MDB_TEMPORARY");
#  ifdef HAVE_STRUCT_BR_MDB_ENTRY_FLAGS
				     printf(", flags=MDB_FLAGS_OFFLOAD");
#  else
				     printf(", flags=0");
#  endif
#  ifdef HAVE_STRUCT_BR_MDB_ENTRY_VID
				     PRINT_FIELD_U(", ", entry, vid);
#  else
				     printf(", vid=0");
#  endif
				     printf(", addr={u=");
				     print_quoted_hex(&entry.addr.u,
						      sizeof(entry.addr.u));
				     printf(", proto=htons(AF_UNSPEC)}}"));

	static const struct nlattr nla = {
		.nla_len = sizeof(nla),
		.nla_type = MDBA_MDB_EATTR_TIMER
	};
	char buf[NLMSG_ALIGN(sizeof(entry)) + sizeof(nla)];
	memcpy(buf, &entry, sizeof(entry));
	memcpy(buf + NLMSG_ALIGN(sizeof(entry)), &nla, sizeof(nla));
	TEST_NLATTR(fd, nlh0 - NLA_HDRLEN * 2, hdrlen + NLA_HDRLEN * 2,
		    init_br_port_msg, print_br_port_msg,
		    MDBA_MDB_ENTRY_INFO, sizeof(buf), buf, sizeof(buf),
		    printf("{ifindex=" IFINDEX_LO_STR);
		    printf(", state=MDB_TEMPORARY");
#  ifdef HAVE_STRUCT_BR_MDB_ENTRY_FLAGS
		    printf(", flags=MDB_FLAGS_OFFLOAD");
#  else
		    printf(", flags=0");
#  endif
#  ifdef HAVE_STRUCT_BR_MDB_ENTRY_VID
		    PRINT_FIELD_U(", ", entry, vid);
#  else
		    printf(", vid=0");
#  endif
		    printf(", addr={u=");
		    print_quoted_hex(&entry.addr.u, sizeof(entry.addr.u));
		    printf(", proto=htons(AF_UNSPEC)}}"
			   ", {nla_len=%u, nla_type=MDBA_MDB_EATTR_TIMER}}}",
			   nla.nla_len));
# endif /* HAVE_STRUCT_BR_MDB_ENTRY */

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_STRUCT_BR_PORT_MSG")

#endif
