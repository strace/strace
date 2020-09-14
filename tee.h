/*
 * Copyright (c) 2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <linux/ioctl.h>

#ifdef HAVE_LINUX_TEE_H
# include <linux/tee.h>
#else
# define TEE_IOCTL_UUID_LEN 16
#endif

#ifndef HAVE_STRUCT_TEE_IOCTL_BUF_DATA
struct tee_ioctl_buf_data {
	uint64_t buf_ptr;
	uint64_t buf_len;
};
#endif
#ifndef HAVE_STRUCT_TEE_IOCTL_CANCEL_ARG
struct tee_ioctl_cancel_arg {
	uint32_t cancel_id;
	uint32_t session;
};
#endif
#ifndef HAVE_STRUCT_TEE_IOCTL_CLOSE_SESSION_ARG
struct tee_ioctl_close_session_arg {
	uint32_t session;
};
#endif
#ifndef HAVE_STRUCT_TEE_IOCTL_SHM_ALLOC_DATA
struct tee_ioctl_shm_alloc_data {
	uint64_t size;
	uint32_t flags;
	int32_t id;
};
#endif
#ifndef HAVE_STRUCT_TEE_IOCTL_SHM_REGISTER_FD_DATA
struct tee_ioctl_shm_register_fd_data {
	int64_t fd;
	uint64_t size;
	uint32_t flags;
	uint8_t _pad1[4];
	int32_t id;
	uint8_t _pad2[4];
} ATTRIBUTE_ALIGNED(8);
#endif
#ifndef HAVE_STRUCT_TEE_IOCTL_SHM_REGISTER_DATA
struct tee_ioctl_shm_register_data {
	uint64_t addr;
	uint64_t length;
	uint32_t flags;
	int32_t id;
};
#endif
#ifndef HAVE_STRUCT_TEE_IOCTL_VERSION_DATA
struct tee_ioctl_version_data {
	uint32_t impl_id;
	uint32_t impl_caps;
	uint32_t gen_caps;
};
#endif
#ifndef HAVE_STRUCT_TEE_IOCTL_PARAM
struct tee_ioctl_param {
	uint64_t attr;
	uint64_t a;
	uint64_t b;
	uint64_t c;
};
#endif
#ifndef HAVE_STRUCT_TEE_IOCTL_INVOKE_ARG
struct tee_ioctl_invoke_arg {
	uint32_t func;
	uint32_t session;
	uint32_t cancel_id;
	uint32_t ret;
	uint32_t ret_origin;
	uint32_t num_params;
	/* num_params tells the actual number of element in params */
	struct tee_ioctl_param params[];

};
#endif
#ifndef HAVE_STRUCT_TEE_IOCL_SUPP_RECV_ARG
struct tee_iocl_supp_recv_arg {
	uint32_t func;
	uint32_t num_params;
	/* num_params tells the actual number of element in params */
	struct tee_ioctl_param params[];
};
#endif
#ifndef HAVE_STRUCT_TEE_IOCL_SUPP_SEND_ARG
struct tee_iocl_supp_send_arg {
	uint32_t ret;
	uint32_t num_params;
	/* num_params tells the actual number of element in params */
	struct tee_ioctl_param params[];
};
#endif
#ifndef HAVE_STRUCT_TEE_IOCTL_OPEN_SESSION_ARG
struct tee_ioctl_open_session_arg {
	uint8_t uuid[TEE_IOCTL_UUID_LEN];
	uint8_t clnt_uuid[TEE_IOCTL_UUID_LEN];
	uint32_t clnt_login;
	uint32_t cancel_id;
	uint32_t session;
	uint32_t ret;
	uint32_t ret_origin;
	uint32_t num_params;
	/* num_params tells the actual number of element in params */
	struct tee_ioctl_param params[];
};
#endif

#define XLAT_MACROS_ONLY
#include "xlat/tee_ioctl_cmds.h"
#undef XLAT_MACROS_ONLY

#include "xlat/tee_ioctl_gen_caps.h"
#include "xlat/tee_ioctl_impl_ids.h"
#include "xlat/tee_ioctl_login_types.h"
#include "xlat/tee_ioctl_max_arg_size.h"
#include "xlat/tee_ioctl_origins.h"
#include "xlat/tee_ioctl_optee_caps.h"
#include "xlat/tee_ioctl_param_attr_types.h"
#include "xlat/tee_ioctl_shm_flags.h"

#define TEE_IOCTL_PARAM_SIZE(x) (sizeof(struct tee_ioctl_param) * (x))
