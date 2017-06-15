/*
 * The request_msgs are purposed to set the general mode of work, in
 * particular, the work of main dispatchers.
 *
 * Copyright (c) 2017 Edgar A. Kaziakhmedov <edgar.kaziakhmedov@virtuozzo.com>
 * Copyright (c) 2017-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef ASINFO_REQUEST_MSGS_H
#define ASINFO_REQUEST_MSGS_H

/* Request types for syscall_dispatcher,
 * arch_dispatcher, which, in turn, could be combined
 */
enum syscall_req_bit {
	SD_REQ_GET_SNAME_BIT,
	SD_REQ_GET_SNUM_BIT,
	SD_REQ_NARGS_BIT,

	SYSCALL_REQ_BIT_LAST
};

enum arch_req_bit {
	AD_REQ_SET_ARCH_BIT = SYSCALL_REQ_BIT_LAST,
	AD_REQ_GET_ARCH_BIT,
	AD_REQ_LIST_ARCH_BIT,

	ARCH_REQ_BIT_LAST
};

enum abi_req_bit {
	ABD_REQ_SET_ABI_BIT = ARCH_REQ_BIT_LAST,
	ABD_REQ_LIST_ABI_BIT,

	ABD_REQ_BIT_LAST
};

enum serv_req_bit {
	SERV_REQ_HELP_BIT = ABD_REQ_BIT_LAST,
	SERV_REQ_VERSION_BIT,
	SERV_REQ_ERROR_BIT,
	SERV_REQ_RAW_BIT,

	SERV_REQ_BIT_LAST
};

#define ENUM_FLAG(name) name = 1 << name##_BIT
enum req_type {
	ENUM_FLAG(SD_REQ_GET_SNAME),
	ENUM_FLAG(SD_REQ_GET_SNUM),
	ENUM_FLAG(SD_REQ_NARGS),
	ENUM_FLAG(AD_REQ_SET_ARCH),
	ENUM_FLAG(AD_REQ_GET_ARCH),
	ENUM_FLAG(AD_REQ_LIST_ARCH),
	ENUM_FLAG(ABD_REQ_SET_ABI),
	ENUM_FLAG(ABD_REQ_LIST_ABI),
	ENUM_FLAG(SERV_REQ_HELP),
	ENUM_FLAG(SERV_REQ_VERSION),
	ENUM_FLAG(SERV_REQ_ERROR),
	ENUM_FLAG(SERV_REQ_RAW)
};
#undef ENUM_FLAG

#define BITMASK(hi, lo) ((1 << (hi)) - (1 << (lo)))
#define REQ_LAST_BIT SERV_REQ_BIT_LAST
#define SD_REQ_MASK BITMASK(SYSCALL_REQ_BIT_LAST, 0)
#define AD_REQ_MASK BITMASK(ARCH_REQ_BIT_LAST, SYSCALL_REQ_BIT_LAST)
#define ABD_REQ_MASK BITMASK(ABD_REQ_BIT_LAST, ARCH_REQ_BIT_LAST)
#define SERV_REQ_MASK BITMASK(SERV_REQ_BIT_LAST, ABD_REQ_BIT_LAST)

#endif /* !ASINFO_REQUEST_MSGS_H */
