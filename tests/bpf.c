/*
 * Check bpf syscall decoding.
 *
 * Copyright (c) 2015-2017 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "scno.h"

#ifdef HAVE_LINUX_BPF_H
# include <linux/bpf.h>
#endif

#include "bpf_attr.h"
#include "print_fields.h"
#include "xmalloc.h"

#include "xlat.h"
#include "xlat/bpf_attach_type.h"
#include "xlat/bpf_commands.h"
#include "xlat/bpf_map_types.h"
#include "xlat/bpf_prog_types.h"
#include "xlat/bpf_test_run_flags.h"

#if defined MPERS_IS_m32 || SIZEOF_KERNEL_LONG_T > 4
# define BIG_ADDR(addr64_, addr32_) addr64_
# define BIG_ADDR_MAYBE(addr_)
#elif defined __arm__ || defined __i386__ || defined __mips__ \
   || defined __powerpc__ || defined __riscv || defined __s390__ \
   || defined __sparc__ || defined __tile__
# define BIG_ADDR(addr64_, addr32_) addr64_ " or " addr32_
# define BIG_ADDR_MAYBE(addr_) addr_ " or "
#else
# define BIG_ADDR(addr64_, addr32_) addr32_
# define BIG_ADDR_MAYBE(addr_)
#endif

#ifndef FD0_PATH
# define FD0_PATH ""
#endif

#ifndef HAVE_STRUCT_BPF_INSN
struct bpf_insn {
	uint8_t	code;
	uint8_t	dst_reg:4;
	uint8_t	src_reg:4;
	int16_t	off;
	int32_t	imm;
};
#endif

#define BPF_ATTR_DATA_FIELD(cmd_) struct cmd_ ## _struct cmd_ ## _data

union bpf_attr_data {
	BPF_ATTR_DATA_FIELD(BPF_MAP_CREATE);
	BPF_ATTR_DATA_FIELD(BPF_MAP_LOOKUP_ELEM);
	BPF_ATTR_DATA_FIELD(BPF_MAP_UPDATE_ELEM);
	BPF_ATTR_DATA_FIELD(BPF_MAP_DELETE_ELEM);
	BPF_ATTR_DATA_FIELD(BPF_MAP_GET_NEXT_KEY);
	BPF_ATTR_DATA_FIELD(BPF_PROG_LOAD);
	BPF_ATTR_DATA_FIELD(BPF_OBJ_PIN);
	BPF_ATTR_DATA_FIELD(BPF_PROG_ATTACH);
	BPF_ATTR_DATA_FIELD(BPF_PROG_DETACH);
	BPF_ATTR_DATA_FIELD(BPF_PROG_TEST_RUN);
	BPF_ATTR_DATA_FIELD(BPF_PROG_GET_NEXT_ID);
	BPF_ATTR_DATA_FIELD(BPF_PROG_GET_FD_BY_ID);
	BPF_ATTR_DATA_FIELD(BPF_MAP_GET_FD_BY_ID);
	BPF_ATTR_DATA_FIELD(BPF_OBJ_GET_INFO_BY_FD);
	BPF_ATTR_DATA_FIELD(BPF_PROG_QUERY);
	BPF_ATTR_DATA_FIELD(BPF_RAW_TRACEPOINT_OPEN);
	BPF_ATTR_DATA_FIELD(BPF_BTF_LOAD);
	BPF_ATTR_DATA_FIELD(BPF_BTF_GET_FD_BY_ID);
	BPF_ATTR_DATA_FIELD(BPF_TASK_FD_QUERY);
	BPF_ATTR_DATA_FIELD(BPF_MAP_FREEZE);
	BPF_ATTR_DATA_FIELD(BPF_MAP_LOOKUP_BATCH);
	BPF_ATTR_DATA_FIELD(BPF_MAP_UPDATE_BATCH);
	BPF_ATTR_DATA_FIELD(BPF_MAP_DELETE_BATCH);
	BPF_ATTR_DATA_FIELD(BPF_LINK_CREATE);
	BPF_ATTR_DATA_FIELD(BPF_LINK_UPDATE);
	BPF_ATTR_DATA_FIELD(BPF_LINK_GET_FD_BY_ID);
	BPF_ATTR_DATA_FIELD(BPF_ENABLE_STATS);
	BPF_ATTR_DATA_FIELD(BPF_ITER_CREATE);
	BPF_ATTR_DATA_FIELD(BPF_LINK_DETACH);
	BPF_ATTR_DATA_FIELD(BPF_PROG_BIND_MAP);
	char char_data[256];
};

struct bpf_attr_check {
	union bpf_attr_data data;
	size_t size;
	size_t iters;
	const char *str;
	void (*init_fn)(struct bpf_attr_check *check, size_t idx);
	void (*print_fn)(const struct bpf_attr_check *check,
			 unsigned long addr, size_t idx);
};

struct bpf_check {
	kernel_ulong_t cmd;
	const char *cmd_str;
	const struct bpf_attr_check *checks;
	size_t count;
};

static const kernel_ulong_t long_bits = (kernel_ulong_t) 0xfacefeed00000000ULL;
static const char *errstr;
static const char *at_fdcwd_str;
static const unsigned int sizeof_attr = sizeof(union bpf_attr_data);
static unsigned int page_size;
static unsigned long end_of_page;

static long
sys_bpf(kernel_ulong_t cmd, kernel_ulong_t attr, kernel_ulong_t size)
{
	long rc = syscall(__NR_bpf, cmd, attr, size);

	errstr = sprintrc(rc);

#ifdef INJECT_RETVAL
	if (rc != INJECT_RETVAL)
		error_msg_and_fail("Got a return value of %ld != %ld",
				   rc, (long) INJECT_RETVAL);

	static char inj_errstr[4096];

	snprintf(inj_errstr, sizeof(inj_errstr), "%s (INJECTED)", errstr);
	errstr = inj_errstr;
#endif

	return rc;
}

#if VERBOSE
# define print_extra_data(addr_, offs_, size_) \
	do { \
		printf("extra_data="); \
		print_quoted_hex((addr_) + (offs_), (size_)); \
		printf(" /* bytes %u..%u */", (offs_), (size_) + (offs_) - 1); \
	} while (0)
#else
# define print_extra_data(addr_, offs_, size_) printf("...")
#endif

static void
print_bpf_attr(const struct bpf_attr_check *check, unsigned long addr,
	       size_t idx)
{
	if (check->print_fn)
		check->print_fn(check, addr, idx);
	else
		printf("%s", check->str);
}

static void
test_bpf(const struct bpf_check *cmd_check)
{
	const struct bpf_attr_check *check = 0;
	const union bpf_attr_data *data = 0;
	unsigned int offset = 0;
	size_t j = 0;

	/* zero addr */
	sys_bpf(cmd_check->cmd, 0, long_bits | sizeof(union bpf_attr_data));
	printf("bpf(%s, NULL, %u) = %s\n",
	       cmd_check->cmd_str, sizeof_attr, errstr);

	/* zero size */
	unsigned long addr = end_of_page - sizeof_attr;
	sys_bpf(cmd_check->cmd, addr, long_bits);
	printf("bpf(%s, %#lx, 0) = %s\n",
	       cmd_check->cmd_str, addr, errstr);

	for (size_t i = 0; i < cmd_check->count; i++) {
		check = &cmd_check->checks[i];
		for (j = 0; j < MAX(check->iters, 1); j++) {
			if (check->init_fn)
				check->init_fn((struct bpf_attr_check *) check, j);
			data = &check->data;
			offset = check->size;

			addr = end_of_page - offset;
			memcpy((void *) addr, data, offset);

			/* starting piece of bpf_attr_data */
			sys_bpf(cmd_check->cmd, addr, offset);
			printf("bpf(%s, {", cmd_check->cmd_str);
			print_bpf_attr(check, addr, j);
			printf("}, %u) = %s\n", offset, errstr);

			/* short read of the starting piece */
			sys_bpf(cmd_check->cmd, addr + 1, offset);
			printf("bpf(%s, %#lx, %u) = %s\n",
			       cmd_check->cmd_str, addr + 1, offset, errstr);
		}
	}

	j = MAX(check->iters, 1) - 1;

	if (offset < sizeof_attr) {
		/* short read of the whole bpf_attr_data */
		memcpy((void *) end_of_page - sizeof_attr + 1, data, offset);
		addr = end_of_page - sizeof_attr + 1;
		memset((void *) addr + offset, 0, sizeof_attr - offset - 1);
		sys_bpf(cmd_check->cmd, addr, sizeof_attr);
		printf("bpf(%s, %#lx, %u) = %s\n",
		       cmd_check->cmd_str, addr, sizeof_attr, errstr);

		/* the whole bpf_attr_data */
		memcpy((void *) end_of_page - sizeof_attr, data, offset);
		addr = end_of_page - sizeof_attr;
		memset((void *) addr + offset, 0, sizeof_attr - offset);
		sys_bpf(cmd_check->cmd, addr, sizeof_attr);
		printf("bpf(%s, {", cmd_check->cmd_str);
		print_bpf_attr(check, addr, j);
		printf("}, %u) = %s\n", sizeof_attr, errstr);

		/* non-zero bytes after the relevant part */
		fill_memory_ex((void *) addr + offset,
			       sizeof_attr - offset, '0', 10);
		sys_bpf(cmd_check->cmd, addr, sizeof_attr);
		printf("bpf(%s, {", cmd_check->cmd_str);
		print_bpf_attr(check, addr, j);
		printf(", ");
		print_extra_data((char *) addr, offset,
				 sizeof_attr - offset);
		printf("}, %u) = %s\n", sizeof_attr, errstr);
	}

	/* short read of the whole page */
	memcpy((void *) end_of_page - page_size + 1, data, offset);
	addr = end_of_page - page_size + 1;
	memset((void *) addr + offset, 0, page_size - offset - 1);
	sys_bpf(cmd_check->cmd, addr, page_size);
	printf("bpf(%s, %#lx, %u) = %s\n",
	       cmd_check->cmd_str, addr, page_size, errstr);

	/* the whole page */
	memcpy((void *) end_of_page - page_size, data, offset);
	addr = end_of_page - page_size;
	memset((void *) addr + offset, 0, page_size - offset);
	sys_bpf(cmd_check->cmd, addr, page_size);
	printf("bpf(%s, {", cmd_check->cmd_str);
	print_bpf_attr(check, addr, j);
	printf("}, %u) = %s\n", page_size, errstr);

	/* non-zero bytes after the whole bpf_attr_data */
	fill_memory_ex((void *) addr + offset,
		       page_size - offset, '0', 10);
	sys_bpf(cmd_check->cmd, addr, page_size);
	printf("bpf(%s, {", cmd_check->cmd_str);
	print_bpf_attr(check, addr, j);
	printf(", ");
	print_extra_data((char *) addr, offset,
			 page_size - offset);
	printf("}, %u) = %s\n", page_size, errstr);

	/* more than a page */
	sys_bpf(cmd_check->cmd, addr, page_size + 1);
	printf("bpf(%s, %#lx, %u) = %s\n",
	       cmd_check->cmd_str, addr, page_size + 1, errstr);
}

static void
init_BPF_MAP_CREATE_attr7(struct bpf_attr_check *check, size_t idx)
{
	struct BPF_MAP_CREATE_struct *attr = &check->data.BPF_MAP_CREATE_data;
	attr->map_ifindex = ifindex_lo();
}

static_assert(ARRAY_SIZE(bpf_map_types_xdata) == 33,
	      "The map_type for tests 1 and 2 below needs to be updated");
static struct bpf_attr_check BPF_MAP_CREATE_checks[] = {
	{
		.data = { .BPF_MAP_CREATE_data = { .map_type = 2 } },
		.size = offsetofend(struct BPF_MAP_CREATE_struct, map_type),
		.str = "map_type=BPF_MAP_TYPE_ARRAY, key_size=0, value_size=0"
		       ", max_entries=0"
	},
	{ /* 1 */
		.data = { .BPF_MAP_CREATE_data = {
			.map_type = 32,
			.key_size = 4,
			.value_size = 8,
			.max_entries = 256,
			.map_flags = 63,
			.inner_map_fd = -1,
			.numa_node = 3141592653,
			.map_name = "0123456789abcde",
		} },
		.size = offsetof(struct BPF_MAP_CREATE_struct, map_name) + 8,
		.str = "map_type=BPF_MAP_TYPE_CGRP_STORAGE, key_size=4"
		       ", value_size=8, max_entries=256"
		       ", map_flags=BPF_F_NO_PREALLOC|BPF_F_NO_COMMON_LRU"
				   "|BPF_F_NUMA_NODE|BPF_F_RDONLY|BPF_F_WRONLY"
				   "|BPF_F_STACK_BUILD_ID"
		       ", inner_map_fd=-1"
		       ", numa_node=3141592653"
		       ", map_name=\"0123456\"...",

	},
	{ /* 2 */
		.data = { .BPF_MAP_CREATE_data = {
			.map_type = 33,
			.key_size = 0xface1e55,
			.value_size = 0xbadc0ded,
			.max_entries = 0xbeefcafe,
			.map_flags = 0xffffc000,
			.inner_map_fd = 2718281828,
			.numa_node = -1,
			.map_name = "",
			.map_ifindex = 3141592653,
		} },
		.size = offsetofend(struct BPF_MAP_CREATE_struct, map_ifindex),
		.str = "map_type=0x21 /* BPF_MAP_TYPE_??? */"
		       ", key_size=4207812181, value_size=3134983661"
		       ", max_entries=3203386110"
		       ", map_flags=0xffffc000 /* BPF_F_??? */"
		       ", inner_map_fd=-1576685468"
		       ", map_name=\"\", map_ifindex=3141592653",

	},
	{ /* 3 */
		.data = { .BPF_MAP_CREATE_data = {
			.map_type = 0xdeadf00d,
			.key_size = 0xface1e55,
			.value_size = 0xbadc0ded,
			.max_entries = 0xbeefcafe,
			.map_flags = 0xc0defead,
			.inner_map_fd = 2718281828,
			.numa_node = -1,
		} },
		.size = offsetofend(struct BPF_MAP_CREATE_struct, map_flags),
		.str = "map_type=0xdeadf00d /* BPF_MAP_TYPE_??? */"
		       ", key_size=4207812181, value_size=3134983661"
		       ", max_entries=3203386110"
		       ", map_flags=BPF_F_NO_PREALLOC|BPF_F_NUMA_NODE"
				   "|BPF_F_RDONLY|BPF_F_STACK_BUILD_ID"
				   "|BPF_F_RDONLY_PROG|BPF_F_CLONE"
				   "|BPF_F_MMAPABLE|BPF_F_PRESERVE_ELEMS"
				   "|BPF_F_INNER_MAP|BPF_F_LINK|0xc0dec000",
	},
	{ /* 4 */
		.data = { .BPF_MAP_CREATE_data = {
			.map_type = 0xdeadf00d,
			.key_size = 0xface1e55,
			.value_size = 0xbadc0ded,
			.max_entries = 0xbeefcafe,
			.map_flags = 0xc0defead,
			.inner_map_fd = 2718281828,
			.numa_node = -1,
		} },
		.size = offsetofend(struct BPF_MAP_CREATE_struct, inner_map_fd),
		.str = "map_type=0xdeadf00d /* BPF_MAP_TYPE_??? */"
		       ", key_size=4207812181, value_size=3134983661"
		       ", max_entries=3203386110"
		       ", map_flags=BPF_F_NO_PREALLOC|BPF_F_NUMA_NODE"
				   "|BPF_F_RDONLY|BPF_F_STACK_BUILD_ID"
				   "|BPF_F_RDONLY_PROG|BPF_F_CLONE"
				   "|BPF_F_MMAPABLE|BPF_F_PRESERVE_ELEMS"
				   "|BPF_F_INNER_MAP|BPF_F_LINK|0xc0dec000"
		       ", inner_map_fd=-1576685468",
	},
	{ /* 5 */
		.data = { .BPF_MAP_CREATE_data = {
			.map_type = 0xdeadf00d,
			.key_size = 0xface1e55,
			.value_size = 0xbadc0ded,
			.max_entries = 0xbeefcafe,
			.map_flags = 0xc0defead,
			.inner_map_fd = 2718281828,
			.numa_node = -1,
		} },
		.size = offsetofend(struct BPF_MAP_CREATE_struct, numa_node),
		.str = "map_type=0xdeadf00d /* BPF_MAP_TYPE_??? */"
		       ", key_size=4207812181, value_size=3134983661"
		       ", max_entries=3203386110"
		       ", map_flags=BPF_F_NO_PREALLOC|BPF_F_NUMA_NODE"
				   "|BPF_F_RDONLY|BPF_F_STACK_BUILD_ID"
				   "|BPF_F_RDONLY_PROG|BPF_F_CLONE"
				   "|BPF_F_MMAPABLE|BPF_F_PRESERVE_ELEMS"
				   "|BPF_F_INNER_MAP|BPF_F_LINK|0xc0dec000"
		       ", inner_map_fd=-1576685468"
		       ", numa_node=4294967295 /* NUMA_NO_NODE */",
	},
	{ /* 6 */
		.data = { .BPF_MAP_CREATE_data = {
			.map_type = 0xdeadf00d,
			.key_size = 0xface1e55,
			.value_size = 0xbadc0ded,
			.max_entries = 0xbeefcafe,
			.map_flags = 0xc0defead,
			.inner_map_fd = 2718281828,
			.numa_node = -1,
			.map_name = "fedcba9876543210",
		} },
		.size = offsetofend(struct BPF_MAP_CREATE_struct, map_name),
		.str = "map_type=0xdeadf00d /* BPF_MAP_TYPE_??? */"
		       ", key_size=4207812181, value_size=3134983661"
		       ", max_entries=3203386110"
		       ", map_flags=BPF_F_NO_PREALLOC|BPF_F_NUMA_NODE"
				   "|BPF_F_RDONLY|BPF_F_STACK_BUILD_ID"
				   "|BPF_F_RDONLY_PROG|BPF_F_CLONE"
				   "|BPF_F_MMAPABLE|BPF_F_PRESERVE_ELEMS"
				   "|BPF_F_INNER_MAP|BPF_F_LINK|0xc0dec000"
		       ", inner_map_fd=-1576685468"
		       ", numa_node=4294967295 /* NUMA_NO_NODE */"
		       ", map_name=\"fedcba987654321\"...",
	},
	{ /* 7 */
		.data = { .BPF_MAP_CREATE_data = {
			.map_type = 0xdeadf00d,
			.key_size = 0xface1e55,
			.value_size = 0xbadc0ded,
			.max_entries = 0xbeefcafe,
			.map_flags = 0xc0defead,
			.inner_map_fd = 2718281828,
			.numa_node = -1,
			.map_name = "0123456789abcde",
		} },
		.size = offsetofend(struct BPF_MAP_CREATE_struct, map_ifindex),
		.str = "map_type=0xdeadf00d /* BPF_MAP_TYPE_??? */"
		       ", key_size=4207812181, value_size=3134983661"
		       ", max_entries=3203386110"
		       ", map_flags=BPF_F_NO_PREALLOC|BPF_F_NUMA_NODE"
				   "|BPF_F_RDONLY|BPF_F_STACK_BUILD_ID"
				   "|BPF_F_RDONLY_PROG|BPF_F_CLONE"
				   "|BPF_F_MMAPABLE|BPF_F_PRESERVE_ELEMS"
				   "|BPF_F_INNER_MAP|BPF_F_LINK|0xc0dec000"
		       ", inner_map_fd=-1576685468"
		       ", numa_node=4294967295 /* NUMA_NO_NODE */"
		       ", map_name=\"0123456789abcde\""
		       ", map_ifindex=" IFINDEX_LO_STR,
		.init_fn = init_BPF_MAP_CREATE_attr7,
	},
	{ /* 8 */
		.data = { .BPF_MAP_CREATE_data = {
			.btf_fd = 0xbadc0ded,
			.btf_key_type_id = 0xfacefeed,
			.btf_value_type_id = 0xcafef00d
		} },
		.size = offsetofend(struct BPF_MAP_CREATE_struct,
				    btf_value_type_id),
		.str = "map_type=BPF_MAP_TYPE_UNSPEC"
		       ", key_size=0"
		       ", value_size=0"
		       ", max_entries=0"
		       ", map_flags=0"
		       ", inner_map_fd=0" FD0_PATH
		       ", map_name=\"\""
		       ", map_ifindex=0"
		       ", btf_fd=-1159983635"
		       ", btf_key_type_id=4207869677"
		       ", btf_value_type_id=3405705229"
	},
	{ /* 9 */
		.data = { .BPF_MAP_CREATE_data = {
			.btf_fd = 0xbadc0ded,
			.btf_key_type_id = 0xfacefeed,
			.btf_value_type_id = 0xcafef00d,
			.btf_vmlinux_value_type_id = 0xdeadc0de,
		} },
		.size = offsetofend(struct BPF_MAP_CREATE_struct,
				    btf_vmlinux_value_type_id),
		.str = "map_type=BPF_MAP_TYPE_UNSPEC"
		       ", key_size=0"
		       ", value_size=0"
		       ", max_entries=0"
		       ", map_flags=0"
		       ", inner_map_fd=0" FD0_PATH
		       ", map_name=\"\""
		       ", map_ifindex=0"
		       ", btf_fd=-1159983635"
		       ", btf_key_type_id=4207869677"
		       ", btf_value_type_id=3405705229"
		       ", btf_vmlinux_value_type_id=3735929054"
	},
	{ /* 10 */
		.data = { .BPF_MAP_CREATE_data = {
			.map_type = BPF_MAP_TYPE_BLOOM_FILTER,
			.map_extra = 4
		} },
		.size = offsetofend(struct BPF_MAP_CREATE_struct, map_extra),
		.str = "map_type=BPF_MAP_TYPE_BLOOM_FILTER"
		       ", key_size=0"
		       ", value_size=0"
		       ", max_entries=0"
		       ", map_flags=0"
		       ", inner_map_fd=0" FD0_PATH
		       ", map_name=\"\""
		       ", map_ifindex=0"
		       ", btf_fd=0" FD0_PATH
		       ", btf_key_type_id=0"
		       ", btf_value_type_id=0"
		       ", btf_vmlinux_value_type_id=0"
		       ", map_extra=4"
	},
};

static const struct bpf_attr_check BPF_MAP_LOOKUP_ELEM_checks[] = {
	{
		.data = { .BPF_MAP_LOOKUP_ELEM_data = { .map_fd = -1 } },
		.size = offsetofend(struct BPF_MAP_LOOKUP_ELEM_struct, map_fd),
		.str = "map_fd=-1, key=NULL, value=NULL"
	},
	{
		.data = { .BPF_MAP_LOOKUP_ELEM_data = {
			.map_fd = -1,
			.key = 0xdeadbeef,
			.value = 0xbadc0ded,
			.flags = 4
		} },
		.size = offsetofend(struct BPF_MAP_LOOKUP_ELEM_struct, flags),
		.str = "map_fd=-1, key=0xdeadbeef, value=0xbadc0ded"
		       ", flags=BPF_F_LOCK"
	}
};

#define BPF_MAP_LOOKUP_AND_DELETE_ELEM_checks BPF_MAP_LOOKUP_ELEM_checks

static const struct bpf_attr_check BPF_MAP_UPDATE_ELEM_checks[] = {
	{
		.data = { .BPF_MAP_UPDATE_ELEM_data = { .map_fd = -1 } },
		.size = offsetofend(struct BPF_MAP_UPDATE_ELEM_struct, map_fd),
		.str = "map_fd=-1, key=NULL, value=NULL, flags=BPF_ANY"
	},
	{
		.data = { .BPF_MAP_UPDATE_ELEM_data = {
			.map_fd = -1,
			.key = 0xdeadbeef,
			.value = 0xbadc0ded,
			.flags = 2
		} },
		.size = offsetofend(struct BPF_MAP_UPDATE_ELEM_struct, flags),
		.str = "map_fd=-1, key=0xdeadbeef, value=0xbadc0ded"
		       ", flags=BPF_EXIST"
	}
};

static const struct bpf_attr_check BPF_MAP_DELETE_ELEM_checks[] = {
	{
		.data = { .BPF_MAP_DELETE_ELEM_data = { .map_fd = -1 } },
		.size = offsetofend(struct BPF_MAP_DELETE_ELEM_struct, map_fd),
		.str = "map_fd=-1, key=NULL"
	},
	{
		.data = { .BPF_MAP_DELETE_ELEM_data = {
			.map_fd = -1,
			.key = 0xdeadbeef
		} },
		.size = offsetofend(struct BPF_MAP_DELETE_ELEM_struct, key),
		.str = "map_fd=-1, key=0xdeadbeef"
	}
};

static const struct bpf_attr_check BPF_MAP_GET_NEXT_KEY_checks[] = {
	{
		.data = { .BPF_MAP_GET_NEXT_KEY_data = { .map_fd = -1 } },
		.size = offsetofend(struct BPF_MAP_GET_NEXT_KEY_struct, map_fd),
		.str = "map_fd=-1, key=NULL, next_key=NULL"
	},
	{
		.data = { .BPF_MAP_GET_NEXT_KEY_data = {
			.map_fd = -1,
			.key = 0xdeadbeef,
			.next_key = 0xbadc0ded
		} },
		.size = offsetofend(struct BPF_MAP_GET_NEXT_KEY_struct, next_key),
		.str = "map_fd=-1, key=0xdeadbeef, next_key=0xbadc0ded"
	}
};

static const struct bpf_attr_check BPF_MAP_FREEZE_checks[] = {
	{
		.data = { .BPF_MAP_FREEZE_data = { .map_fd = -1 } },
		.size = offsetofend(struct BPF_MAP_FREEZE_struct, map_fd),
		.str = "map_fd=-1"
	}
};

static const struct bpf_insn insns[] = {
	{
		.code = 0x95,
		.dst_reg = 10,
		.src_reg = 11,
		.off = 0xdead,
		.imm = 0xbadc0ded,
	},
};
static const char license[] = "GPL";
static const char pathname[] = "/sys/fs/bpf/foo/bar";

static char *log_buf;
/*
 * This has to be a macro, otherwise the compiler complains that
 * initializer element is not constant.
 */
#define log_buf_size 4096U

static char *
get_log_buf(void)
{
	if (!log_buf)
		log_buf = tail_alloc(log_buf_size);
	return log_buf;
}

static char *
get_log_buf_tail(void)
{
	return get_log_buf() + log_buf_size;
}

#if VERBOSE
# define INSNS_FMT \
	"[{code=BPF_JMP|BPF_K|BPF_EXIT, dst_reg=BPF_REG_10" \
	", src_reg=0xb /* BPF_REG_??? */, off=%d, imm=%#x}]"
# define INSNS_ARG insns[0].off, insns[0].imm
#else
# define INSNS_FMT "%p"
# define INSNS_ARG insns
#endif

static void
init_BPF_PROG_LOAD_attr3(struct bpf_attr_check *check, size_t idx)
{
	struct BPF_PROG_LOAD_struct *attr = &check->data.BPF_PROG_LOAD_data;

	attr->insns = (uintptr_t) insns;
	attr->license = (uintptr_t) license;
	attr->log_buf = (uintptr_t) get_log_buf_tail();
}

static void
print_BPF_PROG_LOAD_attr3(const struct bpf_attr_check *check,
			  unsigned long addr, size_t idx)
{
	printf("prog_type=BPF_PROG_TYPE_SOCKET_FILTER, insn_cnt=%u"
	       ", insns=" INSNS_FMT ", license=\"%s\", log_level=2718281828"
	       ", log_size=%u, log_buf=%p"
	       ", kern_version=KERNEL_VERSION(51966, 240, 13)"
	       ", prog_flags=0x100 /* BPF_F_??? */"
	       ", prog_name=\"0123456789abcde\"..., prog_ifindex=3203399405",
	       (unsigned int) ARRAY_SIZE(insns), INSNS_ARG, license,
	       log_buf_size, get_log_buf_tail());
}

static void
init_BPF_PROG_LOAD_attr4(struct bpf_attr_check *check, size_t idx)
{
	struct BPF_PROG_LOAD_struct *attr = &check->data.BPF_PROG_LOAD_data;

	attr->insns = (uintptr_t) insns;
	attr->license = (uintptr_t) license;
	attr->log_buf = (uintptr_t) get_log_buf();
	attr->prog_ifindex = ifindex_lo();

	strncpy(log_buf, "log test", 9);
}

static void
print_BPF_PROG_LOAD_attr4(const struct bpf_attr_check *check,
			  unsigned long addr, size_t idx)
{
	printf("prog_type=BPF_PROG_TYPE_UNSPEC, insn_cnt=%u, insns=" INSNS_FMT
	       ", license=\"%s\", log_level=2718281828, log_size=4"
	       ", log_buf=\"log \"..."
	       ", kern_version=KERNEL_VERSION(51966, 240, 13)"
	       ", prog_flags=BPF_F_STRICT_ALIGNMENT|BPF_F_ANY_ALIGNMENT"
	       "|BPF_F_TEST_RND_HI32|BPF_F_TEST_STATE_FREQ|BPF_F_SLEEPABLE"
	       "|BPF_F_XDP_HAS_FRAGS|BPF_F_XDP_DEV_BOUND_ONLY"
	       "|BPF_F_TEST_REG_INVARIANTS|0x100"
	       ", prog_name=\"0123456789abcde\"..., prog_ifindex=%s"
	       ", expected_attach_type=BPF_CGROUP_INET6_BIND",
	       (unsigned int) ARRAY_SIZE(insns), INSNS_ARG,
	       license, IFINDEX_LO_STR);
}

static_assert(ARRAY_SIZE(bpf_prog_types_xdata) == 33,
	      "The prog_type for tests 1 and 5 below needs to be updated");
static struct bpf_attr_check BPF_PROG_LOAD_checks[] = {
	{
		.data = { .BPF_PROG_LOAD_data = { .prog_type = 1 } },
		.size = offsetofend(struct BPF_PROG_LOAD_struct, prog_type),
		.str = "prog_type=BPF_PROG_TYPE_SOCKET_FILTER"
		       ", insn_cnt=0, insns=NULL, license=NULL"
	},
	{ /* 1 */
		.data = { .BPF_PROG_LOAD_data = {
			.prog_type = 33,
			.insn_cnt = 0xbadc0ded,
			.insns = 0,
			.license = 0,
			.log_level = 42,
			.log_size = 3141592653U,
			.log_buf = 0,
			.kern_version = 0xcafef00d,
			.prog_flags = 0,
		} },
		.size = offsetofend(struct BPF_PROG_LOAD_struct, prog_flags),
		.str = "prog_type=0x21 /* BPF_PROG_TYPE_??? */"
		       ", insn_cnt=3134983661, insns=NULL, license=NULL"
		       ", log_level=42, log_size=3141592653, log_buf=NULL"
		       ", kern_version=KERNEL_VERSION(51966, 240, 13)"
		       ", prog_flags=0",
	},
	{ /* 2 */
		.data = { .BPF_PROG_LOAD_data = {
			.prog_type = 20,
			.insn_cnt = 0xbadc0ded,
			.insns = 0xffffffff00000000,
			.license = 0xffffffff00000000,
			.log_level = 2718281828U,
			.log_size = log_buf_size,
			.log_buf = 0xffffffff00000000,
			.kern_version = 0xcafef00d,
			.prog_flags = 1,
			.prog_name = "fedcba987654321",
		} },
		.size = offsetofend(struct BPF_PROG_LOAD_struct, prog_name),
		.str = "prog_type=BPF_PROG_TYPE_LIRC_MODE2"
		       ", insn_cnt=3134983661"
		       ", insns=" BIG_ADDR("0xffffffff00000000", "NULL")
		       ", license=" BIG_ADDR("0xffffffff00000000", "NULL")
		       ", log_level=2718281828, log_size=4096"
		       ", log_buf=" BIG_ADDR("0xffffffff00000000", "NULL")
		       ", kern_version=KERNEL_VERSION(51966, 240, 13)"
		       ", prog_flags=BPF_F_STRICT_ALIGNMENT"
		       ", prog_name=\"fedcba987654321\"",
	},
	{ /* 3 */
		.data = { .BPF_PROG_LOAD_data = {
			.prog_type = 1,
			.insn_cnt = ARRAY_SIZE(insns),
			.log_level = 2718281828U,
			.log_size = log_buf_size,
			.kern_version = 0xcafef00d,
			.prog_flags = 0x100,
			.prog_name = "0123456789abcdef",
			.prog_ifindex = 0xbeeffeed,
		} },
		.size = offsetofend(struct BPF_PROG_LOAD_struct, prog_ifindex),
		.init_fn = init_BPF_PROG_LOAD_attr3,
		.print_fn = print_BPF_PROG_LOAD_attr3
	},
	{ /* 4 */
		.data = { .BPF_PROG_LOAD_data = {
			.prog_type = 0,
			.insn_cnt = ARRAY_SIZE(insns),
			.log_level = 2718281828U,
			.log_size = 4,
			.kern_version = 0xcafef00d,
			.prog_flags = 0x1ff,
			.prog_name = "0123456789abcdef",
			.expected_attach_type = 9,
		} },
		.size = offsetofend(struct BPF_PROG_LOAD_struct,
				    expected_attach_type),
		.init_fn = init_BPF_PROG_LOAD_attr4,
		.print_fn = print_BPF_PROG_LOAD_attr4
	},
	{ /* 5 */
		.data = { .BPF_PROG_LOAD_data = {
			.prog_type = 32,
			.insn_cnt = 0xbadc0ded,
			.insns = 0xffffffff00000000,
			.license = 0xffffffff00000000,
			.log_level = 2718281828U,
			.log_size = log_buf_size,
			.log_buf = 0xffffffff00000000,
			.kern_version = 0xcafef00d,
			.prog_flags = 1,
			.prog_name = "fedcba987654321",
		} },
		.size = offsetofend(struct BPF_PROG_LOAD_struct, prog_name),
		.str = "prog_type=BPF_PROG_TYPE_NETFILTER"
		       ", insn_cnt=3134983661"
		       ", insns=" BIG_ADDR("0xffffffff00000000", "NULL")
		       ", license=" BIG_ADDR("0xffffffff00000000", "NULL")
		       ", log_level=2718281828, log_size=4096"
		       ", log_buf=" BIG_ADDR("0xffffffff00000000", "NULL")
		       ", kern_version=KERNEL_VERSION(51966, 240, 13)"
		       ", prog_flags=BPF_F_STRICT_ALIGNMENT"
		       ", prog_name=\"fedcba987654321\"",
	},
	{ /* 6 */
		.data = { .BPF_PROG_LOAD_data = {
			.prog_flags = 2,
			.expected_attach_type = 17,
			.prog_btf_fd = 0xbadc0ded,
			.func_info_rec_size = 0xdad1bef2,
			.func_info = 0xfac1fed2fac3fed4,
			.func_info_cnt = 0xdad3bef4,
			.line_info_rec_size = 0xdad5bef6,
			.line_info = 0xfac5fed5fac7fed8,
			.line_info_cnt = 0xdad7bef8,
			.attach_btf_id = 0xdad7befa,
			.attach_prog_fd = 0xbadc0def,
			.fd_array = 0xfaceb00c,
		} },
		.size = offsetofend(struct BPF_PROG_LOAD_struct, fd_array),
		.str = "prog_type=BPF_PROG_TYPE_UNSPEC"
		       ", insn_cnt=0"
		       ", insns=NULL"
		       ", license=NULL"
		       ", log_level=0"
		       ", log_size=0"
		       ", log_buf=NULL"
		       ", kern_version=KERNEL_VERSION(0, 0, 0)"
		       ", prog_flags=BPF_F_ANY_ALIGNMENT"
		       ", prog_name=\"\""
		       ", prog_ifindex=0"
		       ", expected_attach_type=BPF_FLOW_DISSECTOR"
		       ", prog_btf_fd=-1159983635"
		       ", func_info_rec_size=3671178994"
		       ", func_info=0xfac1fed2fac3fed4"
		       ", func_info_cnt=3671310068"
		       ", line_info_rec_size=3671441142"
		       ", line_info=0xfac5fed5fac7fed8"
		       ", line_info_cnt=3671572216"
		       ", attach_btf_id=3671572218"
		       ", attach_prog_fd=-1159983633"
		       ", fd_array=0xfaceb00c"
	},
};

static void
init_BPF_OBJ_PIN_attr(struct bpf_attr_check *check, size_t idx)
{
	struct BPF_OBJ_PIN_struct *attr = &check->data.BPF_OBJ_PIN_data;
	attr->pathname = (uintptr_t) pathname;
}

static void
init_BPF_OBJ_PIN_str(struct bpf_attr_check *check, size_t idx)
{
	check->str = xasprintf("pathname=NULL, bpf_fd=-1, file_flags=BPF_F_PATH_FD"
			       ", path_fd=%s", at_fdcwd_str);
}

static struct bpf_attr_check BPF_OBJ_PIN_checks[] = {
	{
		.data = { .BPF_OBJ_PIN_data = { .pathname = 0 } },
		.size = offsetofend(struct BPF_OBJ_PIN_struct, pathname),
		.str = "pathname=NULL, bpf_fd=0" FD0_PATH
	},
	{
		.data = { .BPF_OBJ_PIN_data = {
			.pathname = 0xFFFFFFFFFFFFFFFFULL
		} },
		.size = offsetofend(struct BPF_OBJ_PIN_struct, pathname),
		.str = "pathname=" BIG_ADDR("0xffffffffffffffff", "0xffffffff")
		       ", bpf_fd=0" FD0_PATH,
	},
	{
		.data = { .BPF_OBJ_PIN_data = { .bpf_fd = -1 } },
		.size = offsetofend(struct BPF_OBJ_PIN_struct, bpf_fd),
		.init_fn = init_BPF_OBJ_PIN_attr,
		.str = "pathname=\"/sys/fs/bpf/foo/bar\", bpf_fd=-1"
	},
	{
		.data = { .BPF_OBJ_PIN_data = {
			.bpf_fd = -1,
			.file_flags = 0x18
		} },
		.size = offsetofend(struct BPF_OBJ_PIN_struct, file_flags),
		.init_fn = init_BPF_OBJ_PIN_attr,
		.str = "pathname=\"/sys/fs/bpf/foo/bar\", bpf_fd=-1"
		       ", file_flags=BPF_F_RDONLY|BPF_F_WRONLY"
	},
	{
		.data = { .BPF_OBJ_PIN_data = {
			.pathname = 0,
			.bpf_fd = -1,
			.file_flags = 0x4000,
			.path_fd = -100
		} },
		.size = offsetofend(struct BPF_OBJ_PIN_struct, path_fd),
		.init_fn = init_BPF_OBJ_PIN_str,
	}
};

#define BPF_OBJ_GET_checks BPF_OBJ_PIN_checks

static const struct bpf_attr_check BPF_PROG_ATTACH_checks[] = {
	{
		.data = { .BPF_PROG_ATTACH_data = { .target_fd = -1 } },
		.size = offsetofend(struct BPF_PROG_ATTACH_struct, target_fd),
		.str = "target_fd=-1, attach_bpf_fd=0" FD0_PATH
		       ", attach_type=BPF_CGROUP_INET_INGRESS, attach_flags=0"
	},
	{
		.data = { .BPF_PROG_ATTACH_data = {
			.target_fd = -1,
			.attach_bpf_fd = -2,
			.attach_type = 2,
			.attach_flags = 7
		} },
		.size = offsetofend(struct BPF_PROG_ATTACH_struct, attach_flags),
		.str = "target_fd=-1, attach_bpf_fd=-2"
		       ", attach_type=BPF_CGROUP_INET_SOCK_CREATE"
		       ", attach_flags=BPF_F_ALLOW_OVERRIDE|BPF_F_ALLOW_MULTI"
				       "|BPF_F_REPLACE"
	},
	{
		.data = { .BPF_PROG_ATTACH_data = {
			.target_fd = -1,
			.attach_bpf_fd = -2,
			.attach_type = 2,
			.attach_flags = 0xdfc0,
			.replace_bpf_fd = -3,
		} },
		.size = offsetofend(struct BPF_PROG_ATTACH_struct, replace_bpf_fd),
		.str = "target_fd=-1, attach_bpf_fd=-2"
		       ", attach_type=BPF_CGROUP_INET_SOCK_CREATE"
		       ", attach_flags=0xdfc0 /* BPF_F_??? */"
		       ", replace_bpf_fd=-3"
	},
};


static const struct bpf_attr_check BPF_PROG_DETACH_checks[] = {
	{
		.data = { .BPF_PROG_DETACH_data = { .target_fd = -1 } },
		.size = offsetofend(struct BPF_PROG_DETACH_struct, target_fd),
		.str = "target_fd=-1, attach_type=BPF_CGROUP_INET_INGRESS"
	},
	{
		.data = { .BPF_PROG_DETACH_data = {
			.target_fd = -1,
			.attach_type = 2
		} },
		.size = offsetofend(struct BPF_PROG_DETACH_struct, attach_type),
		.str = "target_fd=-1, attach_type=BPF_CGROUP_INET_SOCK_CREATE"
	}
};

static const struct bpf_attr_check BPF_PROG_TEST_RUN_checks[] = {
	{
		.data = { .BPF_PROG_TEST_RUN_data = { .prog_fd = -1 } },
		.size = offsetofend(struct BPF_PROG_TEST_RUN_struct, prog_fd),
		.str = "test={prog_fd=-1, retval=0, data_size_in=0"
		       ", data_size_out=0, data_in=NULL, data_out=NULL"
		       ", repeat=0, duration=0}"
	},
	{
		.data = { .BPF_PROG_TEST_RUN_data = {
			.prog_fd = -1,
			.retval = 0xfac1fed2,
			.data_size_in = 0xfac3fed4,
			.data_size_out = 0xfac5fed6,
			.data_in = (uint64_t) 0xfacef11dbadc2dedULL,
			.data_out = (uint64_t) 0xfacef33dbadc4dedULL,
			.repeat = 0xfac7fed8,
			.duration = 0xfac9feda
		} },
		.size = offsetofend(struct BPF_PROG_TEST_RUN_struct, duration),
		.str = "test={prog_fd=-1, retval=4207017682"
		       ", data_size_in=4207148756, data_size_out=4207279830"
		       ", data_in=0xfacef11dbadc2ded"
		       ", data_out=0xfacef33dbadc4ded"
		       ", repeat=4207410904, duration=4207541978}"
	},
	{
		.data = { .BPF_PROG_TEST_RUN_data = {
			.prog_fd = -1,
			.retval = 0xfac1fed2,
			.data_size_in = 0xfac3fed4,
			.data_size_out = 0xfac5fed6,
			.data_in = (uint64_t) 0xfacef11dbadc2dedULL,
			.data_out = (uint64_t) 0xfacef33dbadc4dedULL,
			.repeat = 0xfac7fed8,
			.duration = 0xfac9feda,
			.ctx_size_in = 0,
			.ctx_size_out = 0xfacdfede,
			.ctx_in = (uint64_t) 0xfacef55dbadc6dedULL,
		} },
		.size = offsetofend(struct BPF_PROG_TEST_RUN_struct, ctx_in),
		.str = "test={prog_fd=-1, retval=4207017682"
		       ", data_size_in=4207148756, data_size_out=4207279830"
		       ", data_in=0xfacef11dbadc2ded"
		       ", data_out=0xfacef33dbadc4ded"
		       ", repeat=4207410904"
		       ", duration=4207541978"
		       ", ctx_size_in=0, ctx_size_out=4207804126"
		       ", ctx_in=0xfacef55dbadc6ded, ctx_out=NULL}"
	},
	{
		.data = { .BPF_PROG_TEST_RUN_data = {
			.prog_fd = -1,
			.retval = 0xfac1fed2,
			.data_size_in = 0xfac3fed4,
			.data_size_out = 0xfac5fed6,
			.data_in = (uint64_t) 0xfacef11dbadc2dedULL,
			.data_out = (uint64_t) 0xfacef33dbadc4dedULL,
			.repeat = 0xfac7fed8,
			.duration = 0xfac9feda,
			.ctx_size_in = 0xfacbfedc,
			.ctx_size_out = 0xfacdfede,
			.ctx_in = (uint64_t) 0xfacef55dbadc6dedULL,
			.ctx_out = (uint64_t) 0xfacef77dbadc8dedULL,
			.flags = BPF_F_TEST_RUN_ON_CPU|BPF_F_TEST_XDP_LIVE_FRAMES,
			.cpu = 0,
		} },
		.size = offsetofend(struct BPF_PROG_TEST_RUN_struct, cpu),
		.str = "test={prog_fd=-1, retval=4207017682"
		       ", data_size_in=4207148756, data_size_out=4207279830"
		       ", data_in=0xfacef11dbadc2ded"
		       ", data_out=0xfacef33dbadc4ded"
		       ", repeat=4207410904"
		       ", duration=4207541978"
		       ", ctx_size_in=4207673052"
		       ", ctx_size_out=4207804126"
		       ", ctx_in=0xfacef55dbadc6ded"
		       ", ctx_out=0xfacef77dbadc8ded"
		       ", flags=BPF_F_TEST_RUN_ON_CPU|BPF_F_TEST_XDP_LIVE_FRAMES"
		       ", cpu=0}"
	},
	{
		.data = { .BPF_PROG_TEST_RUN_data = {
			.prog_fd = -1,
			.retval = 0xfac1fed2,
			.data_size_in = 0xfac3fed4,
			.data_size_out = 0xfac5fed6,
			.data_in = (uint64_t) 0xfacef11dbadc2dedULL,
			.data_out = (uint64_t) 0xfacef33dbadc4dedULL,
			.repeat = 0xfac7fed8,
			.duration = 0xfac9feda,
			.ctx_size_in = 0,
			.ctx_size_out = 0,
			.ctx_in = 0,
			.ctx_out = 0,
			.flags = 0xfffffffc,
			.cpu = 3141592653,
			.batch_size = 2718281828,
		} },
		.size = offsetofend(struct BPF_PROG_TEST_RUN_struct, batch_size),
		.str = "test={prog_fd=-1, retval=4207017682"
		       ", data_size_in=4207148756, data_size_out=4207279830"
		       ", data_in=0xfacef11dbadc2ded"
		       ", data_out=0xfacef33dbadc4ded"
		       ", repeat=4207410904"
		       ", duration=4207541978"
		       ", ctx_size_in=0, ctx_size_out=0"
		       ", ctx_in=NULL, ctx_out=NULL"
		       ", flags=0xfffffffc /* BPF_F_??? */"
		       ", cpu=3141592653, batch_size=2718281828}"
	},
};

static const struct bpf_attr_check BPF_PROG_GET_NEXT_ID_checks[] = {
	{
		.data = { .BPF_PROG_GET_NEXT_ID_data = {
			.start_id = 0xdeadbeef
		} },
		.size = offsetofend(struct BPF_PROG_GET_NEXT_ID_struct, start_id),
		.str = "start_id=3735928559, next_id=0"
	},
	{
		.data = { .BPF_PROG_GET_NEXT_ID_data = {
			.start_id = 0xdeadbeef
		} },
		.size = 1,
		/*                        0xde000000 0x000000ef */
		.str = "start_id=" BE_LE("3724541952", "239") ", next_id=0"
	},
	{
		.data = { .BPF_PROG_GET_NEXT_ID_data = {
			.start_id = 0xbadc0ded,
			.next_id = 0xcafef00d
		} },
		.size = offsetofend(struct BPF_PROG_GET_NEXT_ID_struct, next_id),
		.str = "start_id=3134983661, next_id=3405705229"
	},
	{
		.data = { .BPF_PROG_GET_NEXT_ID_data = {
			.start_id = 0xbadc0ded,
			.next_id = 0xcafef00d,
			.open_flags = 0xffffff27
		} },
		.size = offsetofend(struct BPF_PROG_GET_NEXT_ID_struct, open_flags),
		.str = "start_id=3134983661, next_id=3405705229"
		       ", open_flags=0xffffff27 /* BPF_F_??? */"
	}
};

#define BPF_MAP_GET_NEXT_ID_checks BPF_PROG_GET_NEXT_ID_checks
#define BPF_BTF_GET_NEXT_ID_checks BPF_PROG_GET_NEXT_ID_checks
#define BPF_LINK_GET_NEXT_ID_checks BPF_PROG_GET_NEXT_ID_checks

static const struct bpf_attr_check BPF_PROG_GET_FD_BY_ID_checks[] = {
	{
		.data = { .BPF_PROG_GET_FD_BY_ID_data = {
			.prog_id = 0xdeadbeef
		} },
		.size = offsetofend(struct BPF_PROG_GET_FD_BY_ID_struct, prog_id),
		.str = "prog_id=3735928559, next_id=0"
	},
	{
		.data = { .BPF_PROG_GET_FD_BY_ID_data = {
			.prog_id = 0xbadc0ded,
			.next_id = 0xcafef00d
		} },
		.size = offsetofend(struct BPF_PROG_GET_FD_BY_ID_struct, next_id),
		.str = "prog_id=3134983661, next_id=3405705229"
	},
	{
		.data = { .BPF_PROG_GET_FD_BY_ID_data = {
			.prog_id = 0xbadc0ded,
			.next_id = 0xcafef00d,
			.open_flags = 0xffffff27
		} },
		.size = offsetofend(struct BPF_PROG_GET_FD_BY_ID_struct, open_flags),
		.str = "prog_id=3134983661, next_id=3405705229"
		       ", open_flags=0xffffff27 /* BPF_F_??? */"
	}
};

static const struct bpf_attr_check BPF_MAP_GET_FD_BY_ID_checks[] = {
	{
		.data = { .BPF_MAP_GET_FD_BY_ID_data = {
			.map_id = 0xdeadbeef
		} },
		.size = offsetofend(struct BPF_MAP_GET_FD_BY_ID_struct, map_id),
		.str = "map_id=3735928559, next_id=0"
	},
	{
		.data = { .BPF_MAP_GET_FD_BY_ID_data = {
			.map_id = 0xbadc0ded,
			.next_id = 0xcafef00d
		} },
		.size = offsetofend(struct BPF_MAP_GET_FD_BY_ID_struct, next_id),
		.str = "map_id=3134983661, next_id=3405705229"
	},
	{
		.data = { .BPF_MAP_GET_FD_BY_ID_data = {
			.map_id = 0xbadc0ded,
			.next_id = 0xcafef00d,
			.open_flags = 0xffffff27
		} },
		.size = offsetofend(struct BPF_MAP_GET_FD_BY_ID_struct, open_flags),
		.str = "map_id=3134983661, next_id=3405705229"
		       ", open_flags=0xffffff27 /* BPF_F_??? */"
	}
};

static const struct bpf_attr_check BPF_OBJ_GET_INFO_BY_FD_checks[] = {
	{
		.data = { .BPF_OBJ_GET_INFO_BY_FD_data = { .bpf_fd = -1 } },
		.size = offsetofend(struct BPF_OBJ_GET_INFO_BY_FD_struct, bpf_fd),
		.str = "info={bpf_fd=-1, info_len=0, info=NULL}"
	},
	{
		.data = { .BPF_OBJ_GET_INFO_BY_FD_data = {
			.bpf_fd = -1,
			.info_len = 0xdeadbeef,
			.info = (uint64_t) 0xfacefeedbadc0dedULL
		} },
		.size = offsetofend(struct BPF_OBJ_GET_INFO_BY_FD_struct, info),
		.str = "info={bpf_fd=-1, info_len=3735928559"
		       ", info=0xfacefeedbadc0ded}"
	}
};


static uint32_t prog_load_ids[] = { 0, 1, 0xffffffff, 2718281828, };
uint32_t *prog_load_ids_ptr;

static void
init_BPF_PROG_QUERY_attr4(struct bpf_attr_check *check, size_t idx)
{
	struct BPF_PROG_QUERY_struct *attr = &check->data.BPF_PROG_QUERY_data;

	if (!prog_load_ids_ptr)
		prog_load_ids_ptr = tail_memdup(prog_load_ids,
						sizeof(prog_load_ids));

	attr->prog_ids = (uintptr_t) prog_load_ids_ptr;
	attr->prog_cnt = ARRAY_SIZE(prog_load_ids);
}

static void
print_BPF_PROG_QUERY_attr4(const struct bpf_attr_check *check,
			   unsigned long addr, size_t idx)
{
	printf("query={target_fd=-1153374643"
	       ", attach_type=0xfeedface /* BPF_??? */"
	       ", query_flags=BPF_F_QUERY_EFFECTIVE|0xdeadf00c"
	       ", attach_flags=BPF_F_ALLOW_MULTI|BPF_F_REPLACE"
	       "|BPF_F_BEFORE|BPF_F_AFTER|BPF_F_ID|0xbeefcac0"
#if defined(INJECT_RETVAL)
	       ", prog_ids=[0, 1, 4294967295, 2718281828], prog_cnt=4}"
#else
	       ", prog_ids=%p, prog_cnt=4}", prog_load_ids_ptr
#endif
	       );
}

static void
init_BPF_PROG_QUERY_attr5(struct bpf_attr_check *check, size_t idx)
{
	struct BPF_PROG_QUERY_struct *attr = &check->data.BPF_PROG_QUERY_data;

	if (!prog_load_ids_ptr)
		prog_load_ids_ptr = tail_memdup(prog_load_ids,
						sizeof(prog_load_ids));

	attr->prog_ids = (uintptr_t) prog_load_ids_ptr;
	attr->prog_cnt = ARRAY_SIZE(prog_load_ids) + 1;
}

static void
print_BPF_PROG_QUERY_attr5(const struct bpf_attr_check *check,
			   unsigned long addr, size_t idx)
{
	printf("query={target_fd=-1153374643"
	       ", attach_type=0xfeedface /* BPF_??? */"
	       ", query_flags=BPF_F_QUERY_EFFECTIVE|0xdeadf00c"
	       ", attach_flags=BPF_F_ALLOW_MULTI|BPF_F_REPLACE"
	       "|BPF_F_BEFORE|BPF_F_AFTER|BPF_F_ID|0xbeefcac0"
#if defined(INJECT_RETVAL)
	       ", prog_ids=[0, 1, 4294967295, 2718281828, ... /* %p */]"
	       ", prog_cnt=5}",
	       prog_load_ids_ptr + ARRAY_SIZE(prog_load_ids)
#else
	       ", prog_ids=%p, prog_cnt=5}", prog_load_ids_ptr
#endif
	       );
}

static struct bpf_attr_check BPF_PROG_QUERY_checks[] = {
	{
		.data = { .BPF_PROG_QUERY_data = { .target_fd = -1 } },
		.size = offsetofend(struct BPF_PROG_QUERY_struct, target_fd),
		.str = "query={target_fd=-1"
		       ", attach_type=BPF_CGROUP_INET_INGRESS, query_flags=0"
		       ", attach_flags=0, prog_ids=NULL, prog_cnt=0}",
	},
	{ /* 1 */
		.data = { .BPF_PROG_QUERY_data = {
			.target_fd = 3141592653U,
			.attach_type = 55,
			.query_flags = 1,
			.attach_flags = 3,
		} },
		.size = offsetofend(struct BPF_PROG_QUERY_struct, attach_flags),
		.str = "query={target_fd=-1153374643"
		       ", attach_type=BPF_NETKIT_PEER"
		       ", query_flags=BPF_F_QUERY_EFFECTIVE"
		       ", attach_flags=BPF_F_ALLOW_OVERRIDE|BPF_F_ALLOW_MULTI"
		       ", prog_ids=NULL, prog_cnt=0}",
	},
	{ /* 2 */
		.data = { .BPF_PROG_QUERY_data = {
			.target_fd = 3141592653U,
			.attach_type = 56,
			.query_flags = 0xfffffffe,
			.attach_flags = 0xffffdfc0,
			.prog_ids = 0xffffffffffffffffULL,
			.prog_cnt = 2718281828,
		} },
		.size = offsetofend(struct BPF_PROG_QUERY_struct, prog_cnt),
		.str = "query={target_fd=-1153374643"
		       ", attach_type=0x38 /* BPF_??? */"
		       ", query_flags=0xfffffffe /* BPF_F_QUERY_??? */"
		       ", attach_flags=0xffffdfc0 /* BPF_F_??? */"
		       ", prog_ids="
		       BIG_ADDR("0xffffffffffffffff", "0xffffffff")
		       ", prog_cnt=2718281828}",
	},
	{ /* 3 */
		.data = { .BPF_PROG_QUERY_data = {
			.target_fd = 3141592653U,
			.attach_type = 0xfeedface,
			.query_flags = 0xdeadf00d,
			.attach_flags = 0xbeef203f,
			.prog_ids = 0xffffffffffffffffULL,
			.prog_cnt = 0,
		} },
		.size = offsetofend(struct BPF_PROG_QUERY_struct, prog_cnt),
		.str = "query={target_fd=-1153374643"
		       ", attach_type=0xfeedface /* BPF_??? */"
		       ", query_flags=BPF_F_QUERY_EFFECTIVE|0xdeadf00c"
		       ", attach_flags=BPF_F_ALLOW_OVERRIDE|BPF_F_ALLOW_MULTI"
		       "|BPF_F_REPLACE|BPF_F_BEFORE|BPF_F_AFTER|BPF_F_ID"
		       "|BPF_F_LINK|0xbeef0000"
		       ", prog_ids=" BIG_ADDR_MAYBE("0xffffffffffffffff") "[]"
		       ", prog_cnt=0}",
	},
	{ /* 4 */
		.data = { .BPF_PROG_QUERY_data = {
			.target_fd = 3141592653U,
			.attach_type = 0xfeedface,
			.query_flags = 0xdeadf00d,
			.attach_flags = 0xbeefcafe,
		} },
		.size = offsetofend(struct BPF_PROG_QUERY_struct, prog_cnt),
		.init_fn = init_BPF_PROG_QUERY_attr4,
		.print_fn = print_BPF_PROG_QUERY_attr4,
	},
	{ /* 5 */
		.data = { .BPF_PROG_QUERY_data = {
			.target_fd = 3141592653U,
			.attach_type = 0xfeedface,
			.query_flags = 0xdeadf00d,
			.attach_flags = 0xbeefcafe,
		} },
		.size = offsetofend(struct BPF_PROG_QUERY_struct, prog_cnt),
		.init_fn = init_BPF_PROG_QUERY_attr5,
		.print_fn = print_BPF_PROG_QUERY_attr5,
	},
};


static void
init_BPF_RAW_TRACEPOINT_attr2(struct bpf_attr_check *check, size_t idx)
{
	/* TODO: test the 128 byte limit */
	static const char tp_name[] = "0123456789qwertyuiop0123456789qwe";

	struct BPF_RAW_TRACEPOINT_OPEN_struct *attr =
		&check->data.BPF_RAW_TRACEPOINT_OPEN_data;

	attr->name = (uintptr_t) tp_name;
}

static struct bpf_attr_check BPF_RAW_TRACEPOINT_OPEN_checks[] = {
	{
		.data = { .BPF_RAW_TRACEPOINT_OPEN_data = { .name = 0 } },
		.size = offsetofend(struct BPF_RAW_TRACEPOINT_OPEN_struct,
				    name),
		.str = "raw_tracepoint={name=NULL, prog_fd=0" FD0_PATH "}",
	},
	{ /* 1 */
		.data = { .BPF_RAW_TRACEPOINT_OPEN_data = {
			.name = 0xffffffff00000000ULL,
			.prog_fd = 0xdeadbeef,
		} },
		.size = offsetofend(struct BPF_RAW_TRACEPOINT_OPEN_struct,
				    prog_fd),
		.str = "raw_tracepoint="
		       "{name=" BIG_ADDR("0xffffffff00000000", "NULL")
		       ", prog_fd=-559038737}",
	},
	{
		.data = { .BPF_RAW_TRACEPOINT_OPEN_data = {
			.prog_fd = 0xdeadbeef,
		} },
		.size = offsetofend(struct BPF_RAW_TRACEPOINT_OPEN_struct,
				    prog_fd),
		.init_fn = init_BPF_RAW_TRACEPOINT_attr2,
		.str = "raw_tracepoint="
		       "{name=\"0123456789qwertyuiop0123456789qw\"..."
		       ", prog_fd=-559038737}",
	}
};

static void
init_BPF_BTF_LOAD_attr(struct bpf_attr_check *check, size_t idx)
{
	static const char sample_btf_data[] = "bPf\0daTum";

	static char *btf_data;
	if (!btf_data)
		btf_data = tail_memdup(sample_btf_data,
				       sizeof(sample_btf_data) - 1);

	struct BPF_BTF_LOAD_struct *attr = &check->data.BPF_BTF_LOAD_data;
	attr->btf = (uintptr_t) btf_data;
}

static struct bpf_attr_check BPF_BTF_LOAD_checks[] = {
	{
		.data = { .BPF_BTF_LOAD_data = { .btf = 0 } },
		.size = offsetofend(struct BPF_BTF_LOAD_struct, btf),
		.str = "btf=NULL, btf_log_buf=NULL, btf_size=0"
		       ", btf_log_size=0, btf_log_level=0"
	},
	{ /* 1 */
		.data = { .BPF_BTF_LOAD_data = {
			.btf_log_buf = 0xfacefeeddeadbeefULL,
			.btf_size = 9,
			.btf_log_size = -1U,
			.btf_log_level = 42
		} },
		.size = offsetofend(struct BPF_BTF_LOAD_struct, btf_log_level),
		.init_fn = init_BPF_BTF_LOAD_attr,
		.str = "btf=\"bPf\\0daTum\""
		       ", btf_log_buf=0xfacefeeddeadbeef"
		       ", btf_size=9"
		       ", btf_log_size=4294967295"
		       ", btf_log_level=42"
	}
};

static const struct bpf_attr_check BPF_BTF_GET_FD_BY_ID_checks[] = {
	{
		.data = { .BPF_BTF_GET_FD_BY_ID_data = { .btf_id = 0xdeadbeef } },
		.size = offsetofend(struct BPF_BTF_GET_FD_BY_ID_struct, btf_id),
		.str = "btf_id=3735928559"
	}
};

static const struct bpf_attr_check BPF_TASK_FD_QUERY_checks[] = {
	{
		.data = { .BPF_TASK_FD_QUERY_data = { .pid = 1735928559 } },
		.size = offsetofend(struct BPF_TASK_FD_QUERY_struct, pid),
		.str = "task_fd_query={pid=1735928559, fd=0" FD0_PATH
		       ", flags=0, buf_len=0, buf=NULL, prog_id=0"
		       ", fd_type=BPF_FD_TYPE_RAW_TRACEPOINT"
		       ", probe_offset=0, probe_addr=0}"
	},
	{ /* 1 */
		.data = { .BPF_TASK_FD_QUERY_data = {
			.pid = 1405705229,
			.fd = 0xdeadbeef,
			.flags = 0xfacefeed,
			.buf_len = 0xdefaced,
			.buf = 0xfffffffffffffffe,
			.prog_id = 0xbadc0ded,
			.fd_type = 5,
			.probe_offset = 0xfac1fed2fac3fed4,
			.probe_addr = 0xfac5fed5fac7fed8
		} },
		.size = offsetofend(struct BPF_TASK_FD_QUERY_struct, probe_addr),
		.str = "task_fd_query={pid=1405705229"
		       ", fd=-559038737"
		       ", flags=4207869677"
		       ", buf_len=233811181"
		       ", buf=" BIG_ADDR("0xfffffffffffffffe", "0xfffffffe")
		       ", prog_id=3134983661"
		       ", fd_type=BPF_FD_TYPE_URETPROBE"
		       ", probe_offset=0xfac1fed2fac3fed4"
		       ", probe_addr=0xfac5fed5fac7fed8}"
	}
};

static const struct bpf_attr_check BPF_MAP_LOOKUP_BATCH_checks[] = {
	{
		.data = { .BPF_MAP_LOOKUP_BATCH_data = { .map_fd = -1 } },
		.size = offsetofend(struct BPF_MAP_LOOKUP_BATCH_struct, map_fd ),
		.str = "batch={in_batch=NULL, out_batch=NULL, keys=NULL"
		       ", values=NULL, count=0, map_fd=-1, elem_flags=BPF_ANY"
		       ", flags=0}"
	},
	{
		.data = { .BPF_MAP_LOOKUP_BATCH_data = {
			.in_batch = 0xfacefeed,
			.out_batch = 0xbadc0ded,
			.keys = 0xdeadf00d,
			.values = 0xfffffffe,
			.count = 3,
			.map_fd = -1,
			.elem_flags = 4,
			.flags = 4
		} },
		.size = offsetofend(struct BPF_MAP_LOOKUP_BATCH_struct, flags),
		.str = "batch={in_batch=0xfacefeed, out_batch=0xbadc0ded"
		       ", keys=0xdeadf00d, values=0xfffffffe, count=3"
		       ", map_fd=-1, elem_flags=BPF_F_LOCK, flags=0x4}"
	}
};

static const struct bpf_attr_check BPF_MAP_UPDATE_BATCH_checks[] = {
	{
		.data = { .BPF_MAP_UPDATE_BATCH_data = { .map_fd = -1 } },
		.size = offsetofend(struct BPF_MAP_UPDATE_BATCH_struct, map_fd ),
		.str = "batch={keys=NULL, values=NULL, count=0, map_fd=-1"
		       ", elem_flags=BPF_ANY, flags=0}"
	},
	{
		.data = { .BPF_MAP_UPDATE_BATCH_data = {
			.keys = 0xdeadf00d,
			.values = 0xfffffffe,
			.count = 3,
			.map_fd = -1,
			.elem_flags = 4,
			.flags = 4
		} },
		.size = offsetofend(struct BPF_MAP_UPDATE_BATCH_struct, flags),
		.str = "batch={keys=0xdeadf00d, values=0xfffffffe, count=3"
		       ", map_fd=-1, elem_flags=BPF_F_LOCK, flags=0x4}"
	}
};

static const struct bpf_attr_check BPF_MAP_DELETE_BATCH_checks[] = {
	{
		.data = { .BPF_MAP_DELETE_BATCH_data = { .map_fd = -1 } },
		.size = offsetofend(struct BPF_MAP_DELETE_BATCH_struct, map_fd ),
		.str = "batch={keys=NULL, count=0, map_fd=-1"
		       ", elem_flags=BPF_ANY, flags=0}"
	},
	{
		.data = { .BPF_MAP_DELETE_BATCH_data = {
			.keys = 0xdeadf00d,
			.count = 3,
			.map_fd = -1,
			.elem_flags = 4,
			.flags = 4
		} },
		.size = offsetofend(struct BPF_MAP_DELETE_BATCH_struct, flags),
		.str = "batch={keys=0xdeadf00d, count=3, map_fd=-1"
		       ", elem_flags=BPF_F_LOCK, flags=0x4}"
	}
};

static void
init_BPF_LINK_CREATE_attr1(struct bpf_attr_check *check, size_t idx)
{
	struct BPF_LINK_CREATE_struct *attr = &check->data.BPF_LINK_CREATE_data;

	attr->attach_type = idx;
}

static void
print_BPF_LINK_CREATE_attr1(const struct bpf_attr_check *check,
			    unsigned long addr, size_t idx)
{
	printf("link_create={prog_fd=-1, target_fd=-559038737"
	       ", attach_type=%s, flags=0x4}",
	       sprintxval(bpf_attach_type, idx, "BPF_???"));
}

/* Keep sorted */
static const uint8_t special_attach_types[] =
	{ 0, BPF_TRACE_ITER, BPF_PERF_EVENT, BPF_TRACE_KPROBE_MULTI };

static void
init_BPF_LINK_CREATE_attr2(struct bpf_attr_check *check, size_t idx)
{
	struct BPF_LINK_CREATE_struct *attr = &check->data.BPF_LINK_CREATE_data;

	/* skip special_attach_types */
	for (size_t i = 0; i < ARRAY_SIZE(special_attach_types)
			   && idx >= special_attach_types[i]; i++, idx++);

	attr->attach_type = idx;

	check->data.char_data[19] = ' ';
	check->data.char_data[23] = 'O';
	check->data.char_data[27] = 'H';
	check->data.char_data[31] = ' ';
	check->data.char_data[35] = 'H';
	check->data.char_data[39] = 'A';
	check->data.char_data[43] = 'I';
	check->data.char_data[47] = '!';
}

static void
print_BPF_LINK_CREATE_attr2(const struct bpf_attr_check *check,
			    unsigned long addr, size_t idx)
{
	/* skip special_attach_types */
	for (size_t i = 0; i < ARRAY_SIZE(special_attach_types)
			   && idx >= special_attach_types[i]; i++, idx++);

	printf("link_create={prog_fd=-1, target_fd=-559038737"
	       ", attach_type=%s, flags=0xbadc0ded}, "
#if VERBOSE
	       "extra_data=\"\\x00\\x00\\x00\\x20\\x00\\x00\\x00\\x4f"
	       "\\x00\\x00\\x00\\x48\\x00\\x00\\x00\\x20\\x00\\x00\\x00\\x48"
	       "\\x00\\x00\\x00\\x41\\x00\\x00\\x00\\x49\\x00\\x00\\x00\\x21\""
	       " /* bytes 16..47 */"
#else
	       "..."
#endif
	       ,
	       sprintxval(bpf_attach_type, idx, "BPF_???"));
}

static const int iter_info_data[] = { 0, 42, 314159265, 0xbadc0ded, -1 };
static int *iter_info_data_p;

static void
init_BPF_LINK_CREATE_attr7(struct bpf_attr_check *check, size_t idx)
{
	struct BPF_LINK_CREATE_struct *attr = &check->data.BPF_LINK_CREATE_data;

	close(iter_info_data[1]);

	if (!iter_info_data_p) {
		iter_info_data_p = tail_memdup(iter_info_data,
					       sizeof(iter_info_data));
	}

	attr->iter_info = (uintptr_t) iter_info_data_p;
	attr->iter_info_len = ARRAY_SIZE(iter_info_data) + idx;
}

static void
print_BPF_LINK_CREATE_attr7(const struct bpf_attr_check *check,
			    unsigned long addr, size_t idx)
{
	printf("link_create={prog_fd=0" FD0_PATH ", target_fd=0" FD0_PATH
	       ", attach_type=BPF_TRACE_ITER, flags=0"
	       ", iter_info=[{map={map_fd=0" FD0_PATH "}}, {map={map_fd=42}}"
	       ", {map={map_fd=314159265}}, {map={map_fd=-1159983635}}"
	       ", {map={map_fd=-1}}");
	if (idx) {
		printf(", ... /* %p */",
		       iter_info_data_p + ARRAY_SIZE(iter_info_data));
	}
	printf("], iter_info_len=%zu}", ARRAY_SIZE(iter_info_data) + idx);

}

static const char *syms_data[] = { "foo", NULL, "OH\0HAI",
				   "abcdefghijklmnopqrstuvwxyz0123456789" };
static char **syms_data_p;
static const uint64_t addrs_data[] = { 0, 1, 0xbadc0ded,
				       0xfacefeeddeadc0deULL };
static uint64_t *addrs_data_p;

static_assert(ARRAY_SIZE(syms_data) == ARRAY_SIZE(addrs_data),
	      "syms_data and addrs_data have to have the same element count");

static void
init_BPF_LINK_CREATE_attr12(struct bpf_attr_check *check, size_t idx)
{
	struct BPF_LINK_CREATE_struct *attr = &check->data.BPF_LINK_CREATE_data;

	if (!syms_data_p)
		syms_data_p = tail_memdup(syms_data, sizeof(syms_data));
	if (!addrs_data_p)
		addrs_data_p = tail_memdup(addrs_data, sizeof(addrs_data));

	attr->kprobe_multi.cnt = ARRAY_SIZE(syms_data) + idx;
	attr->kprobe_multi.syms = (uintptr_t) syms_data_p;
	attr->kprobe_multi.addrs = (uintptr_t) addrs_data_p;
	attr->kprobe_multi.cookies = (uintptr_t) addrs_data_p;
}

static void
print_BPF_LINK_CREATE_attr12(const struct bpf_attr_check *check,
			     unsigned long addr, size_t idx)
{
	printf("link_create={prog_fd=0" FD0_PATH ", target_fd=0" FD0_PATH
	       ", attach_type=BPF_TRACE_KPROBE_MULTI, flags=0"
	       ", kprobe_multi={flags=BPF_F_KPROBE_MULTI_RETURN|0xfacebeee"
	       ", cnt=%zu", ARRAY_SIZE(syms_data) + idx);
	printf(", syms=[\"foo\", NULL, \"OH\""
	       ", \"abcdefghijklmnopqrstuvwxyz012345\"...");
	if (idx)
		printf(", ... /* %p */", syms_data_p + ARRAY_SIZE(syms_data));
	for (size_t i = 0; i < 2; i++) {
		printf("], %s=[0, 0x1, 0xbadc0ded, 0xfacefeeddeadc0de",
		       i ? "cookies" : "addrs");
		if (idx) {
			printf(", ... /* %p */",
			       addrs_data_p + ARRAY_SIZE(addrs_data));
		}
	}
	printf("]}}");
}

static struct bpf_attr_check BPF_LINK_CREATE_checks[] = {
	{ /* 0 */
		.data = { .BPF_LINK_CREATE_data = { .prog_fd = 0, .target_fd = 0 } },
		.size = offsetofend(struct BPF_LINK_CREATE_struct, target_fd),
		.str = "link_create={prog_fd=0" FD0_PATH ", target_fd=0" FD0_PATH
		       ", attach_type=BPF_CGROUP_INET_INGRESS, flags=0}"
	},
	{ /* 1 */
		.data = { .BPF_LINK_CREATE_data = {
			.prog_fd = -1,
			.target_fd = 0xdeadbeef,
			.flags = 4
		} },
		.size = offsetofend(struct BPF_LINK_CREATE_struct, flags),
		.iters = ARRAY_SIZE(bpf_attach_type_xdata),
		.init_fn = init_BPF_LINK_CREATE_attr1,
		.print_fn = print_BPF_LINK_CREATE_attr1,
	},
	{ /* 2 - all non-special attach_types */
		.data = { .BPF_LINK_CREATE_data = {
			.prog_fd = -1,
			.target_fd = 0xdeadbeef,
			.attach_type = 5,
			.flags = 0xbadc0ded
		} },
		.size = 48,
		.iters = ARRAY_SIZE(bpf_attach_type_xdata)
			 - ARRAY_SIZE(special_attach_types),
		.init_fn = init_BPF_LINK_CREATE_attr2,
		.print_fn = print_BPF_LINK_CREATE_attr2,
	},

	{ /* 3 */
		.data = { .BPF_LINK_CREATE_data = {
			.attach_type = 0,
		} },
		.size = offsetofend(struct BPF_LINK_CREATE_struct,
				    target_btf_id),
		.str = "link_create={prog_fd=0" FD0_PATH", target_fd=0" FD0_PATH
		       ", attach_type=BPF_CGROUP_INET_INGRESS, flags=0}"
	},
	{ /* 4 */
		.data = { .BPF_LINK_CREATE_data = {
			.attach_type = 0,
			.target_btf_id = 0xfacefeed,
		} },
		.size = offsetofend(struct BPF_LINK_CREATE_struct,
				    target_btf_id),
		.str = "link_create={prog_fd=0" FD0_PATH", target_fd=0" FD0_PATH
		       ", attach_type=BPF_CGROUP_INET_INGRESS, flags=0"
		       ", target_btf_id=4207869677}"
	},

	{ /* 5 */
		.data = { .BPF_LINK_CREATE_data = {
			.attach_type = 28,
		} },
		.size = offsetofend(struct BPF_LINK_CREATE_struct,
				    iter_info_len),
		.str = "link_create={prog_fd=0" FD0_PATH", target_fd=0" FD0_PATH
		       ", attach_type=BPF_TRACE_ITER, flags=0"
		       ", iter_info=NULL, iter_info_len=0}"
	},
	{ /* 6 */
		.data = { .BPF_LINK_CREATE_data = {
			.attach_type = 28,
			.iter_info = 0xffffffff00000000,
			.iter_info_len = 0xdeadface,
		} },
		.size = offsetofend(struct BPF_LINK_CREATE_struct,
				    iter_info_len),
		.str = "link_create={prog_fd=0" FD0_PATH", target_fd=0" FD0_PATH
		       ", attach_type=BPF_TRACE_ITER, flags=0"
		       ", iter_info=" BIG_ADDR("0xffffffff00000000", "NULL")
		       ", iter_info_len=3735943886}"
	},
	{ /* 7 */
		.data = { .BPF_LINK_CREATE_data = {
			.attach_type = 28,
		} },
		.size = offsetofend(struct BPF_LINK_CREATE_struct,
				    iter_info_len),
		.iters = 2,
		.init_fn = init_BPF_LINK_CREATE_attr7,
		.print_fn = print_BPF_LINK_CREATE_attr7,
	},

	{ /* 8 */
		.data = { .BPF_LINK_CREATE_data = {
			.attach_type = 41,
		} },
		.size = offsetofend(struct BPF_LINK_CREATE_struct,
				    perf_event.bpf_cookie),
		.str = "link_create={prog_fd=0" FD0_PATH", target_fd=0" FD0_PATH
		       ", attach_type=BPF_PERF_EVENT, flags=0"
		       ", perf_event={bpf_cookie=0}}"
	},
	{ /* 9 */
		.data = { .BPF_LINK_CREATE_data = {
			.attach_type = 41,
			.perf_event = { .bpf_cookie = 0xdeadc0defacecafeULL },
		} },
		.size = offsetofend(struct BPF_LINK_CREATE_struct,
				    perf_event.bpf_cookie),
		.str = "link_create={prog_fd=0" FD0_PATH", target_fd=0" FD0_PATH
		       ", attach_type=BPF_PERF_EVENT, flags=0"
		       ", perf_event={bpf_cookie=0xdeadc0defacecafe}}"
	},

	{ /* 10 */
		.data = { .BPF_LINK_CREATE_data = {
			.attach_type = 42,
		} },
		.size = offsetofend(struct BPF_LINK_CREATE_struct,
				    kprobe_multi.cookies),
		.str = "link_create={prog_fd=0" FD0_PATH", target_fd=0" FD0_PATH
		       ", attach_type=BPF_TRACE_KPROBE_MULTI, flags=0"
		       ", kprobe_multi={flags=0, cnt=0, syms=NULL, addrs=NULL"
		       ", cookies=NULL}}"
	},
	{ /* 11 */
		.data = { .BPF_LINK_CREATE_data = {
			.attach_type = 42,
			.kprobe_multi = {
				.flags = 0xdeadc0de,
				.cnt = 0xbadfaced,
				.syms = 0xffffffff00000000,
				.addrs = 0xffffffff00000000,
				.cookies = 0xffffffff00000000,
			},
		} },
		.size = offsetofend(struct BPF_LINK_CREATE_struct,
				    kprobe_multi.cookies),
		.str = "link_create={prog_fd=0" FD0_PATH", target_fd=0" FD0_PATH
		       ", attach_type=BPF_TRACE_KPROBE_MULTI, flags=0"
		       ", kprobe_multi={flags=0xdeadc0de /* BPF_F_??? */"
		       ", cnt=3135220973"
		       ", syms=" BIG_ADDR("0xffffffff00000000", "NULL")
		       ", addrs=" BIG_ADDR("0xffffffff00000000", "NULL")
		       ", cookies=" BIG_ADDR("0xffffffff00000000", "NULL") "}}"
	},
	/*
	 * Note that here we rely on the fact that this attach_type has the
	 * largest de-facto attr_size to get the additional checks performed
	 * with the last check passed.
	 */
	{ /* 12 */
		.data = { .BPF_LINK_CREATE_data = {
			.attach_type = 42,
			.kprobe_multi = {
				.flags = 0xfacebeef,
			}
		} },
		.size = offsetofend(struct BPF_LINK_CREATE_struct,
				    kprobe_multi.cookies),
		.iters = 2,
		.init_fn = init_BPF_LINK_CREATE_attr12,
		.print_fn = print_BPF_LINK_CREATE_attr12,
	},
};

static const struct bpf_attr_check BPF_LINK_UPDATE_checks[] = {
	{
		.data = { .BPF_LINK_UPDATE_data = {
			.link_fd = -1,
			.new_prog_fd = -2
		} },
		.size = offsetofend(struct BPF_LINK_UPDATE_struct, old_prog_fd),
		.str = "link_update={link_fd=-1, new_prog_fd=-2, flags=0}"
	},
	{
		.data = { .BPF_LINK_UPDATE_data = {
			.link_fd = -1,
			.new_prog_fd = 0xdeadbeef,
			.flags = 4,
			.old_prog_fd = 0xdeadf00d
		} },
		.size = offsetofend(struct BPF_LINK_UPDATE_struct, old_prog_fd),
		.str = "link_update={link_fd=-1, new_prog_fd=-559038737"
		       ", flags=BPF_F_REPLACE, old_prog_fd=-559026163}"
	}
};

static const struct bpf_attr_check BPF_LINK_GET_FD_BY_ID_checks[] = {
	{
		.data = { .BPF_LINK_GET_FD_BY_ID_data = { .link_id = 0xdeadbeef } },
		.size = offsetofend(struct BPF_LINK_GET_FD_BY_ID_struct, link_id),
		.str = "link_id=3735928559"
	}
};

static const struct bpf_attr_check BPF_ENABLE_STATS_checks[] = {
	{
		.data = { .BPF_ENABLE_STATS_data = { .type = 0 } },
		.size = offsetofend(struct BPF_ENABLE_STATS_struct, type),
		.str = "enable_stats={type=BPF_STATS_RUN_TIME}"
	},
	{
		.data = { .BPF_ENABLE_STATS_data = { .type = 1 } },
		.size = offsetofend(struct BPF_ENABLE_STATS_struct, type),
		.str = "enable_stats={type=0x1 /* BPF_STATS_??? */}"
	}
};

static const struct bpf_attr_check BPF_ITER_CREATE_checks[] = {
	{
		.data = { .BPF_ITER_CREATE_data = {
			.link_fd = -1,
			.flags = 0
		} },
		.size = offsetofend(struct BPF_ITER_CREATE_struct, flags),
		.str = "iter_create={link_fd=-1, flags=0}"
	},
	{
		.data = { .BPF_ITER_CREATE_data = {
			.link_fd = -1,
			.flags = -1U,
		} },
		.size = offsetofend(struct BPF_ITER_CREATE_struct, flags),
		.str = "iter_create={link_fd=-1, flags=0xffffffff}"
	}
};

static const struct bpf_attr_check BPF_LINK_DETACH_checks[] = {
	{
		.data = { .BPF_LINK_DETACH_data = { .link_fd = -1 } },
		.size = offsetofend(struct BPF_LINK_DETACH_struct, link_fd),
		.str = "link_detach={link_fd=-1}"
	}
};

static const struct bpf_attr_check BPF_PROG_BIND_MAP_checks[] = {
	{
		.data = { .BPF_PROG_BIND_MAP_data = {
			.prog_fd = -1,
			.map_fd = -2,
			.flags = 0
		} },
		.size = offsetofend(struct BPF_PROG_BIND_MAP_struct, flags),
		.str = "prog_bind_map={prog_fd=-1, map_fd=-2, flags=0}"
	},
	{
		.data = { .BPF_PROG_BIND_MAP_data = {
			.prog_fd = -1,
			.map_fd = -2,
			.flags = -1U,
		} },
		.size = offsetofend(struct BPF_PROG_BIND_MAP_struct, flags),
		.str = "prog_bind_map={prog_fd=-1, map_fd=-2, flags=0xffffffff}"
	}
};


#define CHK(cmd_) \
	{ \
		cmd_, #cmd_, \
		cmd_##_checks, ARRAY_SIZE(cmd_##_checks), \
	} \
	/* End of CHK definition */

int
main(void)
{
	static const struct bpf_check checks[] = {
		CHK(BPF_MAP_CREATE),
		CHK(BPF_MAP_LOOKUP_ELEM),
		CHK(BPF_MAP_UPDATE_ELEM),
		CHK(BPF_MAP_DELETE_ELEM),
		CHK(BPF_MAP_GET_NEXT_KEY),
		CHK(BPF_PROG_LOAD),
		CHK(BPF_OBJ_PIN),
		CHK(BPF_OBJ_GET),
		CHK(BPF_PROG_ATTACH),
		CHK(BPF_PROG_DETACH),
		CHK(BPF_PROG_TEST_RUN),
		CHK(BPF_PROG_GET_NEXT_ID),
		CHK(BPF_MAP_GET_NEXT_ID),
		CHK(BPF_PROG_GET_FD_BY_ID),
		CHK(BPF_MAP_GET_FD_BY_ID),
		CHK(BPF_OBJ_GET_INFO_BY_FD),
		CHK(BPF_PROG_QUERY),
		CHK(BPF_RAW_TRACEPOINT_OPEN),
		CHK(BPF_BTF_LOAD),
		CHK(BPF_BTF_GET_FD_BY_ID),
		CHK(BPF_TASK_FD_QUERY),
		CHK(BPF_MAP_LOOKUP_AND_DELETE_ELEM),
		CHK(BPF_MAP_FREEZE),
		CHK(BPF_BTF_GET_NEXT_ID),
		CHK(BPF_MAP_LOOKUP_BATCH),
		CHK(BPF_MAP_UPDATE_BATCH),
		CHK(BPF_MAP_DELETE_BATCH),
		CHK(BPF_LINK_CREATE),
		CHK(BPF_LINK_UPDATE),
		CHK(BPF_LINK_GET_NEXT_ID),
		CHK(BPF_LINK_GET_FD_BY_ID),
		CHK(BPF_ENABLE_STATS),
		CHK(BPF_ITER_CREATE),
		CHK(BPF_LINK_DETACH),
		CHK(BPF_PROG_BIND_MAP),
	};

	page_size = get_page_size();
	end_of_page = (unsigned long) tail_alloc(1) + 1;

	at_fdcwd_str =
#ifdef YFLAG
		xasprintf("AT_FDCWD<%s>", get_fd_path(get_dir_fd(".")));
#else
		"AT_FDCWD";
#endif

	for (size_t i = 0; i < ARRAY_SIZE(checks); i++)
		test_bpf(checks + i);

	sys_bpf(0xfacefeed, 0, (kernel_ulong_t) 0xfacefeedbadc0dedULL);
	printf("bpf(0xfacefeed /* BPF_??? */, NULL, %u) = %s\n",
	       0xbadc0dedu, errstr);

	sys_bpf(0xfacefeed, end_of_page, 40);
	printf("bpf(0xfacefeed /* BPF_??? */, %#lx, 40) = %s\n",
	       end_of_page, errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
