/*
 * Copyright (c) 2021 Eugene Syromyatnikov <evgsyr@gmail.com>.
 * Copyright (c) 2021-2022 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include <linux/ioctl.h>
#include <linux/seccomp.h>

#include "xlat/seccomp_ioctl_addfd_flags.h"
#include "xlat/seccomp_ioctl_resp_flags.h"

#define SECCOMP_IOCTL_NOTIF_ID_VALID_WRONG_DIR SECCOMP_IOR(2, __u64)

static void
print_struct_seccomp_data(struct tcb *const tcp,
			  const struct seccomp_data *const data)
{
	tprint_struct_begin();
	PRINT_FIELD_SYSCALL_NAME(*data, nr, data->arch);
	tprint_struct_next();
	PRINT_FIELD_XVAL(*data, arch, audit_arch, "AUDIT_ARCH_???");
	tprint_struct_next();
	PRINT_FIELD_ADDR64(*data, instruction_pointer);
	tprint_struct_next();
	PRINT_FIELD_ARRAY(*data, args, tcp, print_xint_array_member);
	tprint_struct_end();
}

static int
print_struct_seccomp_notif(struct tcb *const tcp, const kernel_ulong_t addr)
{
	/*
	 * NB: There is a (poorly designed) seccomp(SECCOMP_GET_NOTIF_SIZES)
	 *     operation that hints that struct seccomp_notif/seccomp_notif_data
	 *     may vary, but that in turn will change the ID of the respective
	 *     ioctl, rendering the whole idea of getting the struct size
	 *     in advance dubious at best.  Let's just put some safeguards
	 *     in place for the time being, in case updated headers bring
	 *     changes in either ioctl code or struct size.
	 */
	CHECK_IOCTL_SIZE(SECCOMP_IOCTL_NOTIF_RECV, 80);
	CHECK_TYPE_SIZE(struct seccomp_notif, 80);
	struct seccomp_notif notif;

	if (umove_or_printaddr(tcp, addr, &notif))
		return RVAL_IOCTL_DECODED;

	if (entering(tcp)) {
		if (is_filled((const char *) &notif, 0, sizeof(notif)))
			return 0;
	}

	tprint_struct_begin();
	PRINT_FIELD_X(notif, id);
	tprint_struct_next();
	PRINT_FIELD_TGID(notif, pid, tcp);
	tprint_struct_next();
	PRINT_FIELD_X(notif, flags);
	tprint_struct_next();
	PRINT_FIELD_OBJ_TCB_PTR(notif, data, tcp, print_struct_seccomp_data);
	tprint_struct_end();

	if (entering(tcp)) {
		tprint_value_changed();
		return 0;
	}

	return RVAL_IOCTL_DECODED;
}

static void
print_struct_seccomp_notif_resp(struct tcb *const tcp,
				const kernel_ulong_t addr)
{
	CHECK_IOCTL_SIZE(SECCOMP_IOCTL_NOTIF_SEND, 24);
	CHECK_TYPE_SIZE(struct seccomp_notif_resp, 24);
	struct seccomp_notif_resp resp;

	if (umove_or_printaddr(tcp, addr, &resp))
		return;

	tprint_struct_begin();
	PRINT_FIELD_X(resp, id);
	tprint_struct_next();
	PRINT_FIELD_D(resp, val);
	tprint_struct_next();
	PRINT_FIELD_ERR_D(resp, error);
	tprint_struct_next();
	PRINT_FIELD_FLAGS(resp, flags, seccomp_ioctl_resp_flags,
			  "SECCOMP_USER_NOTIF_FLAG_???");
	tprint_struct_end();
}

static void
print_struct_seccomp_notif_addfd(struct tcb *const tcp,
				 const kernel_ulong_t addr)
{
	CHECK_IOCTL_SIZE(SECCOMP_IOCTL_NOTIF_ADDFD, 24);
	CHECK_TYPE_SIZE(struct seccomp_notif_addfd, 24);
	struct seccomp_notif_addfd addfd;

	if (umove_or_printaddr(tcp, addr, &addfd))
		return;

	tprint_struct_begin();
	PRINT_FIELD_X(addfd, id);
	tprint_struct_next();
	PRINT_FIELD_FLAGS(addfd, flags, seccomp_ioctl_addfd_flags,
			  "SECCOMP_ADDFD_FLAG_???");
	tprint_struct_next();
	PRINT_FIELD_FD(addfd, srcfd, tcp);
	tprint_struct_next();
	PRINT_FIELD_D(addfd, newfd);
	tprint_struct_next();
	PRINT_FIELD_FLAGS(addfd, newfd_flags, open_mode_flags, "O_???");
	tprint_struct_end();
}

int
seccomp_ioctl(struct tcb *const tcp, const unsigned int code,
	     const kernel_ulong_t arg)
{
	switch (code) {
	case SECCOMP_IOCTL_NOTIF_RECV:
		if (entering(tcp))
			tprint_arg_next();

		return print_struct_seccomp_notif(tcp, arg);

	case SECCOMP_IOCTL_NOTIF_SEND:
		tprint_arg_next();
		print_struct_seccomp_notif_resp(tcp, arg);

		return RVAL_IOCTL_DECODED;

	case SECCOMP_IOCTL_NOTIF_ID_VALID_WRONG_DIR:
	case SECCOMP_IOCTL_NOTIF_ID_VALID:
		tprint_arg_next();
		printnum_int64(tcp, arg, "%#" PRIx64);

		return RVAL_IOCTL_DECODED;

	case SECCOMP_IOCTL_NOTIF_ADDFD:
		tprint_arg_next();
		print_struct_seccomp_notif_addfd(tcp, arg);

		return RVAL_IOCTL_DECODED;

	default:
		return RVAL_DECODED;
	}
}
