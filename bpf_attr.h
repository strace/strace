/*
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@altlinux.org>
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

#ifndef STRACE_BPF_ATTR_H
#define STRACE_BPF_ATTR_H

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

#ifndef BPF_OBJ_NAME_LEN
# define BPF_OBJ_NAME_LEN 16U
#else
# if BPF_OBJ_NAME_LEN != 16U
#  error "Unexpected value of BPF_OBJ_NAME_LEN"
# endif
#endif

#ifndef BPF_TAG_SIZE
# define BPF_TAG_SIZE 8
#else
# if BPF_TAG_SIZE != 8
#  error "Unexpected value of BPF_TAG_SIZE"
# endif
#endif

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
};

#define BPF_MAP_CREATE_struct_size \
	sizeof(struct BPF_MAP_CREATE_struct)
#define expected_BPF_MAP_CREATE_struct_size 48

struct BPF_MAP_LOOKUP_ELEM_struct {
	uint32_t map_fd;
	uint64_t ATTRIBUTE_ALIGNED(8) key;
	uint64_t ATTRIBUTE_ALIGNED(8) value;
};

#define BPF_MAP_LOOKUP_ELEM_struct_size \
	sizeof(struct BPF_MAP_LOOKUP_ELEM_struct)
#define expected_BPF_MAP_LOOKUP_ELEM_struct_size 24

struct BPF_MAP_UPDATE_ELEM_struct {
	uint32_t map_fd;
	uint64_t ATTRIBUTE_ALIGNED(8) key;
	uint64_t ATTRIBUTE_ALIGNED(8) value;
	uint64_t ATTRIBUTE_ALIGNED(8) flags;
};

#define BPF_MAP_UPDATE_ELEM_struct_size \
	sizeof(struct BPF_MAP_UPDATE_ELEM_struct)
#define expected_BPF_MAP_UPDATE_ELEM_struct_size 32

struct BPF_MAP_DELETE_ELEM_struct {
	uint32_t map_fd;
	uint64_t ATTRIBUTE_ALIGNED(8) key;
};

#define BPF_MAP_DELETE_ELEM_struct_size \
	sizeof(struct BPF_MAP_DELETE_ELEM_struct)
#define expected_BPF_MAP_DELETE_ELEM_struct_size 16

struct BPF_MAP_GET_NEXT_KEY_struct {
	uint32_t map_fd;
	uint64_t ATTRIBUTE_ALIGNED(8) key;
	uint64_t ATTRIBUTE_ALIGNED(8) next_key;
};

#define BPF_MAP_GET_NEXT_KEY_struct_size \
	sizeof(struct BPF_MAP_GET_NEXT_KEY_struct)
#define expected_BPF_MAP_GET_NEXT_KEY_struct_size 24

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
};

#define BPF_PROG_LOAD_struct_size \
	offsetofend(struct BPF_PROG_LOAD_struct, expected_attach_type)
#define expected_BPF_PROG_LOAD_struct_size 72

struct BPF_OBJ_PIN_struct {
	uint64_t ATTRIBUTE_ALIGNED(8) pathname;
	uint32_t bpf_fd;
	uint32_t file_flags;
};

#define BPF_OBJ_PIN_struct_size \
	sizeof(struct BPF_OBJ_PIN_struct)
#define expected_BPF_OBJ_PIN_struct_size 16

#define BPF_OBJ_GET_struct BPF_OBJ_PIN_struct
#define BPF_OBJ_GET_struct_size BPF_OBJ_PIN_struct_size

struct BPF_PROG_ATTACH_struct {
	uint32_t target_fd;
	uint32_t attach_bpf_fd;
	uint32_t attach_type;
	uint32_t attach_flags;
};

#define BPF_PROG_ATTACH_struct_size \
	sizeof(struct BPF_PROG_ATTACH_struct)
#define expected_BPF_PROG_ATTACH_struct_size 16

struct BPF_PROG_DETACH_struct {
	uint32_t target_fd;
	uint32_t dummy;
	uint32_t attach_type;
};

#define BPF_PROG_DETACH_struct_size \
	sizeof(struct BPF_PROG_DETACH_struct)
#define expected_BPF_PROG_DETACH_struct_size 12

struct BPF_PROG_TEST_RUN_struct /* test */ {
	uint32_t prog_fd;
	uint32_t retval;
	uint32_t data_size_in;
	uint32_t data_size_out;
	uint64_t ATTRIBUTE_ALIGNED(8) data_in;
	uint64_t ATTRIBUTE_ALIGNED(8) data_out;
	uint32_t repeat;
	uint32_t duration;
};

#define BPF_PROG_TEST_RUN_struct_size \
	sizeof(struct BPF_PROG_TEST_RUN_struct)
#define expected_BPF_PROG_TEST_RUN_struct_size 40

struct BPF_PROG_GET_NEXT_ID_struct {
	uint32_t start_id;
	uint32_t next_id;
	uint32_t open_flags;
};

#define BPF_PROG_GET_NEXT_ID_struct_size \
	sizeof(struct BPF_PROG_GET_NEXT_ID_struct)
#define expected_BPF_PROG_GET_NEXT_ID_struct_size 12

#define BPF_MAP_GET_NEXT_ID_struct BPF_PROG_GET_NEXT_ID_struct
#define BPF_MAP_GET_NEXT_ID_struct_size BPF_PROG_GET_NEXT_ID_struct_size

struct BPF_PROG_GET_FD_BY_ID_struct {
	uint32_t prog_id;
	uint32_t next_id;
	uint32_t open_flags;
};

#define BPF_PROG_GET_FD_BY_ID_struct_size \
	sizeof(struct BPF_PROG_GET_FD_BY_ID_struct)
#define expected_BPF_PROG_GET_FD_BY_ID_struct_size 12

struct BPF_MAP_GET_FD_BY_ID_struct {
	uint32_t map_id;
	uint32_t next_id;
	uint32_t open_flags;
};

#define BPF_MAP_GET_FD_BY_ID_struct_size \
	sizeof(struct BPF_MAP_GET_FD_BY_ID_struct)
#define expected_BPF_MAP_GET_FD_BY_ID_struct_size 12

struct BPF_OBJ_GET_INFO_BY_FD_struct /* info */ {
	uint32_t bpf_fd;
	uint32_t info_len;
	uint64_t ATTRIBUTE_ALIGNED(8) info;
};

#define BPF_OBJ_GET_INFO_BY_FD_struct_size \
	sizeof(struct BPF_OBJ_GET_INFO_BY_FD_struct)
#define expected_BPF_OBJ_GET_INFO_BY_FD_struct_size 16

struct BPF_PROG_QUERY_struct /* query */ {
	uint32_t target_fd;
	uint32_t attach_type;
	uint32_t query_flags;
	uint32_t attach_flags;
	uint64_t ATTRIBUTE_ALIGNED(8) prog_ids;
	uint32_t prog_cnt;
};

#define BPF_PROG_QUERY_struct_size \
	offsetofend(struct BPF_PROG_QUERY_struct, prog_cnt)
#define expected_BPF_PROG_QUERY_struct_size 28

struct BPF_RAW_TRACEPOINT_OPEN_struct /* raw_tracepoint */ {
	uint64_t ATTRIBUTE_ALIGNED(8) name;
	uint32_t prog_fd;
};

#define BPF_RAW_TRACEPOINT_OPEN_struct_size \
	offsetofend(struct BPF_RAW_TRACEPOINT_OPEN_struct, prog_fd)
#define expected_BPF_RAW_TRACEPOINT_OPEN_struct_size 12

struct bpf_map_info_struct {
	uint32_t type;
	uint32_t id;
	uint32_t key_size;
	uint32_t value_size;
	uint32_t max_entries;
	uint32_t map_flags;
	char     name[BPF_OBJ_NAME_LEN];
	uint32_t ifindex;
	/*
	 * The kernel UAPI is broken by Linux commit
	 * v4.16-rc1~123^2~109^2~5^2~4 .
	 */
	uint64_t ATTRIBUTE_ALIGNED(8) netns_dev; /* skip check */
	uint64_t ATTRIBUTE_ALIGNED(8) netns_ino; /* skip check */
};

#define bpf_map_info_struct_size \
	sizeof(struct bpf_map_info_struct)
#define expected_bpf_map_info_struct_size 64

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
	/*
	 * The kernel UAPI is broken by Linux commit
	 * v4.16-rc1~123^2~227^2~5^2~2 .
	 */
	uint64_t ATTRIBUTE_ALIGNED(8) netns_dev; /* skip check */
	uint64_t ATTRIBUTE_ALIGNED(8) netns_ino; /* skip check */
};

#define bpf_prog_info_struct_size \
	sizeof(struct bpf_prog_info_struct)
#define expected_bpf_prog_info_struct_size 104

#endif /* !STRACE_BPF_ATTR_H */
