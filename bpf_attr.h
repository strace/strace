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

#ifndef BPF_OBJ_NAME_LEN
# define BPF_OBJ_NAME_LEN 16U
#else
# if BPF_OBJ_NAME_LEN != 16U
#  error "Unexpected value of BPF_OBJ_NAME_LEN"
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
	uint64_t flags;
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
};

#define BPF_PROG_LOAD_struct_size \
	offsetofend(struct BPF_PROG_LOAD_struct, prog_ifindex)
#define expected_BPF_PROG_LOAD_struct_size 68

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

#endif /* !STRACE_BPF_ATTR_H */
