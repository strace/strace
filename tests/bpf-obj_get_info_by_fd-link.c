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
#include <mntent.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "xmalloc.h"
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

static void
test_struct_ops_info(int link_fd, const char *link_fd_path)
{
	static const unsigned int sizes[] = {
		offsetof(struct bpf_link_info_struct, struct_ops.cgroup_id),
		offsetof(struct bpf_link_info_struct, struct_ops.cgroup_id) + 1,
		/* A complete field containing zero, then a nonzero value. */
		offsetofend(struct bpf_link_info_struct, struct_ops.cgroup_id),
		offsetofend(struct bpf_link_info_struct, struct_ops.cgroup_id),
		bpf_link_info_struct_size,
		bpf_link_info_struct_size + 1,
	};
	unsigned char *buf = tail_alloc(bpf_link_info_struct_size + 1);

	for (unsigned int i = 0; i < ARRAY_SIZE(sizes); ++i) {
		struct bpf_link_info_struct info = {
			.type = BPF_LINK_TYPE_STRUCT_OPS,
			.id = 42,
			.struct_ops = {
				.map_id = 0xfacefeed,
				.pad_ = -1U,
				.cgroup_id = i == 2 ? 0 : 0xfedcba9876543210ULL,
			},
		};
		union {
			struct BPF_OBJ_GET_INFO_BY_FD_struct attr;
			unsigned char bytes[
				BPF_OBJ_GET_INFO_BY_FD_struct_size + 1];
		} data = {
			.attr = {
				.bpf_fd = link_fd,
				.info_len = sizes[i],
				.info = (uintptr_t) buf,
			},
		};

		memcpy(buf, &info, bpf_link_info_struct_size);
		buf[bpf_link_info_struct_size] = 0xab;
		/*
		 * Reject the syscall before the kernel overwrites the synthetic
		 * info, while retaining a real BPF link FD for object detection.
		 */
		data.bytes[BPF_OBJ_GET_INFO_BY_FD_struct_size] = 1;
		if (sys_bpf(BPF_OBJ_GET_INFO_BY_FD, &data,
			    BPF_OBJ_GET_INFO_BY_FD_struct_size + 1) != -1)
			error_msg_and_fail("invalid attribute tail accepted");

		struct bpf_link_info_struct expected = {};

		memcpy(&expected, buf, MIN(sizes[i], sizeof(expected)));
		printf("bpf(BPF_OBJ_GET_INFO_BY_FD"
		       ", {info={bpf_fd=%d<%s>, info_len=%u"
		       ", info={type=BPF_LINK_TYPE_STRUCT_OPS"
		       ", id=42, prog_id=0"
		       ", struct_ops={map_id=4207869677",
		       link_fd, link_fd_path, sizes[i]);
		if (sizes[i] > offsetof(struct bpf_link_info_struct,
					struct_ops.cgroup_id))
			printf(", cgroup_id=%" PRIu64,
			       expected.struct_ops.cgroup_id);
		printf("}");
		if (sizes[i] > bpf_link_info_struct_size)
			printf(", extra_data=\"\\xab\" /* bytes 64..64 */");
		printf("}}, extra_data=\"\\x01\" /* bytes 16..16 */}"
		       ", %zu) = %s\n",
		       BPF_OBJ_GET_INFO_BY_FD_struct_size + 1, errstr);
	}
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

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

	/* Find the cgroup2 mount point */
	char *cg_mnt = NULL;
	FILE *f = setmntent("/proc/mounts", "r");
	if (f) {
		struct mntent *me;
		while ((me = getmntent(f))) {
			if (!strcmp(me->mnt_type, "cgroup2")) {
				cg_mnt = strdup(me->mnt_dir);
				break;
			}
		}
		endmntent(f);
	}
	if (!cg_mnt)
		error_msg_and_skip("cgroup2 not mounted");

	/*
	 * Try the process's own cgroup first, then fall back
	 * to the cgroup2 root.
	 */
	int cgroup_fd = -1;
	f = fopen("/proc/self/cgroup", "r");
	if (f) {
		char *line = NULL;
		size_t len = 0;
		while (getline(&line, &len, f) > 0) {
			char *path = strchr(line, '/');
			if (!path)
				continue;
			char *nl = strchr(path, '\n');
			if (nl)
				*nl = '\0';
			char *cg_path = xasprintf("%s%s", cg_mnt, path);
			cgroup_fd = open(cg_path, O_RDONLY | O_DIRECTORY);
			free(cg_path);
			if (cgroup_fd >= 0)
				break;
		}
		free(line);
		fclose(f);
	}
	if (cgroup_fd < 0)
		cgroup_fd = open(cg_mnt, O_RDONLY | O_DIRECTORY);
	free(cg_mnt);
	if (cgroup_fd < 0)
		perror_msg_and_skip("open cgroup");

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

	char *link_fd_path = get_fd_path(link_fd);
	printf("bpf(BPF_OBJ_GET_INFO_BY_FD"
	       ", {info={bpf_fd=%d<%s>"
	       ", info_len=%zu",
	       link_fd, link_fd_path, LINK_INFO_SZ);
	if (get_info_attr.info_len != LINK_INFO_SZ)
		printf(" => %u", get_info_attr.info_len);
	printf(", info={type=");
	printxval(bpf_link_type, link_info->type, "BPF_LINK_TYPE_???");
	printf(", id=%u", link_info->id);
	printf(", prog_id=%u", link_info->prog_id);
	printf(", cgroup={cgroup_id=%" PRIu64, link_info->cgroup.cgroup_id);
	printf(", attach_type=");
	printxval(bpf_attach_type, link_info->cgroup.attach_type, "BPF_???");
	printf("}}}}, %zu) = %s\n", sizeof(get_info_attr), errstr);

	test_struct_ops_info(link_fd, link_fd_path);
	free(link_fd_path);
	puts("+++ exited with 0 +++");
	return 0;
}
