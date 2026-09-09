/*
 * Check bpf(BPF_OBJ_GET_INFO_BY_FD) decoding for bpf-link objects.
 *
 * Copyright (c) 2018-2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <fcntl.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "scno.h"

#include <linux/bpf.h>
#include "bpf_attr.h"

#include "xlat.h"
#include "xlat/bpf_attach_type.h"
#include "xlat/bpf_link_type.h"

#define XLAT_MACROS_ONLY
#include "xlat/bpf_commands.h"
#undef XLAT_MACROS_ONLY

static const char *errstr;

static long
sys_bpf(kernel_ulong_t cmd, void *attr, kernel_ulong_t size)
{
	long rc = syscall(__NR_bpf, cmd, attr, size);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	static const struct bpf_insn cgroup_prog[] = {
		{ /* r0 = 1 (allow) */
			.code    = BPF_ALU64 | BPF_K | BPF_MOV,
			.dst_reg = BPF_REG_0,
			.imm     = 1,
		},
		{ /* exit */
			.code = BPF_JMP | BPF_K | BPF_EXIT,
		},
	};
	static const char license[] = "GPL";

	struct BPF_PROG_LOAD_struct prog_attr = {
		.prog_type          = BPF_PROG_TYPE_CGROUP_SKB,
		.insn_cnt           = ARRAY_SIZE(cgroup_prog),
		.insns              = (uintptr_t) cgroup_prog,
		.license            = (uintptr_t) license,
		.expected_attach_type = BPF_CGROUP_INET_EGRESS,
	};

	int prog_fd = sys_bpf(BPF_PROG_LOAD, &prog_attr,
			      sizeof(prog_attr));
	if (prog_fd < 0)
		perror_msg_and_skip("BPF_PROG_LOAD");

	int cgroup_fd = open("/sys/fs/cgroup", O_RDONLY | O_DIRECTORY);
	if (cgroup_fd < 0)
		perror_msg_and_skip("open /sys/fs/cgroup");

	struct BPF_LINK_CREATE_struct link_attr = {
		.prog_fd     = prog_fd,
		.target_fd   = cgroup_fd,
		.attach_type = BPF_CGROUP_INET_EGRESS,
	};

	int link_fd = sys_bpf(BPF_LINK_CREATE, &link_attr,
			      sizeof(link_attr));
	if (link_fd < 0)
		perror_msg_and_skip("BPF_LINK_CREATE");

#define LINK_INFO_SZ sizeof(struct bpf_link_info_struct)
	struct bpf_link_info_struct *link_info = tail_alloc(LINK_INFO_SZ);
	struct BPF_OBJ_GET_INFO_BY_FD_struct get_info_attr = {
		.bpf_fd   = link_fd,
		.info_len = LINK_INFO_SZ,
		.info     = (uintptr_t) link_info,
	};

	memset(link_info, 0, LINK_INFO_SZ);
	long ret = sys_bpf(BPF_OBJ_GET_INFO_BY_FD, &get_info_attr,
			   sizeof(get_info_attr));
	if (ret < 0)
		perror_msg_and_skip("BPF_OBJ_GET_INFO_BY_FD");

	printf("bpf(BPF_OBJ_GET_INFO_BY_FD"
	       ", {info={bpf_fd=%d<anon_inode:bpf-link>"
	       ", info_len=%zu",
	       link_fd, LINK_INFO_SZ);
	if (get_info_attr.info_len != LINK_INFO_SZ)
		printf(" => %u", get_info_attr.info_len);
	printf(", info={type=");
	printxval(bpf_link_type, link_info->type, "BPF_LINK_TYPE_???");
	printf(", id=%u", link_info->id);
	printf(", prog_id=%u", link_info->prog_id);
	printf(", cgroup={cgroup_id=%" PRIu64, link_info->cgroup.cgroup_id);
	printf(", attach_type=");
	printxval(bpf_attach_type, link_info->cgroup.attach_type, "BPF_???");
	printf("}}}, %zu) = %s\n", sizeof(get_info_attr), errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
