/*
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/ioctl.h>
#include <linux/pidfd.h>

#include "xlat/pidfd_coredump_mask.h"
#include "xlat/pidfd_info_mask.h"

CHECK_TYPE_SIZE(struct pidfd_info, PIDFD_INFO_SIZE_VER3);

static bool
pidfd_ioctl_is_namespace_fd(const unsigned int code)
{
	switch (code) {
		case PIDFD_GET_CGROUP_NAMESPACE:
		case PIDFD_GET_IPC_NAMESPACE:
		case PIDFD_GET_MNT_NAMESPACE:
		case PIDFD_GET_NET_NAMESPACE:
		case PIDFD_GET_PID_NAMESPACE:
		case PIDFD_GET_PID_FOR_CHILDREN_NAMESPACE:
		case PIDFD_GET_TIME_NAMESPACE:
		case PIDFD_GET_TIME_FOR_CHILDREN_NAMESPACE:
		case PIDFD_GET_USER_NAMESPACE:
		case PIDFD_GET_UTS_NAMESPACE:
			return true;
		default:
			return false;
	}
}

static bool
pidfd_ioctl_is_get_info(const unsigned int code)
{
	return _IOC_TYPE(code) == PIDFS_IOCTL_MAGIC
	       && _IOC_NR(code) == _IOC_NR(PIDFD_GET_INFO);
}

static void
pidfd_ioctl_print_pidfd_info_fields(struct tcb *tcp,
				    const struct pidfd_info *info,
				    const unsigned int usize)
{
	const unsigned int len = MIN(sizeof(*info), usize);
	const uint64_t m = info->mask;

	if ((m & PIDFD_INFO_CGROUPID)
	    && len >= offsetofend(struct pidfd_info, cgroupid)) {
		tprint_struct_next();
		PRINT_FIELD_U(*info, cgroupid);
	}

	if ((m & PIDFD_INFO_PID)
	    && len >= offsetofend(struct pidfd_info, ppid)) {
		tprint_struct_next();
		PRINT_FIELD_TID(*info, pid, tcp);
		tprint_struct_next();
		PRINT_FIELD_TGID(*info, tgid, tcp);
		tprint_struct_next();
		PRINT_FIELD_TGID(*info, ppid, tcp);
	}

	if ((m & PIDFD_INFO_CREDS)
	    && len >= offsetofend(struct pidfd_info, fsgid)) {
		tprint_struct_next();
		tprints_field_name("ruid");
		printuid(info->ruid);
		tprint_struct_next();
		tprints_field_name("rgid");
		printuid(info->rgid);
		tprint_struct_next();
		tprints_field_name("euid");
		printuid(info->euid);
		tprint_struct_next();
		tprints_field_name("egid");
		printuid(info->egid);
		tprint_struct_next();
		tprints_field_name("suid");
		printuid(info->suid);
		tprint_struct_next();
		tprints_field_name("sgid");
		printuid(info->sgid);
		tprint_struct_next();
		tprints_field_name("fsuid");
		printuid(info->fsuid);
		tprint_struct_next();
		tprints_field_name("fsgid");
		printuid(info->fsgid);
	}

	if ((m & PIDFD_INFO_EXIT)
	    && len >= offsetofend(struct pidfd_info, exit_code)) {
		tprint_struct_next();
		tprints_field_name("exit_code");
		print_wait_status(info->exit_code);
	}

	if ((m & PIDFD_INFO_COREDUMP)
	    && len >= offsetofend(struct pidfd_info, coredump_mask)) {
		tprint_struct_next();
		PRINT_FIELD_FLAGS(*info, coredump_mask,
				  pidfd_coredump_mask,
				  "PIDFD_COREDUMP_???");
	}

	if ((m & PIDFD_INFO_COREDUMP_SIGNAL)
	    && len >= offsetofend(struct pidfd_info, coredump_signal)) {
		tprint_struct_next();
		tprints_field_name("coredump_signal");
		printsignal(info->coredump_signal);
	}

	if ((m & PIDFD_INFO_COREDUMP_CODE)
	    && len >= offsetofend(struct pidfd_info, coredump_code)) {
		tprint_struct_next();
		tprints_field_name("coredump_code");
		PRINT_VAL_U(info->coredump_code);
	}

	if ((m & PIDFD_INFO_SUPPORTED_MASK)
	    && len >= offsetofend(struct pidfd_info, supported_mask)) {
		tprint_struct_next();
		PRINT_FIELD_FLAGS(*info, supported_mask, pidfd_info_mask,
				  "PIDFD_INFO_???");
	}
}

int
pidfd_ioctl(struct tcb *tcp, unsigned int code, kernel_ulong_t arg)
{
	if (pidfd_ioctl_is_namespace_fd(code)) {
		tprints_arg_next_name("argp");
		PRINT_VAL_X(arg);
		return RVAL_IOCTL_DECODED | RVAL_FD;
	}

	if (!pidfd_ioctl_is_get_info(code))
		return RVAL_DECODED;

	if (entering(tcp))
		tprints_arg_next_name("argp");

	const unsigned int usize = _IOC_SIZE(code);

	if (usize < PIDFD_INFO_SIZE_VER0) {
		printaddr(arg);
		return RVAL_IOCTL_DECODED;
	}

	struct pidfd_info info = { 0 };

	if (entering(tcp)) {
		if (umove_or_printaddr(tcp, arg, &info.mask))
			return RVAL_IOCTL_DECODED;

		if (sizeof(info.mask) > sizeof(long)) {
			typeof(info.mask) *req_mask_p = xmalloc(sizeof(*req_mask_p));
			*req_mask_p = info.mask;
			set_tcb_priv_data(tcp, req_mask_p, free);
		} else {
			set_tcb_priv_ulong(tcp, (unsigned long) info.mask);
		}

		tprint_struct_begin();
		tprints_field_name("mask");
		printflags64(pidfd_info_mask, info.mask, "PIDFD_INFO_???");

		return 0;
	}

	uint64_t req_mask;

	if (sizeof(info.mask) > sizeof(long)) {
		typeof(info.mask) *req_mask_p = get_tcb_priv_data(tcp);
		if (!req_mask_p) {
			tprint_struct_end();
			return RVAL_IOCTL_DECODED;
		}
		req_mask = *req_mask_p;
	} else {
		req_mask = get_tcb_priv_ulong(tcp);
	}

	const unsigned int len = MIN(sizeof(info), usize);
	if (tfetch_mem(tcp, arg, len, &info)) {
		if (info.mask != req_mask) {
			tprint_value_changed();
			printflags64(pidfd_info_mask, info.mask,
				     "PIDFD_INFO_???");
		}
		pidfd_ioctl_print_pidfd_info_fields(tcp, &info, usize);
	}

	tprint_struct_end();

	return RVAL_IOCTL_DECODED;
}
