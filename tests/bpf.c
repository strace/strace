/*
 * Check bpf syscall decoding.
 *
 * Copyright (c) 2015-2017 Dmitry V. Levin <ldv@altlinux.org>
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
test_bpf_(kernel_ulong_t cmd, const char *cmd_str,
	  unsigned int (*init_first)(const unsigned long eop),
	  void (*print_first)(const unsigned long eop),
	  unsigned int (*init_attr)(const unsigned long eop),
	  void (*print_attr)(const unsigned long eop))
{
	/* zero addr */
	sys_bpf(cmd, 0, long_bits | sizeof(union bpf_attr_data));
	printf("bpf(%s, NULL, %u) = %s\n",
	       cmd_str, sizeof_attr, errstr);

	/* zero size */
	unsigned long addr = end_of_page - sizeof_attr;
	sys_bpf(cmd, addr, long_bits);
	printf("bpf(%s, %#lx, 0) = %s\n",
	       cmd_str, addr, errstr);

	/* the first field only */
	unsigned int offset = init_first(end_of_page);
	addr = end_of_page - offset;
	sys_bpf(cmd, addr, offset);
	printf("bpf(%s, {", cmd_str);
	print_first(addr);
	printf("}, %u) = %s\n", offset, errstr);

	/* efault after the first field */
	sys_bpf(cmd, addr, offset + 1);
	printf("bpf(%s, %#lx, %u) = %s\n",
	       cmd_str, addr, offset + 1, errstr);

	/* the relevant part of bpf_attr_data */
	offset = init_attr(end_of_page);
	addr = end_of_page - offset;
	sys_bpf(cmd, addr, offset);
	printf("bpf(%s, {", cmd_str);
	print_attr(addr);
	printf("}, %u) = %s\n", offset, errstr);

	/* short read of the relevant part of bpf_attr_data */
	sys_bpf(cmd, addr + 1, offset);
	printf("bpf(%s, %#lx, %u) = %s\n",
	       cmd_str, addr + 1, offset, errstr);

	if (offset < sizeof_attr) {
		/* short read of the whole bpf_attr_data */
		memmove((void *) end_of_page - sizeof_attr + 1,
			(void *) addr, offset);
		addr = end_of_page - sizeof_attr + 1;
		memset((void *) addr + offset, 0,
		       sizeof_attr - offset - 1);
		sys_bpf(cmd, addr, sizeof_attr);
		printf("bpf(%s, %#lx, %u) = %s\n",
		       cmd_str, addr, sizeof_attr, errstr);

		/* the whole bpf_attr_data */
		memmove((void *) end_of_page - sizeof_attr,
			(void *) addr, offset);
		addr = end_of_page - sizeof_attr;
		memset((void *) addr + offset, 0,
		       sizeof_attr - offset);
		sys_bpf(cmd, addr, sizeof_attr);
		printf("bpf(%s, {", cmd_str);
		print_attr(addr);
		printf("}, %u) = %s\n", sizeof_attr, errstr);

		/* non-zero bytes after the relevant part */
		fill_memory_ex((void *) addr + offset,
			       sizeof_attr - offset, '0', 10);
		sys_bpf(cmd, addr, sizeof_attr);
		printf("bpf(%s, {", cmd_str);
		print_attr(addr);
		printf(", ");
		print_extra_data((char *) addr, offset,
				 sizeof_attr - offset);
		printf("}, %u) = %s\n", sizeof_attr, errstr);
	}

	/* short read of the whole page */
	memmove((void *) end_of_page - page_size + 1,
		(void *) addr, offset);
	addr = end_of_page - page_size + 1;
	memset((void *) addr + offset, 0,
	       page_size - offset - 1);
	sys_bpf(cmd, addr, page_size);
	printf("bpf(%s, %#lx, %u) = %s\n",
	       cmd_str, addr, page_size, errstr);

	/* the whole page */
	memmove((void *) end_of_page - page_size,
		(void *) addr, offset);
	addr = end_of_page - page_size;
	memset((void *) addr + offset, 0, page_size - offset);
	sys_bpf(cmd, addr, page_size);
	printf("bpf(%s, {", cmd_str);
	print_attr(addr);
	printf("}, %u) = %s\n", page_size, errstr);

	/* non-zero bytes after the whole bpf_attr_data */
	fill_memory_ex((void *) addr + offset,
		       page_size - offset, '0', 10);
	sys_bpf(cmd, addr, page_size);
	printf("bpf(%s, {", cmd_str);
	print_attr(addr);
	printf(", ");
	print_extra_data((char *) addr, offset,
			 page_size - offset);
	printf("}, %u) = %s\n", page_size, errstr);

	/* more than a page */
	sys_bpf(cmd, addr, page_size + 1);
	printf("bpf(%s, %#lx, %u) = %s\n",
	       cmd_str, addr, page_size + 1, errstr);
}

#define TEST_BPF(cmd_)							\
	test_bpf_((cmd_), #cmd_,					\
		  init_ ## cmd_ ## _first, print_ ## cmd_ ## _first,	\
		  init_ ## cmd_ ## _attr, print_ ## cmd_ ## _attr)	\
	/* End of TEST_BPF definition. */

#define DEF_BPF_INIT_FIRST(cmd_, field_, value_)			\
	static unsigned int						\
	init_ ## cmd_ ## _first(const unsigned long eop)		\
	{								\
		static const struct cmd_ ## _struct attr = {		\
			.field_ = value_				\
		};							\
		static const unsigned int offset = sizeof(attr.field_);	\
		const unsigned long addr = eop - offset;		\
									\
		memcpy((void *) addr, &attr.field_, offset);		\
		return offset;						\
	}								\
	/* End of DEF_INIT_FIRST definition. */

DEF_BPF_INIT_FIRST(BPF_MAP_CREATE, map_type, 2)

static void
print_BPF_MAP_CREATE_first(const unsigned long addr)
{
	printf("map_type=BPF_MAP_TYPE_ARRAY, key_size=0, value_size=0"
	       ", max_entries=0, map_flags=0, inner_map_fd=0");
}

static unsigned int
init_BPF_MAP_CREATE_attr(const unsigned long eop)
{
	static const struct BPF_MAP_CREATE_struct attr = {
		.map_type = 1,
		.key_size = 4,
		.value_size = 8,
		.max_entries = 256,
		.map_flags = 7,
		.inner_map_fd = -1,
		.numa_node = 42
	};
	static const unsigned int offset =
		offsetofend(struct BPF_MAP_CREATE_struct, numa_node);
	const unsigned long addr = eop - offset;

	memcpy((void *) addr, &attr, offset);
	return offset;
}

static void
print_BPF_MAP_CREATE_attr(const unsigned long addr)
{
	printf("map_type=BPF_MAP_TYPE_HASH, key_size=4"
	       ", value_size=8, max_entries=256"
	       ", map_flags=BPF_F_NO_PREALLOC|BPF_F_NO_COMMON_LRU"
	       "|BPF_F_NUMA_NODE, inner_map_fd=-1, numa_node=42");
}

DEF_BPF_INIT_FIRST(BPF_MAP_LOOKUP_ELEM, map_fd, -1)

static void
print_BPF_MAP_LOOKUP_ELEM_first(const unsigned long addr)
{
	printf("map_fd=-1, key=0, value=0");
}

static unsigned int
init_BPF_MAP_LOOKUP_ELEM_attr(const unsigned long eop)
{
	static const struct BPF_MAP_LOOKUP_ELEM_struct attr = {
		.map_fd = -1,
		.key = 0xdeadbeef,
		.value = 0xbadc0ded
	};
	static const unsigned int offset =
		offsetofend(struct BPF_MAP_LOOKUP_ELEM_struct, value);
	const unsigned long addr = eop - offset;

	memcpy((void *) addr, &attr, offset);
	return offset;
}

static void
print_BPF_MAP_LOOKUP_ELEM_attr(const unsigned long addr)
{
	printf("map_fd=-1, key=0xdeadbeef, value=0xbadc0ded");
}

#define init_BPF_MAP_UPDATE_ELEM_first init_BPF_MAP_LOOKUP_ELEM_first

static void
print_BPF_MAP_UPDATE_ELEM_first(const unsigned long addr)
{
	printf("map_fd=-1, key=0, value=0, flags=BPF_ANY");
}

static unsigned int
init_BPF_MAP_UPDATE_ELEM_attr(const unsigned long eop)
{
	static const struct BPF_MAP_UPDATE_ELEM_struct attr = {
		.map_fd = -1,
		.key = 0xdeadbeef,
		.value = 0xbadc0ded,
		.flags = 2
	};
	static const unsigned int offset =
		offsetofend(struct BPF_MAP_UPDATE_ELEM_struct, flags);
	const unsigned long addr = eop - offset;

	memcpy((void *) addr, &attr, offset);
	return offset;
}

static void
print_BPF_MAP_UPDATE_ELEM_attr(const unsigned long addr)
{
	printf("map_fd=-1, key=0xdeadbeef, value=0xbadc0ded, flags=BPF_EXIST");
}

#define init_BPF_MAP_DELETE_ELEM_first init_BPF_MAP_LOOKUP_ELEM_first

static void
print_BPF_MAP_DELETE_ELEM_first(const unsigned long addr)
{
	printf("map_fd=-1, key=0");
}

static unsigned int
init_BPF_MAP_DELETE_ELEM_attr(const unsigned long eop)
{
	static const struct BPF_MAP_DELETE_ELEM_struct attr = {
		.map_fd = -1,
		.key = 0xdeadbeef
	};
	static const unsigned int offset =
		offsetofend(struct BPF_MAP_DELETE_ELEM_struct, key);
	const unsigned long addr = eop - offset;

	memcpy((void *) addr, &attr, offset);
	return offset;
}

static void
print_BPF_MAP_DELETE_ELEM_attr(const unsigned long addr)
{
	printf("map_fd=-1, key=0xdeadbeef");
}

#define init_BPF_MAP_GET_NEXT_KEY_first init_BPF_MAP_LOOKUP_ELEM_first

static void
print_BPF_MAP_GET_NEXT_KEY_first(const unsigned long addr)
{
	printf("map_fd=-1, key=0, next_key=0");
}

static unsigned int
init_BPF_MAP_GET_NEXT_KEY_attr(const unsigned long eop)
{
	static const struct BPF_MAP_GET_NEXT_KEY_struct attr = {
		.map_fd = -1,
		.key = 0xdeadbeef,
		.next_key = 0xbadc0ded
	};
	static const unsigned int offset =
		offsetofend(struct BPF_MAP_GET_NEXT_KEY_struct, next_key);
	const unsigned long addr = eop - offset;

	memcpy((void *) addr, &attr, offset);
	return offset;
}

static void
print_BPF_MAP_GET_NEXT_KEY_attr(const unsigned long addr)
{
	printf("map_fd=-1, key=0xdeadbeef, next_key=0xbadc0ded");
}

DEF_BPF_INIT_FIRST(BPF_PROG_LOAD, prog_type, 1)

static void
print_BPF_PROG_LOAD_first(const unsigned long addr)
{

	printf("prog_type=BPF_PROG_TYPE_SOCKET_FILTER, insn_cnt=0, insns=0"
	       ", license=NULL");
}

static const struct bpf_insn insns[] = {
	{ .code = 0x95 }
};
static char log_buf[4096];

static unsigned int
init_BPF_PROG_LOAD_attr(const unsigned long eop)
{
	const struct BPF_PROG_LOAD_struct attr = {
		.prog_type = 1,
		.insn_cnt = ARRAY_SIZE(insns),
		.insns = (uintptr_t) insns,
		.license = (uintptr_t) "GPL",
		.log_level = 42,
		.log_size = sizeof(log_buf),
		.log_buf = (uintptr_t) log_buf,
		.kern_version = 0xcafef00d,
		.prog_flags = 1
	};
	static const unsigned int offset =
		offsetofend(struct BPF_PROG_LOAD_struct, prog_flags);
	const unsigned long addr = eop - offset;

	memcpy((void *) addr, &attr, offset);
	return offset;
}

static void
print_BPF_PROG_LOAD_attr(const unsigned long addr)
{
	printf("prog_type=BPF_PROG_TYPE_SOCKET_FILTER, insn_cnt=%u, insns=%p"
	       ", license=\"GPL\", log_level=42, log_size=4096, log_buf=%p"
	       ", kern_version=KERNEL_VERSION(51966, 240, 13)"
	       ", prog_flags=BPF_F_STRICT_ALIGNMENT",
	       (unsigned int) ARRAY_SIZE(insns), insns,
	       log_buf);
}

DEF_BPF_INIT_FIRST(BPF_OBJ_PIN, pathname, 0)

static void
print_BPF_OBJ_PIN_first(const unsigned long addr)
{

	printf("pathname=NULL, bpf_fd=0");
}

static unsigned int
init_BPF_OBJ_PIN_attr(const unsigned long eop)
{
	const struct BPF_OBJ_PIN_struct attr = {
		.pathname = (uintptr_t) "/sys/fs/bpf/foo/bar",
		.bpf_fd = -1
	};
	static const unsigned int offset =
		offsetofend(struct BPF_OBJ_PIN_struct, bpf_fd);
	const unsigned long addr = eop - offset;

	memcpy((void *) addr, &attr, offset);
	return offset;
}

static void
print_BPF_OBJ_PIN_attr(const unsigned long addr)
{
	printf("pathname=\"/sys/fs/bpf/foo/bar\", bpf_fd=-1");
}

#define init_BPF_OBJ_GET_first init_BPF_OBJ_PIN_first
#define print_BPF_OBJ_GET_first print_BPF_OBJ_PIN_first
#define init_BPF_OBJ_GET_attr init_BPF_OBJ_PIN_attr
#define print_BPF_OBJ_GET_attr print_BPF_OBJ_PIN_attr

DEF_BPF_INIT_FIRST(BPF_PROG_ATTACH, target_fd, -1)

static void
print_BPF_PROG_ATTACH_first(const unsigned long addr)
{
	printf("target_fd=-1, attach_bpf_fd=0"
	       ", attach_type=BPF_CGROUP_INET_INGRESS, attach_flags=0");
}

static unsigned int
init_BPF_PROG_ATTACH_attr(const unsigned long eop)
{
	static const struct BPF_PROG_ATTACH_struct attr = {
		.target_fd = -1,
		.attach_bpf_fd = -2,
		.attach_type = 2,
		.attach_flags = 1
	};
	static const unsigned int offset =
		offsetofend(struct BPF_PROG_ATTACH_struct, attach_flags);
	const unsigned long addr = eop - offset;

	memcpy((void *) addr, &attr, offset);
	return offset;
}

static void
print_BPF_PROG_ATTACH_attr(const unsigned long addr)
{
	printf("target_fd=-1, attach_bpf_fd=-2"
	       ", attach_type=BPF_CGROUP_INET_SOCK_CREATE"
	       ", attach_flags=BPF_F_ALLOW_OVERRIDE");
}

#define init_BPF_PROG_DETACH_first init_BPF_PROG_ATTACH_first

static unsigned int
init_BPF_PROG_DETACH_attr(const unsigned long eop)
{
	static const struct BPF_PROG_DETACH_struct attr = {
		.target_fd = -1,
		.attach_type = 2
	};
	static const unsigned int offset =
		offsetofend(struct BPF_PROG_DETACH_struct, attach_type);
	const unsigned long addr = eop - offset;

	memcpy((void *) addr, &attr, offset);
	return offset;
}


static void
print_BPF_PROG_DETACH_first(const unsigned long addr)
{
	printf("target_fd=-1, attach_type=BPF_CGROUP_INET_INGRESS");
}

static void
print_BPF_PROG_DETACH_attr(const unsigned long addr)
{
	printf("target_fd=-1, attach_type=BPF_CGROUP_INET_SOCK_CREATE");
}

DEF_BPF_INIT_FIRST(BPF_PROG_TEST_RUN, prog_fd, -1)

static void
print_BPF_PROG_TEST_RUN_first(const unsigned long addr)
{
	printf("test={prog_fd=-1, retval=0, data_size_in=0, data_size_out=0"
	       ", data_in=0, data_out=0, repeat=0, duration=0}");
}

static const struct BPF_PROG_TEST_RUN_struct sample_BPF_PROG_TEST_RUN_attr = {
	.prog_fd = -1,
	.retval = 0xfac1fed2,
	.data_size_in = 0xfac3fed4,
	.data_size_out = 0xfac5fed6,
	.data_in = (uint64_t) 0xfacef11dbadc2ded,
	.data_out = (uint64_t) 0xfacef33dbadc4ded,
	.repeat = 0xfac7fed8,
	.duration = 0xfac9feda
};
static unsigned int
init_BPF_PROG_TEST_RUN_attr(const unsigned long eop)
{
	static const unsigned int offset =
		offsetofend(struct BPF_PROG_TEST_RUN_struct, duration);
	const unsigned long addr = eop - offset;

	memcpy((void *) addr, &sample_BPF_PROG_TEST_RUN_attr, offset);
	return offset;
}

static void
print_BPF_PROG_TEST_RUN_attr(const unsigned long addr)
{
	PRINT_FIELD_D("test={", sample_BPF_PROG_TEST_RUN_attr, prog_fd);
	PRINT_FIELD_U(", ", sample_BPF_PROG_TEST_RUN_attr, retval);
	PRINT_FIELD_U(", ", sample_BPF_PROG_TEST_RUN_attr, data_size_in);
	PRINT_FIELD_U(", ", sample_BPF_PROG_TEST_RUN_attr, data_size_out);
	PRINT_FIELD_X(", ", sample_BPF_PROG_TEST_RUN_attr, data_in);
	PRINT_FIELD_X(", ", sample_BPF_PROG_TEST_RUN_attr, data_out);
	PRINT_FIELD_U(", ", sample_BPF_PROG_TEST_RUN_attr, repeat);
	PRINT_FIELD_U(", ", sample_BPF_PROG_TEST_RUN_attr, duration);
	printf("}");
}

DEF_BPF_INIT_FIRST(BPF_PROG_GET_NEXT_ID, start_id, 0xdeadbeef)

static void
print_BPF_PROG_GET_NEXT_ID_first(const unsigned long addr)
{
	printf("start_id=%u, next_id=0", 0xdeadbeef);
}

static unsigned int
init_BPF_PROG_GET_NEXT_ID_attr(const unsigned long eop)
{
	static const struct BPF_PROG_GET_NEXT_ID_struct attr = {
		.start_id = 0xbadc0ded,
		.next_id = 0xcafef00d
	};
	static const unsigned int offset =
		offsetofend(struct BPF_PROG_GET_NEXT_ID_struct, next_id);
	const unsigned long addr = eop - offset;

	memcpy((void *) addr, &attr, offset);
	return offset;
}

static void
print_BPF_PROG_GET_NEXT_ID_attr(const unsigned long addr)
{
	printf("start_id=%u, next_id=%u", 0xbadc0ded, 0xcafef00d);
}

#define init_BPF_MAP_GET_NEXT_ID_first init_BPF_PROG_GET_NEXT_ID_first
#define print_BPF_MAP_GET_NEXT_ID_first print_BPF_PROG_GET_NEXT_ID_first
#define init_BPF_MAP_GET_NEXT_ID_attr init_BPF_PROG_GET_NEXT_ID_attr
#define print_BPF_MAP_GET_NEXT_ID_attr print_BPF_PROG_GET_NEXT_ID_attr

#define init_BPF_PROG_GET_FD_BY_ID_first init_BPF_PROG_GET_NEXT_ID_first
#define init_BPF_PROG_GET_FD_BY_ID_attr init_BPF_PROG_GET_NEXT_ID_attr

static void
print_BPF_PROG_GET_FD_BY_ID_first(const unsigned long addr)
{
	printf("prog_id=%u, next_id=0", 0xdeadbeef);
}

static void
print_BPF_PROG_GET_FD_BY_ID_attr(const unsigned long addr)
{
	printf("prog_id=%u, next_id=%u", 0xbadc0ded, 0xcafef00d);
}

#define init_BPF_MAP_GET_FD_BY_ID_first init_BPF_PROG_GET_NEXT_ID_first
#define init_BPF_MAP_GET_FD_BY_ID_attr init_BPF_PROG_GET_NEXT_ID_attr

static void
print_BPF_MAP_GET_FD_BY_ID_first(const unsigned long addr)
{
	printf("map_id=%u, next_id=0", 0xdeadbeef);
}

static void
print_BPF_MAP_GET_FD_BY_ID_attr(const unsigned long addr)
{
	printf("map_id=%u, next_id=%u", 0xbadc0ded, 0xcafef00d);
}

DEF_BPF_INIT_FIRST(BPF_OBJ_GET_INFO_BY_FD, bpf_fd, -1)

static void
print_BPF_OBJ_GET_INFO_BY_FD_first(const unsigned long addr)
{
	printf("info={bpf_fd=-1, info_len=0, info=0}");
}

static const struct BPF_OBJ_GET_INFO_BY_FD_struct
	sample_BPF_OBJ_GET_INFO_BY_FD_attr = {
		.bpf_fd = -1,
		.info_len = 0xdeadbeef,
		.info = (uint64_t) 0xfacefeedbadc0ded
	};
static unsigned int
init_BPF_OBJ_GET_INFO_BY_FD_attr(const unsigned long eop)
{
	static const unsigned int offset =
		offsetofend(struct BPF_OBJ_GET_INFO_BY_FD_struct, info);
	const unsigned long addr = eop - offset;

	memcpy((void *) addr, &sample_BPF_OBJ_GET_INFO_BY_FD_attr, offset);
	return offset;
}

static void
print_BPF_OBJ_GET_INFO_BY_FD_attr(const unsigned long addr)
{
	PRINT_FIELD_D("info={", sample_BPF_OBJ_GET_INFO_BY_FD_attr, bpf_fd);
	PRINT_FIELD_U(", ", sample_BPF_OBJ_GET_INFO_BY_FD_attr, info_len);
	PRINT_FIELD_X(", ", sample_BPF_OBJ_GET_INFO_BY_FD_attr, info);
	printf("}");
}

int
main(void)
{
	page_size = get_page_size();
	end_of_page = (unsigned long) tail_alloc(1) + 1;

	TEST_BPF(BPF_MAP_CREATE);
	TEST_BPF(BPF_MAP_LOOKUP_ELEM);
	TEST_BPF(BPF_MAP_UPDATE_ELEM);
	TEST_BPF(BPF_MAP_DELETE_ELEM);
	TEST_BPF(BPF_MAP_GET_NEXT_KEY);
	TEST_BPF(BPF_PROG_LOAD);
	TEST_BPF(BPF_OBJ_PIN);
	TEST_BPF(BPF_OBJ_GET);
	TEST_BPF(BPF_PROG_ATTACH);
	TEST_BPF(BPF_PROG_DETACH);
	TEST_BPF(BPF_PROG_TEST_RUN);
	TEST_BPF(BPF_PROG_GET_NEXT_ID);
	TEST_BPF(BPF_MAP_GET_NEXT_ID);
	TEST_BPF(BPF_PROG_GET_FD_BY_ID);
	TEST_BPF(BPF_MAP_GET_FD_BY_ID);
	TEST_BPF(BPF_OBJ_GET_INFO_BY_FD);

	sys_bpf(0xfacefeed, 0, (kernel_ulong_t) 0xfacefeedbadc0dedULL);
	printf("bpf(0xfacefeed /* BPF_??? */, NULL, %u) = %s\n",
	       0xbadc0dedu, errstr);

	sys_bpf(0xfacefeed, end_of_page, 40);
	printf("bpf(0xfacefeed /* BPF_??? */, %#lx, 40) = %s\n",
	       end_of_page, errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
