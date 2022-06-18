/*
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2018-2022 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_BPF_ATTR_H
# define STRACE_BPF_ATTR_H

/*
 * The policy is that all fields of type uint64_t in this header file
 * must have ATTRIBUTE_ALIGNED(8).
 *
 * This should not cause any contradictions with <linux/bpf.h>
 * unless the latter is buggy.
 *
 * By word "buggy" I mean containing such changes as Linux kernel commit
 * v4.16-rc1~123^2~109^2~5^2~4.
 */

# ifndef BPF_OBJ_NAME_LEN
#  define BPF_OBJ_NAME_LEN 16U
# else
#  if BPF_OBJ_NAME_LEN != 16U
#   error "Unexpected value of BPF_OBJ_NAME_LEN"
#  endif
# endif

# ifndef BPF_TAG_SIZE
#  define BPF_TAG_SIZE 8
# else
#  if BPF_TAG_SIZE != 8
#   error "Unexpected value of BPF_TAG_SIZE"
#  endif
# endif

struct BPF_MAP_CREATE_struct {
	uint32_t map_type;
	uint32_t key_size;
	uint32_t value_size;
	uint32_t max_entries;
	uint32_t map_flags;
	uint32_t inner_map_fd;
	uint32_t numa_node;
	char     map_name[BPF_OBJ_NAME_LEN];
	uint32_t map_ifindex;
	uint32_t btf_fd;
	uint32_t btf_key_type_id;
	uint32_t btf_value_type_id;
	uint32_t btf_vmlinux_value_type_id;
	uint64_t ATTRIBUTE_ALIGNED(8) map_extra;
};

# define BPF_MAP_CREATE_struct_size \
	sizeof(struct BPF_MAP_CREATE_struct)
# define expected_BPF_MAP_CREATE_struct_size 72

struct BPF_MAP_LOOKUP_ELEM_struct {
	uint32_t map_fd;
	uint64_t ATTRIBUTE_ALIGNED(8) key;
	uint64_t ATTRIBUTE_ALIGNED(8) value;
	uint64_t ATTRIBUTE_ALIGNED(8) flags;
};

# define BPF_MAP_LOOKUP_ELEM_struct_size \
	sizeof(struct BPF_MAP_LOOKUP_ELEM_struct)
# define expected_BPF_MAP_LOOKUP_ELEM_struct_size 32

struct BPF_MAP_UPDATE_ELEM_struct {
	uint32_t map_fd;
	uint64_t ATTRIBUTE_ALIGNED(8) key;
	uint64_t ATTRIBUTE_ALIGNED(8) value;
	uint64_t ATTRIBUTE_ALIGNED(8) flags;
};

# define BPF_MAP_UPDATE_ELEM_struct_size \
	sizeof(struct BPF_MAP_UPDATE_ELEM_struct)
# define expected_BPF_MAP_UPDATE_ELEM_struct_size 32

struct BPF_MAP_DELETE_ELEM_struct {
	uint32_t map_fd;
	uint64_t ATTRIBUTE_ALIGNED(8) key;
};

# define BPF_MAP_DELETE_ELEM_struct_size \
	sizeof(struct BPF_MAP_DELETE_ELEM_struct)
# define expected_BPF_MAP_DELETE_ELEM_struct_size 16

struct BPF_MAP_GET_NEXT_KEY_struct {
	uint32_t map_fd;
	uint64_t ATTRIBUTE_ALIGNED(8) key;
	uint64_t ATTRIBUTE_ALIGNED(8) next_key;
};

# define BPF_MAP_GET_NEXT_KEY_struct_size \
	sizeof(struct BPF_MAP_GET_NEXT_KEY_struct)
# define expected_BPF_MAP_GET_NEXT_KEY_struct_size 24

struct BPF_MAP_FREEZE_struct {
	uint32_t map_fd;
};

# define BPF_MAP_FREEZE_struct_size \
	sizeof(struct BPF_MAP_FREEZE_struct)
# define expected_BPF_MAP_FREEZE_struct_size 4

struct BPF_PROG_LOAD_struct {
	uint32_t prog_type;
	uint32_t insn_cnt;
	uint64_t ATTRIBUTE_ALIGNED(8) insns;
	uint64_t ATTRIBUTE_ALIGNED(8) license;
	uint32_t log_level;
	uint32_t log_size;
	uint64_t ATTRIBUTE_ALIGNED(8) log_buf;
	uint32_t kern_version;
	uint32_t prog_flags;
	char     prog_name[BPF_OBJ_NAME_LEN];
	uint32_t prog_ifindex;
	uint32_t expected_attach_type;
	uint32_t prog_btf_fd;
	uint32_t func_info_rec_size;
	uint64_t ATTRIBUTE_ALIGNED(8) func_info;
	uint32_t func_info_cnt;
	uint32_t line_info_rec_size;
	uint64_t ATTRIBUTE_ALIGNED(8) line_info;
	uint32_t line_info_cnt;
	uint32_t attach_btf_id;
	uint32_t attach_prog_fd;
	uint32_t pad;
	uint64_t ATTRIBUTE_ALIGNED(8) fd_array;
};

# define BPF_PROG_LOAD_struct_size \
	sizeof(struct BPF_PROG_LOAD_struct)
# define expected_BPF_PROG_LOAD_struct_size 128

struct BPF_OBJ_PIN_struct {
	uint64_t ATTRIBUTE_ALIGNED(8) pathname;
	uint32_t bpf_fd;
	uint32_t file_flags;
};

# define BPF_OBJ_PIN_struct_size \
	sizeof(struct BPF_OBJ_PIN_struct)
# define expected_BPF_OBJ_PIN_struct_size 16

# define BPF_OBJ_GET_struct BPF_OBJ_PIN_struct
# define BPF_OBJ_GET_struct_size BPF_OBJ_PIN_struct_size

struct BPF_PROG_ATTACH_struct {
	uint32_t target_fd;
	uint32_t attach_bpf_fd;
	uint32_t attach_type;
	uint32_t attach_flags;
	uint32_t replace_bpf_fd;
};

# define BPF_PROG_ATTACH_struct_size \
	sizeof(struct BPF_PROG_ATTACH_struct)
# define expected_BPF_PROG_ATTACH_struct_size 20

struct BPF_PROG_DETACH_struct {
	uint32_t target_fd;
	uint32_t dummy;
	uint32_t attach_type;
};

# define BPF_PROG_DETACH_struct_size \
	sizeof(struct BPF_PROG_DETACH_struct)
# define expected_BPF_PROG_DETACH_struct_size 12

struct BPF_PROG_TEST_RUN_struct /* test */ {
	uint32_t prog_fd;
	uint32_t retval;
	uint32_t data_size_in;
	uint32_t data_size_out;
	uint64_t ATTRIBUTE_ALIGNED(8) data_in;
	uint64_t ATTRIBUTE_ALIGNED(8) data_out;
	uint32_t repeat;
	uint32_t duration;
	uint32_t ctx_size_in;
	uint32_t ctx_size_out;
	uint64_t ATTRIBUTE_ALIGNED(8) ctx_in;
	uint64_t ATTRIBUTE_ALIGNED(8) ctx_out;
	uint32_t flags;
	uint32_t cpu;
	uint32_t batch_size;
};

# define BPF_PROG_TEST_RUN_struct_size \
	offsetofend(struct BPF_PROG_TEST_RUN_struct, batch_size)
	/* sizeof(struct BPF_PROG_TEST_RUN_struct) */
# define expected_BPF_PROG_TEST_RUN_struct_size 76

struct BPF_PROG_GET_NEXT_ID_struct {
	uint32_t start_id;
	uint32_t next_id;
	uint32_t open_flags;
};

# define BPF_PROG_GET_NEXT_ID_struct_size \
	sizeof(struct BPF_PROG_GET_NEXT_ID_struct)
# define expected_BPF_PROG_GET_NEXT_ID_struct_size 12

# define BPF_MAP_GET_NEXT_ID_struct BPF_PROG_GET_NEXT_ID_struct
# define BPF_MAP_GET_NEXT_ID_struct_size BPF_PROG_GET_NEXT_ID_struct_size

# define BPF_BTF_GET_NEXT_ID_struct BPF_PROG_GET_NEXT_ID_struct
# define BPF_BTF_GET_NEXT_ID_struct_size BPF_PROG_GET_NEXT_ID_struct_size

struct BPF_PROG_GET_FD_BY_ID_struct {
	uint32_t prog_id;
	uint32_t next_id;
	uint32_t open_flags;
};

# define BPF_PROG_GET_FD_BY_ID_struct_size \
	sizeof(struct BPF_PROG_GET_FD_BY_ID_struct)
# define expected_BPF_PROG_GET_FD_BY_ID_struct_size 12

struct BPF_MAP_GET_FD_BY_ID_struct {
	uint32_t map_id;
	uint32_t next_id;
	uint32_t open_flags;
};

# define BPF_MAP_GET_FD_BY_ID_struct_size \
	sizeof(struct BPF_MAP_GET_FD_BY_ID_struct)
# define expected_BPF_MAP_GET_FD_BY_ID_struct_size 12

struct BPF_OBJ_GET_INFO_BY_FD_struct /* info */ {
	uint32_t bpf_fd;
	uint32_t info_len;
	uint64_t ATTRIBUTE_ALIGNED(8) info;
};

# define BPF_OBJ_GET_INFO_BY_FD_struct_size \
	sizeof(struct BPF_OBJ_GET_INFO_BY_FD_struct)
# define expected_BPF_OBJ_GET_INFO_BY_FD_struct_size 16

struct BPF_PROG_QUERY_struct /* query */ {
	uint32_t target_fd;
	uint32_t attach_type;
	uint32_t query_flags;
	uint32_t attach_flags;
	uint64_t ATTRIBUTE_ALIGNED(8) prog_ids;
	uint32_t prog_cnt;
};

# define BPF_PROG_QUERY_struct_size \
	offsetofend(struct BPF_PROG_QUERY_struct, prog_cnt)
# define expected_BPF_PROG_QUERY_struct_size 28

struct BPF_RAW_TRACEPOINT_OPEN_struct /* raw_tracepoint */ {
	uint64_t ATTRIBUTE_ALIGNED(8) name;
	uint32_t prog_fd;
};

# define BPF_RAW_TRACEPOINT_OPEN_struct_size \
	offsetofend(struct BPF_RAW_TRACEPOINT_OPEN_struct, prog_fd)
# define expected_BPF_RAW_TRACEPOINT_OPEN_struct_size 12

struct BPF_BTF_LOAD_struct {
	uint64_t ATTRIBUTE_ALIGNED(8) btf;
	uint64_t ATTRIBUTE_ALIGNED(8) btf_log_buf;
	uint32_t btf_size;
	uint32_t btf_log_size;
	uint32_t btf_log_level;
};

# define BPF_BTF_LOAD_struct_size \
	offsetofend(struct BPF_BTF_LOAD_struct, btf_log_level)
# define expected_BPF_BTF_LOAD_struct_size 28

struct BPF_BTF_GET_FD_BY_ID_struct {
	uint32_t btf_id;
};

# define BPF_BTF_GET_FD_BY_ID_struct_size \
	sizeof(struct BPF_BTF_GET_FD_BY_ID_struct)
# define expected_BPF_BTF_GET_FD_BY_ID_struct_size 4

struct BPF_TASK_FD_QUERY_struct /* task_fd_query */ {
	uint32_t pid;
	uint32_t fd;
	uint32_t flags;
	uint32_t buf_len;
	uint64_t ATTRIBUTE_ALIGNED(8) buf;
	uint32_t prog_id;
	uint32_t fd_type;
	uint64_t ATTRIBUTE_ALIGNED(8) probe_offset;
	uint64_t ATTRIBUTE_ALIGNED(8) probe_addr;
};

# define BPF_TASK_FD_QUERY_struct_size \
	sizeof(struct BPF_TASK_FD_QUERY_struct)
# define expected_BPF_TASK_FD_QUERY_struct_size 48

struct bpf_map_info_struct {
	uint32_t type;
	uint32_t id;
	uint32_t key_size;
	uint32_t value_size;
	uint32_t max_entries;
	uint32_t map_flags;
	char     name[BPF_OBJ_NAME_LEN];
	uint32_t ifindex;
	uint32_t btf_vmlinux_value_type_id;
	/*
	 * The kernel UAPI is broken by Linux commit
	 * v4.16-rc1~123^2~109^2~5^2~4 .
	 */
	uint64_t ATTRIBUTE_ALIGNED(8) netns_dev; /* skip check */
	uint64_t ATTRIBUTE_ALIGNED(8) netns_ino; /* skip check */
	uint32_t btf_id;
	uint32_t btf_key_type_id;
	uint32_t btf_value_type_id;
	uint32_t pad;
	uint64_t ATTRIBUTE_ALIGNED(8) map_extra;
};

# define bpf_map_info_struct_size \
	sizeof(struct bpf_map_info_struct)
# define expected_bpf_map_info_struct_size 88

struct bpf_prog_info_struct {
	uint32_t type;
	uint32_t id;
	uint8_t  tag[BPF_TAG_SIZE];
	uint32_t jited_prog_len;
	uint32_t xlated_prog_len;
	uint64_t ATTRIBUTE_ALIGNED(8) jited_prog_insns;
	uint64_t ATTRIBUTE_ALIGNED(8) xlated_prog_insns;
	uint64_t ATTRIBUTE_ALIGNED(8) load_time;
	uint32_t created_by_uid;
	uint32_t nr_map_ids;
	uint64_t ATTRIBUTE_ALIGNED(8) map_ids;
	char     name[BPF_OBJ_NAME_LEN];
	uint32_t ifindex;
	uint32_t gpl_compatible:1;
	/*
	 * The kernel UAPI is broken by Linux commit
	 * v4.16-rc1~123^2~227^2~5^2~2 .
	 */
	uint64_t ATTRIBUTE_ALIGNED(8) netns_dev; /* skip check */
	uint64_t ATTRIBUTE_ALIGNED(8) netns_ino; /* skip check */
	uint32_t nr_jited_ksyms;
	uint32_t nr_jited_func_lens;
	uint64_t ATTRIBUTE_ALIGNED(8) jited_ksyms;
	uint64_t ATTRIBUTE_ALIGNED(8) jited_func_lens;
	uint32_t btf_id;
	uint32_t func_info_rec_size;
	uint64_t ATTRIBUTE_ALIGNED(8) func_info;
	uint32_t nr_func_info;
	uint32_t nr_line_info;
	uint64_t ATTRIBUTE_ALIGNED(8) line_info;
	uint64_t ATTRIBUTE_ALIGNED(8) jited_line_info;
	uint32_t nr_jited_line_info;
	uint32_t line_info_rec_size;
	uint32_t jited_line_info_rec_size;
	uint32_t nr_prog_tags;
	uint64_t ATTRIBUTE_ALIGNED(8) prog_tags;
	uint64_t ATTRIBUTE_ALIGNED(8) run_time_ns;
	uint64_t ATTRIBUTE_ALIGNED(8) run_cnt;
	uint64_t ATTRIBUTE_ALIGNED(8) recursion_misses;
	uint32_t verified_insns;
};

# define bpf_prog_info_struct_size \
	offsetofend(struct bpf_prog_info_struct, verified_insns)
# define expected_bpf_prog_info_struct_size 220

struct BPF_MAP_LOOKUP_BATCH_struct /* batch */ {
	uint64_t ATTRIBUTE_ALIGNED(8) in_batch;
	uint64_t ATTRIBUTE_ALIGNED(8) out_batch;
	uint64_t ATTRIBUTE_ALIGNED(8) keys;
	uint64_t ATTRIBUTE_ALIGNED(8) values;
	uint32_t count;
	uint32_t map_fd;
	uint64_t ATTRIBUTE_ALIGNED(8) elem_flags;
	uint64_t ATTRIBUTE_ALIGNED(8) flags;
};

# define BPF_MAP_LOOKUP_BATCH_struct_size \
	sizeof(struct BPF_MAP_LOOKUP_BATCH_struct)
# define expected_BPF_MAP_LOOKUP_BATCH_struct_size 56

# define BPF_MAP_LOOKUP_AND_DELETE_BATCH_struct BPF_MAP_LOOKUP_BATCH_struct
# define BPF_MAP_LOOKUP_AND_DELETE_BATCH_struct_size BPF_MAP_LOOKUP_BATCH_struct_size

# define BPF_MAP_UPDATE_BATCH_struct BPF_MAP_LOOKUP_BATCH_struct
# define BPF_MAP_UPDATE_BATCH_struct_size BPF_MAP_LOOKUP_BATCH_struct_size

# define BPF_MAP_DELETE_BATCH_struct BPF_MAP_LOOKUP_BATCH_struct
# define BPF_MAP_DELETE_BATCH_struct_size BPF_MAP_LOOKUP_BATCH_struct_size

struct BPF_LINK_CREATE_struct /* link_create */ {
	uint32_t prog_fd;
	uint32_t target_fd;
	uint32_t attach_type;
	uint32_t flags;
	union {
		uint32_t target_btf_id;

		struct {
			uint64_t ATTRIBUTE_ALIGNED(8) iter_info;
			uint32_t iter_info_len;
		};

		struct {
			uint64_t ATTRIBUTE_ALIGNED(8) bpf_cookie;
		} perf_event;

		struct {
			uint32_t flags;
			uint32_t cnt;
			uint64_t ATTRIBUTE_ALIGNED(8) syms;
			uint64_t ATTRIBUTE_ALIGNED(8) addrs;
			uint64_t ATTRIBUTE_ALIGNED(8) cookies;
		} kprobe_multi;
	};
};

# define BPF_LINK_CREATE_struct_size \
	sizeof(struct BPF_LINK_CREATE_struct)
# define expected_BPF_LINK_CREATE_struct_size 48

struct BPF_LINK_UPDATE_struct /* link_update */ {
	uint32_t link_fd;
	uint32_t new_prog_fd;
	uint32_t flags;
	uint32_t old_prog_fd;
};

# define BPF_LINK_UPDATE_struct_size \
	sizeof(struct BPF_LINK_UPDATE_struct)
# define expected_BPF_LINK_UPDATE_struct_size 16

struct BPF_LINK_GET_FD_BY_ID_struct {
	uint32_t link_id;
};

# define BPF_LINK_GET_FD_BY_ID_struct_size \
	sizeof(struct BPF_LINK_GET_FD_BY_ID_struct)
# define expected_BPF_LINK_GET_FD_BY_ID_struct_size 4

#endif /* !STRACE_BPF_ATTR_H */
