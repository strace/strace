/*
 * Copyright (c) 2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_TYPES_TEE_H
# define STRACE_TYPES_TEE_H

# include <linux/ioctl.h>

# ifdef HAVE_LINUX_TEE_H
#  include <linux/tee.h>
# else
#  define TEE_IOCTL_UUID_LEN 16
# endif

typedef struct {
	uint64_t buf_ptr;
	uint64_t buf_len;
} struct_tee_ioctl_buf_data;

typedef struct {
	uint32_t cancel_id;
	uint32_t session;
} struct_tee_ioctl_cancel_arg;

typedef struct {
	uint32_t session;
} struct_tee_ioctl_close_session_arg;

typedef struct {
	uint64_t size;
	uint32_t flags;
	int32_t id;
} struct_tee_ioctl_shm_alloc_data;

/* Not in mainline */
typedef struct {
	int64_t fd;
	uint64_t size;
	uint32_t flags;
	uint8_t _pad1[4];
	int32_t id;
	uint8_t _pad2[4];
} ATTRIBUTE_ALIGNED(8) struct_tee_ioctl_shm_register_fd_data;

typedef struct {
	uint64_t addr;
	uint64_t length;
	uint32_t flags;
	int32_t id;
} struct_tee_ioctl_shm_register_data;

typedef struct {
	uint32_t impl_id;
	uint32_t impl_caps;
	uint32_t gen_caps;
} struct_tee_ioctl_version_data;

typedef struct {
	uint64_t attr;
	uint64_t a;
	uint64_t b;
	uint64_t c;
} struct_tee_ioctl_param;

typedef struct {
	uint32_t func;
	uint32_t session;
	uint32_t cancel_id;
	uint32_t ret;
	uint32_t ret_origin;
	uint32_t num_params;
	/* num_params tells the actual number of element in params */
	struct_tee_ioctl_param params[];
} struct_tee_ioctl_invoke_arg;

typedef struct {
	uint32_t func;
	uint32_t num_params;
	/* num_params tells the actual number of element in params */
	struct_tee_ioctl_param params[];
} struct_tee_iocl_supp_recv_arg;

typedef struct {
	uint32_t ret;
	uint32_t num_params;
	/* num_params tells the actual number of element in params */
	struct_tee_ioctl_param params[];
} struct_tee_iocl_supp_send_arg;

typedef struct {
	uint8_t uuid[TEE_IOCTL_UUID_LEN];
	uint8_t clnt_uuid[TEE_IOCTL_UUID_LEN];
	uint32_t clnt_login;
	uint32_t cancel_id;
	uint32_t session;
	uint32_t ret;
	uint32_t ret_origin;
	uint32_t num_params;
	/* num_params tells the actual number of element in params */
	struct_tee_ioctl_param params[];
} struct_tee_ioctl_open_session_arg;

#endif /* STRACE_TYPES_TEE_H */
