/*
 * Check bpf(BPF_OBJ_GET_INFO_BY_FD) decoding.
 *
 * Copyright (c) 2018-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#ifndef CHECK_OBJ_PROG
# define CHECK_OBJ_PROG 0
#endif

#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/sysmacros.h>

#include "print_fields.h"
#include "scno.h"

#ifdef HAVE_LINUX_BPF_H
# include <linux/bpf.h>
#endif

#include "bpf_attr.h"

#include "xlat.h"
#include "xlat/bpf_map_flags.h"
#include "xlat/bpf_map_types.h"
#include "xlat/bpf_prog_types.h"

#define XLAT_MACROS_ONLY
#include "xlat/bpf_commands.h"
#include "xlat/bpf_op_alu.h"
#include "xlat/bpf_op_jmp.h"
#include "xlat/bpf_size.h"
#include "xlat/bpf_src.h"
#include "xlat/ebpf_class.h"
#include "xlat/ebpf_mode.h"
#include "xlat/ebpf_op_alu.h"
#include "xlat/ebpf_regs.h"
#include "xlat/ebpf_size.h"

#ifndef HAVE_STRUCT_BPF_INSN
struct bpf_insn {
	uint8_t	code;
	uint8_t	dst_reg:4;
	uint8_t	src_reg:4;
	int16_t	off;
	int32_t	imm;
};
#endif

static const char *errstr;

static long
sys_bpf(kernel_ulong_t cmd, void *attr, kernel_ulong_t size)
{
	long rc = syscall(__NR_bpf, cmd, attr, size);
	errstr = sprintrc(rc);
	return rc;
}

static void
print_map_create(void *attr_void, size_t size, long rc)
{
	struct BPF_MAP_CREATE_struct *attr = attr_void;

	printf("bpf(BPF_MAP_CREATE, {map_type=BPF_MAP_TYPE_ARRAY, key_size=4"
	       ", value_size=%u, max_entries=%u",
	       attr->value_size, attr->max_entries);
	if (size > offsetof(struct BPF_MAP_CREATE_struct, map_flags))
		printf(", map_flags=0");
	if (size > offsetof(struct BPF_MAP_CREATE_struct, inner_map_fd))
		printf(", inner_map_fd=0</dev/null>");
	if (size > offsetof(struct BPF_MAP_CREATE_struct, map_name))
		printf(", map_name=\"%s\"", attr->map_name);
	if (size > offsetof(struct BPF_MAP_CREATE_struct, map_ifindex))
		printf(", map_ifindex=0");
	if (size > offsetof(struct BPF_MAP_CREATE_struct, btf_fd)) {
		printf(", btf_fd=0</dev/null>"
		       ", btf_key_type_id=0, btf_value_type_id=0");
	}
	if (size > offsetof(struct BPF_MAP_CREATE_struct,
			    btf_vmlinux_value_type_id)) {
		printf(", btf_vmlinux_value_type_id=0");
	}
	printf("}, %zu) = ", size);
	if (rc >= 0)
		printf("%ld<anon_inode:bpf-map>\n", rc);
	else
		puts(errstr);
}

#if CHECK_OBJ_PROG
static struct bpf_insn socket_prog[] = {
	{ /* 0 */
		.code    = BPF_ALU64 | BPF_K | BPF_MOV,
		.dst_reg = BPF_REG_1,
		.imm     = 0,
	},
	{ /* 1 */
		.code    = BPF_STX | BPF_W | BPF_MEM,
		.dst_reg = BPF_REG_10,
		.src_reg = BPF_REG_1,
		.off     = -4,
	},
	{ /* 2 */
		.code = BPF_ALU64 | BPF_X | BPF_MOV,
		.dst_reg = BPF_REG_2,
		.src_reg = BPF_REG_10,
	},
	{ /* 3 */
		.code    = BPF_ALU64 | BPF_K | BPF_ADD,
		.dst_reg = BPF_REG_2,
		.imm     = -4,
	},
	{ /* 4 */
		.code    = BPF_LD | BPF_DW | BPF_IMM,
		.dst_reg = BPF_REG_1,
		.src_reg = 1 /* BPF_PSEUDO_MAP_FD */,
		.imm     = 0, /* to be set to map fd2 */
	},
	{ /* 5 */
		.imm     = 0,
	},
	{ /* 6 */
		.code    = BPF_LD | BPF_DW | BPF_IMM,
		.dst_reg = BPF_REG_1,
		.src_reg = 1 /* BPF_PSEUDO_MAP_FD */,
		.imm     = 0, /* to be set to map fd */
	},
	{ /* 7 */
		.imm     = 0,
	},
	{ /* 8 */
		.code    = BPF_JMP | BPF_K | BPF_CALL,
		.imm     = 0x1, /* BPF_FUNC_map_lookup_elem */
	},
	{ /* 9 */
		.code    = BPF_ALU64 | BPF_K | BPF_MOV,
		.dst_reg = BPF_REG_0,
		.imm     = 0,
	},
	{ /* 10 */
		.code    = BPF_JMP | BPF_K | BPF_EXIT,
	},
};

# if VERBOSE
static const char *socket_prog_fmt =
	"[{code=BPF_ALU64|BPF_K|BPF_MOV"
		", dst_reg=BPF_REG_1, src_reg=BPF_REG_0, off=0, imm=0}"
	", {code=BPF_STX|BPF_W|BPF_MEM"
		", dst_reg=BPF_REG_10, src_reg=BPF_REG_1, off=-4, imm=0}"
	", {code=BPF_ALU64|BPF_X|BPF_MOV"
		", dst_reg=BPF_REG_2, src_reg=BPF_REG_10, off=0, imm=0}"
	", {code=BPF_ALU64|BPF_K|BPF_ADD"
		", dst_reg=BPF_REG_2, src_reg=BPF_REG_0, off=0, imm=0xfffffffc}"
	", {code=BPF_LD|BPF_DW|BPF_IMM"
		", dst_reg=BPF_REG_1, src_reg=BPF_REG_1, off=0, imm=%#x}"
	", {code=BPF_LD|BPF_W|BPF_IMM"
		", dst_reg=BPF_REG_0, src_reg=BPF_REG_0, off=0, imm=0}"
	", {code=BPF_LD|BPF_DW|BPF_IMM"
		", dst_reg=BPF_REG_1, src_reg=BPF_REG_1, off=0, imm=%#x}"
	", {code=BPF_LD|BPF_W|BPF_IMM"
		", dst_reg=BPF_REG_0, src_reg=BPF_REG_0, off=0, imm=0}"
	", {code=BPF_JMP|BPF_K|BPF_CALL"
		", dst_reg=BPF_REG_0, src_reg=BPF_REG_0, off=0, imm=0x1}"
	", {code=BPF_ALU64|BPF_K|BPF_MOV"
		", dst_reg=BPF_REG_0, src_reg=BPF_REG_0, off=0, imm=0}"
	", {code=BPF_JMP|BPF_K|BPF_EXIT"
		", dst_reg=BPF_REG_0, src_reg=BPF_REG_0, off=0, imm=0}"
	"]";
# endif /* VERBOSE */

static const char *license = "BSD";
static char log_buf[4096];

static void
print_prog_load(void *attr_void, size_t size, long rc)
{
	printf("bpf(BPF_PROG_LOAD, {prog_type=BPF_PROG_TYPE_SOCKET_FILTER"
	       ", insn_cnt=%zu, insns=", ARRAY_SIZE(socket_prog));
# if VERBOSE
	printf(socket_prog_fmt, socket_prog[4].imm, socket_prog[6].imm);
# else
	printf("%p", socket_prog);
# endif
	if (size > offsetof(struct BPF_PROG_LOAD_struct, license))
		printf(", license=\"BSD\"");
	if (size > offsetof(struct BPF_PROG_LOAD_struct, log_buf))
		printf(", log_level=7, log_size=%zu, log_buf=\"\"",
		       sizeof(log_buf));
	if (size > offsetof(struct BPF_PROG_LOAD_struct, kern_version))
		printf(", kern_version=KERNEL_VERSION(57005, 192, 222)");
	if (size > offsetof(struct BPF_PROG_LOAD_struct, prog_flags))
		printf(", prog_flags=0");
	if (size > offsetof(struct BPF_PROG_LOAD_struct, prog_name))
		printf(", prog_name=\"test_prog\"");
	if (size > offsetof(struct BPF_PROG_LOAD_struct, prog_ifindex))
		printf(", prog_ifindex=0");
	if (size > offsetof(struct BPF_PROG_LOAD_struct, expected_attach_type))
		printf(", expected_attach_type=BPF_CGROUP_INET_INGRESS");
	if (size > offsetof(struct BPF_PROG_LOAD_struct, prog_btf_fd))
		printf(", prog_btf_fd=0</dev/null>");
	if (size > offsetof(struct BPF_PROG_LOAD_struct, func_info_rec_size))
		printf(", func_info_rec_size=0");
	if (size > offsetof(struct BPF_PROG_LOAD_struct, func_info))
		printf(", func_info=NULL");
	if (size > offsetof(struct BPF_PROG_LOAD_struct, func_info_cnt))
		printf(", func_info_cnt=0");
	if (size > offsetof(struct BPF_PROG_LOAD_struct, line_info_rec_size))
		printf(", line_info_rec_size=0");
	if (size > offsetof(struct BPF_PROG_LOAD_struct, line_info))
		printf(", line_info=NULL");
	if (size > offsetof(struct BPF_PROG_LOAD_struct, line_info_cnt))
		printf(", line_info_cnt=0");
	if (size > offsetof(struct BPF_PROG_LOAD_struct, attach_btf_id))
		printf(", attach_btf_id=0");
	if (size > offsetof(struct BPF_PROG_LOAD_struct, attach_prog_fd))
		printf(", attach_prog_fd=0</dev/null>");
	printf("}, %zu) = ", size);
	if (rc >= 0)
		printf("%ld<anon_inode:bpf-prog>\n", rc);
	else
		puts(errstr);
}
#endif /* CHECK_OBJ_PROG */

static long
try_bpf(kernel_ulong_t cmd, void (*printer)(void *attr, size_t size, long rc),
	void *attr, size_t **sizes)
{
	long rc;

	for (rc = -1; **sizes; (*sizes)++) {
		rc = sys_bpf(cmd, attr, **sizes);
		printer(attr, **sizes, rc);

		if (rc >= 0)
			break;
	}

	return rc;
}

int
main(int ac, char **av)
{
	/*
	 * There is a delay when the locked memory is being reclaimed
	 * after a BPF program or map is removed.
	 *
	 * Privileged tools like iproute2 and bpftool workaround this
	 * by raising RLIMIT_MEMLOCK to infinity prior to creating
	 * BPF objects.
	 *
	 * This test is expected to be invoked without extra privileges
	 * and therefore does not have this option.
	 *
	 * The approach taken by this test is serialize all invocations
	 * and insert a delay long enough to let the locked memory be
	 * reclaimed.
	 */
	lock_file_by_dirname(av[0], "bpf-obj_get_info_by_fd");
	sleep(1);

	int ret;

	struct BPF_MAP_CREATE_struct bpf_map_create_attr = {
		.map_type    = BPF_MAP_TYPE_ARRAY,
		.key_size    = 4,
		.value_size  = 8,
		.max_entries = 1,
		.map_name    = "test_map",
	};
	struct BPF_MAP_CREATE_struct bpf_map_create_attr2 = {
		.map_type    = BPF_MAP_TYPE_ARRAY,
		.key_size    = 4,
		.value_size  = 1,
		.max_entries = 5,
		.map_name    = "test_map_too",
	};
	size_t bpf_map_create_attr_sizes[] = {
		sizeof(bpf_map_create_attr),
		offsetofend(struct BPF_MAP_CREATE_struct, max_entries),
		0,
	};

#if CHECK_OBJ_PROG
	struct BPF_PROG_LOAD_struct bpf_prog_load_attr = {
		.prog_type    = BPF_PROG_TYPE_SOCKET_FILTER,
		.insn_cnt     = ARRAY_SIZE(socket_prog),
		.insns        = (uintptr_t) socket_prog,
		.license      = (uintptr_t) license,
		.log_level    = 7,
		.log_size     = sizeof(log_buf),
		.log_buf      = (uintptr_t) log_buf,
		.kern_version = 0xdeadc0de,
		.prog_name    = "test_prog",
	};
	size_t bpf_prog_load_attr_sizes[] = {
		BPF_PROG_LOAD_struct_size,
		offsetofend(struct BPF_PROG_LOAD_struct, prog_name),
		offsetofend(struct BPF_PROG_LOAD_struct, prog_flags),
		offsetofend(struct BPF_PROG_LOAD_struct, kern_version),
		offsetofend(struct BPF_PROG_LOAD_struct, log_buf),
		offsetofend(struct BPF_PROG_LOAD_struct, license),
		offsetofend(struct BPF_PROG_LOAD_struct, insns),
		0,
	};
#endif /* CHECK_OBJ_PROG */

	size_t *bpf_map_create_attr_size = bpf_map_create_attr_sizes;
	int map_fd = try_bpf(BPF_MAP_CREATE, print_map_create,
			     &bpf_map_create_attr, &bpf_map_create_attr_size);
	if (map_fd < 0)
		perror_msg_and_skip("First BPF_MAP_CREATE failed");
	int map_fd2 = try_bpf(BPF_MAP_CREATE, print_map_create,
			     &bpf_map_create_attr2, &bpf_map_create_attr_size);
	if (map_fd2 < 0)
		perror_msg_and_skip("Second BPF_MAP_CREATE failed");

#if CHECK_OBJ_PROG
	socket_prog[4].imm = map_fd2;
	socket_prog[6].imm = map_fd;

	size_t *bpf_prog_load_attr_size = bpf_prog_load_attr_sizes;
	int prog_fd = try_bpf(BPF_PROG_LOAD, print_prog_load,
			      &bpf_prog_load_attr, &bpf_prog_load_attr_size);
	if (prog_fd < 0)
		perror_msg_and_skip("BPF_PROG_LOAD failed (log: \"%s\")",
				    log_buf);
#endif /* CHECK_OBJ_PROG */

	/*
	 * This has to be a macro, otherwise the compiler complains that
	 * initializer element is not constant.
	 */
#define MAP_INFO_SZ (sizeof(*map_info) + 64)
	struct bpf_map_info_struct *map_info = tail_alloc(MAP_INFO_SZ
							  + sizeof(*map_info));
	struct BPF_OBJ_GET_INFO_BY_FD_struct bpf_map_get_info_attr[] = {
		{
			.bpf_fd   = map_fd,
			.info_len = sizeof(*map_info),
			.info     = (uintptr_t) map_info,
		},
		{
			.bpf_fd   = map_fd2,
			.info_len = MAP_INFO_SZ,
			.info     = (uintptr_t) (map_info + 1),
		},
	};

	for (size_t i = 0; i < 2; i++) {
		memset(map_info + i, 0, MAP_INFO_SZ);
		ret = sys_bpf(BPF_OBJ_GET_INFO_BY_FD,
				  &bpf_map_get_info_attr[i],
				  sizeof(bpf_map_get_info_attr[i]));
		if (ret < 0)
			perror_msg_and_skip("BPF_OBJ_GET_INFO_BY_FD map failed");

		printf("bpf(BPF_OBJ_GET_INFO_BY_FD"
		       ", {info={bpf_fd=%d<anon_inode:bpf-map>, info_len=%zu",
		       i ? map_fd2 : map_fd,
		       i ? MAP_INFO_SZ : sizeof(*map_info));
		if (bpf_map_get_info_attr[i].info_len !=
		    (i ? MAP_INFO_SZ : sizeof(*map_info)))
			printf(" => %u", bpf_map_get_info_attr[i].info_len);

		printf(", info=");
#if VERBOSE
		printf("{type=");
		printxval(bpf_map_types, map_info[i].type, "BPF_MAP_TYPE_???");
		printf(", ");
		PRINT_FIELD_U(map_info[i], id);
		printf(", ");
		PRINT_FIELD_U(map_info[i], key_size);
		printf(", ");
		PRINT_FIELD_U(map_info[i], value_size);
		printf(", ");
		PRINT_FIELD_U(map_info[i], max_entries);
		printf(", map_flags=");
		printflags(bpf_map_flags, map_info[i].map_flags, "BPF_F_???");

		if (bpf_map_get_info_attr[i].info_len >
		    offsetof(struct bpf_map_info_struct, name)) {
			printf(", name=");
			print_quoted_cstring(map_info[i].name,
					     sizeof(map_info[i].name));
		}
		if (bpf_map_get_info_attr[i].info_len >
		    offsetof(struct bpf_map_info_struct, ifindex))
			printf(", ifindex=%u", map_info[i].ifindex);
		if (bpf_map_get_info_attr[i].info_len >
		    offsetof(struct bpf_map_info_struct,
			     btf_vmlinux_value_type_id)) {
			printf(", btf_vmlinux_value_type_id=%u",
			       map_info[i].btf_vmlinux_value_type_id);
		}
		if (bpf_map_get_info_attr[i].info_len >
		    offsetof(struct bpf_map_info_struct, netns_dev))
			printf(", netns_dev=makedev(%#x, %#x)",
			       major(map_info[i].netns_dev),
			       minor(map_info[i].netns_dev));
		if (bpf_map_get_info_attr[i].info_len >
		    offsetof(struct bpf_map_info_struct, netns_ino))
			printf(", netns_ino=%" PRIu64, map_info[i].netns_ino);
		if (bpf_map_get_info_attr[i].info_len >
		    offsetof(struct bpf_map_info_struct, btf_id)) {
			printf(", ");
			PRINT_FIELD_U(map_info[i], btf_id);
		}
		if (bpf_map_get_info_attr[i].info_len >
		    offsetof(struct bpf_map_info_struct, btf_key_type_id)) {
			printf(", ");
			PRINT_FIELD_U(map_info[i], btf_key_type_id);
		}
		if (bpf_map_get_info_attr[i].info_len >
		    offsetof(struct bpf_map_info_struct, btf_value_type_id)) {
			printf(", ");
			PRINT_FIELD_U(map_info[i], btf_value_type_id);
		}
		printf("}");
#else /* !VERBOSE */
		printf("%p", map_info + i);
#endif /* VERBOSE */
		printf("}}, %zu) = %s\n", sizeof(bpf_map_get_info_attr[i]),
		       errstr);
	}

#if CHECK_OBJ_PROG
	/*
	 * This has to be a macro, otherwise the compiler complains that
	 * initializer element is not constant.
	 */
# define PROG_INFO_SZ (sizeof(*prog_info) + 64)
	struct bpf_prog_info_struct *prog_info = tail_alloc(PROG_INFO_SZ);
	struct bpf_insn *xlated_prog = tail_alloc(sizeof(*xlated_prog) * 42);
	uint32_t *map_ids = tail_alloc(sizeof(*map_ids) * 3);
	struct BPF_OBJ_GET_INFO_BY_FD_struct bpf_prog_get_info_attr = {
		.bpf_fd   = prog_fd,
		.info_len = PROG_INFO_SZ,
		.info     = (uintptr_t) prog_info,
	};
	size_t old_prog_info_len = PROG_INFO_SZ;

	memset(prog_info, 0, PROG_INFO_SZ);
	for (unsigned int i = 0; i < 5; i++) {
		prog_info->jited_prog_len = 0;
		prog_info->nr_jited_ksyms = 0;
		prog_info->nr_jited_func_lens = 0;
		prog_info->func_info_rec_size = 0;
		prog_info->nr_func_info = 0;
		prog_info->nr_line_info = 0;
		prog_info->nr_jited_line_info = 0;
		prog_info->jited_line_info = 0;
		prog_info->line_info_rec_size = 0;
		prog_info->jited_line_info_rec_size = 0;
		prog_info->nr_prog_tags = 0;
		memset(prog_info + 1, 0, PROG_INFO_SZ - sizeof(*prog_info));
		switch (i) {
		case 1:
			prog_info->xlated_prog_insns =
				(uintptr_t) (xlated_prog + 42);
			prog_info->xlated_prog_len = 336;
			prog_info->map_ids = (uintptr_t) (map_ids + 3);
			prog_info->nr_map_ids = 3;
			break;
		case 2:
			prog_info->xlated_prog_insns = (uintptr_t) xlated_prog;
			/* TODO: check xlated_prog output */
			prog_info->xlated_prog_len = 0;
			prog_info->map_ids = (uintptr_t) map_ids;
			prog_info->nr_map_ids = 0;
			break;
		case 3:
			prog_info->xlated_prog_insns = (uintptr_t) xlated_prog;
			prog_info->xlated_prog_len = 0;
			prog_info->map_ids = (uintptr_t) map_ids;
			prog_info->nr_map_ids = 3;
			break;
		case 4:
			prog_info->xlated_prog_insns = (uintptr_t) xlated_prog;
			prog_info->xlated_prog_len = 1;
			prog_info->map_ids = (uintptr_t) (map_ids + 1);
			prog_info->nr_map_ids = 1;
		}

		ret = sys_bpf(BPF_OBJ_GET_INFO_BY_FD, &bpf_prog_get_info_attr,
			      sizeof(bpf_prog_get_info_attr));
		if (i != 1 && i != 4  && ret < 0)
			perror_msg_and_skip("BPF_OBJ_GET_INFO_BY_FD"
					    " prog %u failed", i);

		printf("bpf(BPF_OBJ_GET_INFO_BY_FD"
		       ", {info={bpf_fd=%d<anon_inode:bpf-prog>, info_len=%zu",
		       prog_fd, old_prog_info_len);
		if (!i && bpf_prog_get_info_attr.info_len != PROG_INFO_SZ)
			printf(" => %u", bpf_prog_get_info_attr.info_len);
		old_prog_info_len = bpf_prog_get_info_attr.info_len;

		printf(", info=");
# if VERBOSE
		printf("{type=");
		printxval(bpf_prog_types, prog_info->type, "BPF_PROG_TYPE_???");
		printf(", ");
		PRINT_FIELD_U(*prog_info, id);
		printf(", tag=");
		print_quoted_hex(prog_info->tag, sizeof(prog_info->tag));
		printf(", jited_prog_len=0");
		if (prog_info->jited_prog_len)
			printf(" => %u", prog_info->jited_prog_len);
		printf(", jited_prog_insns=NULL");
		switch (i) {
		case 0:
			printf(", xlated_prog_len=0");
			if (prog_info->xlated_prog_len)
				printf(" => %u", prog_info->xlated_prog_len);
			printf(", xlated_prog_insns=NULL");
			break;
		case 1:
			printf(", xlated_prog_len=336");
			if (prog_info->xlated_prog_len != 336)
				printf(" => %u", prog_info->xlated_prog_len);
			if (prog_info->xlated_prog_len)
				printf(", xlated_prog_insns=%p", xlated_prog + 42);
			else
				printf(", xlated_prog_insns=[]");
			break;
		case 2:
		case 3:
			printf(", xlated_prog_len=0");
			if (prog_info->xlated_prog_len)
				printf(" => %u", prog_info->xlated_prog_len);
			printf(", xlated_prog_insns=[]");
			break;
		case 4:
			printf(", xlated_prog_len=1 => 0");
			printf(", xlated_prog_insns=[]");
			break;
		}

		if (bpf_prog_get_info_attr.info_len >
		    offsetof(struct bpf_prog_info_struct, load_time))
			printf(", load_time=%" PRIu64, prog_info->load_time);
		if (bpf_prog_get_info_attr.info_len >
		    offsetof(struct bpf_prog_info_struct, created_by_uid))
			printf(", created_by_uid=%u",
			       prog_info->created_by_uid);

		if (bpf_prog_get_info_attr.info_len >
		    offsetof(struct bpf_prog_info_struct, map_ids)) {
			switch (i) {
			case 0:
				printf(", nr_map_ids=0");
				if (prog_info->nr_map_ids)
					printf(" => 2");
				printf(", map_ids=NULL");
				break;
			case 1:
				printf(", nr_map_ids=3, map_ids=%p",
				       map_ids + 3);
				break;
			case 2:
				printf(", nr_map_ids=0");
				if (prog_info->nr_map_ids)
					printf(" => 2");
				printf(", map_ids=[]");
				break;
			case 3:
				printf(", nr_map_ids=3");
				if (prog_info->nr_map_ids != 3)
					printf(" => 2");
				printf(", map_ids=[%u, %u]",
				       map_info[1].id, map_info[0].id);
				break;
			case 4:
				printf(", nr_map_ids=1");
				if (prog_info->nr_map_ids != 1)
					printf(" => 2");
				printf(", map_ids=[%u]", map_info[1].id);
				break;
			}
		}

		if (bpf_prog_get_info_attr.info_len >
		    offsetof(struct bpf_prog_info_struct, name))
			printf(", name=\"test_prog\"");
		if (bpf_prog_get_info_attr.info_len >
		    offsetof(struct bpf_prog_info_struct, ifindex))
			printf(", ifindex=%u", prog_info->ifindex);
		if (bpf_prog_get_info_attr.info_len >
		    offsetofend(struct bpf_prog_info_struct, ifindex))
			printf(", gpl_compatible=%u", prog_info->gpl_compatible);
		if (bpf_prog_get_info_attr.info_len >
		    offsetof(struct bpf_prog_info_struct, netns_dev))
			printf(", netns_dev=makedev(%#x, %#x)",
			       major(prog_info->netns_dev),
			       minor(prog_info->netns_dev));
		if (bpf_prog_get_info_attr.info_len >
		    offsetof(struct bpf_prog_info_struct, netns_ino))
			printf(", netns_ino=%" PRIu64, prog_info->netns_ino);

		if (bpf_prog_get_info_attr.info_len >
		    offsetof(struct bpf_prog_info_struct, nr_jited_ksyms)) {
			printf(", nr_jited_ksyms=0");
			if (prog_info->nr_jited_ksyms)
				printf(" => %u", prog_info->nr_jited_ksyms);
		}
		if (bpf_prog_get_info_attr.info_len >
		    offsetof(struct bpf_prog_info_struct, nr_jited_func_lens)) {
			printf(", nr_jited_func_lens=0");
			if (prog_info->nr_jited_func_lens)
				printf(" => %u", prog_info->nr_jited_func_lens);
		}
		if (bpf_prog_get_info_attr.info_len >
		    offsetof(struct bpf_prog_info_struct, jited_ksyms))
			printf(", jited_ksyms=NULL");
		if (bpf_prog_get_info_attr.info_len >
		    offsetof(struct bpf_prog_info_struct, jited_func_lens))
			printf(", jited_func_lens=NULL");

		if (bpf_prog_get_info_attr.info_len >
		    offsetof(struct bpf_prog_info_struct, btf_id)) {
			printf(", ");
			PRINT_FIELD_U(*prog_info, btf_id);
		}
		if (bpf_prog_get_info_attr.info_len >
		    offsetof(struct bpf_prog_info_struct, func_info_rec_size)) {
			printf(", func_info_rec_size=0");
			if (prog_info->func_info_rec_size)
				printf(" => %u", prog_info->func_info_rec_size);
		}
		if (bpf_prog_get_info_attr.info_len >
		    offsetof(struct bpf_prog_info_struct, func_info))
			printf(", func_info=NULL");
		if (bpf_prog_get_info_attr.info_len >
		    offsetof(struct bpf_prog_info_struct, nr_func_info)) {
			printf(", nr_func_info=0");
			if (prog_info->nr_func_info)
				printf(" => %u", prog_info->nr_func_info);
		}
		if (bpf_prog_get_info_attr.info_len >
		    offsetof(struct bpf_prog_info_struct, nr_line_info)) {
			printf(", nr_line_info=0");
			if (prog_info->nr_line_info)
				printf(" => %u", prog_info->nr_line_info);
		}
		if (bpf_prog_get_info_attr.info_len >
		    offsetof(struct bpf_prog_info_struct, line_info))
			printf(", line_info=NULL");
		if (bpf_prog_get_info_attr.info_len >
		    offsetof(struct bpf_prog_info_struct, jited_line_info))
			printf(", jited_line_info=NULL");
		if (bpf_prog_get_info_attr.info_len >
		    offsetof(struct bpf_prog_info_struct, nr_jited_line_info)) {
			printf(", nr_jited_line_info=0");
			if (prog_info->nr_jited_line_info)
				printf(" => %u", prog_info->nr_jited_line_info);
		}
		if (bpf_prog_get_info_attr.info_len >
		    offsetof(struct bpf_prog_info_struct, line_info_rec_size)) {
			printf(", line_info_rec_size=0");
			if (prog_info->line_info_rec_size)
				printf(" => %u", prog_info->line_info_rec_size);
		}
		if (bpf_prog_get_info_attr.info_len >
		    offsetof(struct bpf_prog_info_struct, jited_line_info_rec_size)) {
			printf(", jited_line_info_rec_size=0");
			if (prog_info->jited_line_info_rec_size)
				printf(" => %u", prog_info->jited_line_info_rec_size);
		}
		if (bpf_prog_get_info_attr.info_len >
		    offsetof(struct bpf_prog_info_struct, nr_prog_tags)) {
			printf(", nr_prog_tags=0");
			if (prog_info->nr_prog_tags)
				printf(" => %u", prog_info->nr_prog_tags);
		}
		if (bpf_prog_get_info_attr.info_len >
		    offsetof(struct bpf_prog_info_struct, prog_tags))
			printf(", prog_tags=NULL");
		if (bpf_prog_get_info_attr.info_len >
		    offsetof(struct bpf_prog_info_struct, run_time_ns))
			printf(", run_time_ns=%llu",
			       (unsigned long long) prog_info->run_time_ns);
		if (bpf_prog_get_info_attr.info_len >
		    offsetof(struct bpf_prog_info_struct, run_cnt))
			printf(", run_cnt=%llu",
			       (unsigned long long) prog_info->run_cnt);

		printf("}");
# else /* !VERBOSE */
		printf("%p", prog_info);
# endif /* VERBOSE */
		printf("}}, %zu) = %s\n",
		       sizeof(bpf_prog_get_info_attr), errstr);
	}
#endif /* CHECK_OBJ_PROG */

	puts("+++ exited with 0 +++");
	return 0;
}
