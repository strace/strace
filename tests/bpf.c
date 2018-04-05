/*
 * Check bpf syscall decoding.
 *
 * Copyright (c) 2015-2017 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2018 The strace developers.
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

#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <asm/unistd.h>
#include "scno.h"

#ifdef HAVE_LINUX_BPF_H
# include <linux/bpf.h>
#endif

#include "bpf_attr.h"
#include "print_fields.h"

#include "xlat.h"
#include "xlat/bpf_commands.h"

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
	char char_data[256];
};

struct bpf_attr_check {
	union bpf_attr_data data;
	size_t size;
	const char *str;
	void (*init_fn)(struct bpf_attr_check *check);
	void (*print_fn)(const struct bpf_attr_check *check,
			 unsigned long addr);
};

struct bpf_check {
	kernel_ulong_t cmd;
	const char *cmd_str;
	const struct bpf_attr_check *checks;
	size_t count;
};

static const kernel_ulong_t long_bits = (kernel_ulong_t) 0xfacefeed00000000ULL;
static const char *errstr;
static unsigned int sizeof_attr = sizeof(union bpf_attr_data);
static unsigned int page_size;
static unsigned long end_of_page;

static long
sys_bpf(kernel_ulong_t cmd, kernel_ulong_t attr, kernel_ulong_t size)
{
	long rc = syscall(__NR_bpf, cmd, attr, size);
	errstr = sprintrc(rc);
	return rc;
}

#if VERBOSE
# define print_extra_data(addr_, offs_, size_) \
	do { \
		printf("/* bytes %u..%u */ ", (offs_), (size_) + (offs_) - 1); \
		print_quoted_hex((addr_) + (offs_), (size_)); \
	} while (0)
#else
# define print_extra_data(addr_, offs_, size_) printf("...")
#endif

static void
print_bpf_attr(const struct bpf_attr_check *check, unsigned long addr)
{
	if (check->print_fn)
		check->print_fn(check, addr);
	else
		printf("%s", check->str);
}

static void
test_bpf(const struct bpf_check *cmd_check)
{
	const struct bpf_attr_check *check = 0;
	const union bpf_attr_data *data = 0;
	unsigned int offset = 0;

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
		if (check->init_fn)
			check->init_fn((struct bpf_attr_check *) check);
		data = &check->data;
		offset = check->size;

		addr = end_of_page - offset;
		memcpy((void *) addr, data, offset);

		/* starting piece of bpf_attr_data */
		sys_bpf(cmd_check->cmd, addr, offset);
		printf("bpf(%s, {", cmd_check->cmd_str);
		print_bpf_attr(check, addr);
		printf("}, %u) = %s\n", offset, errstr);

		/* short read of the starting piece */
		sys_bpf(cmd_check->cmd, addr + 1, offset);
		printf("bpf(%s, %#lx, %u) = %s\n",
		       cmd_check->cmd_str, addr + 1, offset, errstr);
	}

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
		print_bpf_attr(check, addr);
		printf("}, %u) = %s\n", sizeof_attr, errstr);

		/* non-zero bytes after the relevant part */
		fill_memory_ex((void *) addr + offset,
			       sizeof_attr - offset, '0', 10);
		sys_bpf(cmd_check->cmd, addr, sizeof_attr);
		printf("bpf(%s, {", cmd_check->cmd_str);
		print_bpf_attr(check, addr);
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
	print_bpf_attr(check, addr);
	printf("}, %u) = %s\n", page_size, errstr);

	/* non-zero bytes after the whole bpf_attr_data */
	fill_memory_ex((void *) addr + offset,
		       page_size - offset, '0', 10);
	sys_bpf(cmd_check->cmd, addr, page_size);
	printf("bpf(%s, {", cmd_check->cmd_str);
	print_bpf_attr(check, addr);
	printf(", ");
	print_extra_data((char *) addr, offset,
			 page_size - offset);
	printf("}, %u) = %s\n", page_size, errstr);

	/* more than a page */
	sys_bpf(cmd_check->cmd, addr, page_size + 1);
	printf("bpf(%s, %#lx, %u) = %s\n",
	       cmd_check->cmd_str, addr, page_size + 1, errstr);
}

static const struct bpf_attr_check BPF_MAP_CREATE_checks[] = {
	{
		.data = { .BPF_MAP_CREATE_data = { .map_type = 2 } },
		.size = offsetofend(struct BPF_MAP_CREATE_struct, map_type),
		.str = "map_type=BPF_MAP_TYPE_ARRAY, key_size=0, value_size=0"
		       ", max_entries=0, map_flags=0, inner_map_fd=0"
	},
	{
		.data = { .BPF_MAP_CREATE_data = {
			.map_type = 1,
			.key_size = 4,
			.value_size = 8,
			.max_entries = 256,
			.map_flags = 7,
			.inner_map_fd = -1,
			.numa_node = 42
		} },
		.size = offsetofend(struct BPF_MAP_CREATE_struct, numa_node),
		.str = "map_type=BPF_MAP_TYPE_HASH, key_size=4"
		       ", value_size=8, max_entries=256"
		       ", map_flags=BPF_F_NO_PREALLOC|BPF_F_NO_COMMON_LRU"
		       "|BPF_F_NUMA_NODE, inner_map_fd=-1, numa_node=42"
	}
};

static const struct bpf_attr_check BPF_MAP_LOOKUP_ELEM_checks[] = {
	{
		.data = { .BPF_MAP_LOOKUP_ELEM_data = { .map_fd = -1 } },
		.size = offsetofend(struct BPF_MAP_LOOKUP_ELEM_struct, map_fd),
		.str = "map_fd=-1, key=0, value=0"
	},
	{
		.data = { .BPF_MAP_LOOKUP_ELEM_data = {
			.map_fd = -1,
			.key = 0xdeadbeef,
			.value = 0xbadc0ded
		} },
		.size = offsetofend(struct BPF_MAP_LOOKUP_ELEM_struct, value),
		.str = "map_fd=-1, key=0xdeadbeef, value=0xbadc0ded"
	}
};

static const struct bpf_attr_check BPF_MAP_UPDATE_ELEM_checks[] = {
	{
		.data = { .BPF_MAP_UPDATE_ELEM_data = { .map_fd = -1 } },
		.size = offsetofend(struct BPF_MAP_UPDATE_ELEM_struct, map_fd),
		.str = "map_fd=-1, key=0, value=0, flags=BPF_ANY"
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
		.str = "map_fd=-1, key=0"
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
		.str = "map_fd=-1, key=0, next_key=0"
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

static const struct bpf_insn insns[] = {
	{ .code = 0x95 }
};
static const char license[] = "GPL";
static char log_buf[4096];
static const char pathname[] = "/sys/fs/bpf/foo/bar";

static void
init_BPF_PROG_LOAD_attr(struct bpf_attr_check *check)
{
	struct BPF_PROG_LOAD_struct *attr = &check->data.BPF_PROG_LOAD_data;
	attr->insns = (uintptr_t) insns;
	attr->license = (uintptr_t) license;
	attr->log_buf = (uintptr_t) log_buf;
}

static void
print_BPF_PROG_LOAD_attr(const struct bpf_attr_check *check, unsigned long addr)
{
	printf("prog_type=BPF_PROG_TYPE_SOCKET_FILTER, insn_cnt=%u, insns=%p"
	       ", license=\"%s\", log_level=42, log_size=4096, log_buf=%p"
	       ", kern_version=KERNEL_VERSION(51966, 240, 13)"
	       ", prog_flags=BPF_F_STRICT_ALIGNMENT",
	       (unsigned int) ARRAY_SIZE(insns), insns,
	       license, log_buf);
}

static struct bpf_attr_check BPF_PROG_LOAD_checks[] = {
	{
		.data = { .BPF_PROG_LOAD_data = { .prog_type = 1 } },
		.size = offsetofend(struct BPF_PROG_LOAD_struct, prog_type),
		.str = "prog_type=BPF_PROG_TYPE_SOCKET_FILTER"
		       ", insn_cnt=0, insns=0, license=NULL"
	},
	{
		.data = { .BPF_PROG_LOAD_data = {
			.prog_type = 1,
			.insn_cnt = ARRAY_SIZE(insns),
			.log_level = 42,
			.log_size = sizeof(log_buf),
			.kern_version = 0xcafef00d,
			.prog_flags = 1
		} },
		.size = offsetofend(struct BPF_PROG_LOAD_struct, prog_flags),
		.init_fn = init_BPF_PROG_LOAD_attr,
		.print_fn = print_BPF_PROG_LOAD_attr
	}
};

static void
init_BPF_OBJ_PIN_attr(struct bpf_attr_check *check)
{
	struct BPF_OBJ_PIN_struct *attr = &check->data.BPF_OBJ_PIN_data;
	attr->pathname = (uintptr_t) pathname;
}

static struct bpf_attr_check BPF_OBJ_PIN_checks[] = {
	{
		.data = { .BPF_OBJ_PIN_data = { .pathname = 0 } },
		.size = offsetofend(struct BPF_OBJ_PIN_struct, pathname),
		.str = "pathname=NULL, bpf_fd=0"
	},
	{
		.data = { .BPF_OBJ_PIN_data = {
			.pathname = 0xFFFFFFFFFFFFFFFFULL
		} },
		.size = offsetofend(struct BPF_OBJ_PIN_struct, pathname),
		.str = "pathname="
#if defined MPERS_IS_m32 || SIZEOF_KERNEL_LONG_T > 4
		       "0xffffffffffffffff"
#elif defined __arm__ || defined __i386__ || defined __mips__ || \
      defined __powerpc__ || defined __riscv__ || defined __s390__ \
      || defined __sparc__ || defined __tile__
		       "0xffffffffffffffff or 0xffffffff"
#else
		       "0xffffffff"
#endif
		       ", bpf_fd=0",
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
	}
};

#define BPF_OBJ_GET_checks BPF_OBJ_PIN_checks

static const struct bpf_attr_check BPF_PROG_ATTACH_checks[] = {
	{
		.data = { .BPF_PROG_ATTACH_data = { .target_fd = -1 } },
		.size = offsetofend(struct BPF_PROG_ATTACH_struct, target_fd),
		.str = "target_fd=-1, attach_bpf_fd=0"
		       ", attach_type=BPF_CGROUP_INET_INGRESS, attach_flags=0"
	},
	{
		.data = { .BPF_PROG_ATTACH_data = {
			.target_fd = -1,
			.attach_bpf_fd = -2,
			.attach_type = 2,
			.attach_flags = 1
		} },
		.size = offsetofend(struct BPF_PROG_ATTACH_struct, attach_flags),
		.str = "target_fd=-1, attach_bpf_fd=-2"
		       ", attach_type=BPF_CGROUP_INET_SOCK_CREATE"
		       ", attach_flags=BPF_F_ALLOW_OVERRIDE"
	}
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
		       ", data_size_out=0, data_in=0, data_out=0"
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
	}
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
		.str = "start_id="
#if WORDS_BIGENDIAN
		       "3724541952"	/* 0xde000000 */
#else
		       "239"		/* 0x000000ef */
#endif
		       ", next_id=0"
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
		.str = "info={bpf_fd=-1, info_len=0, info=0}"
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
	};

	page_size = get_page_size();
	end_of_page = (unsigned long) tail_alloc(1) + 1;

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
