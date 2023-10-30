/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "kernel_fcntl.h"
#include <linux/ioctl.h>
#include <linux/userfaultfd.h>

#include "xlat/uffd_flags.h"

SYS_FUNC(userfaultfd)
{
	printflags(uffd_flags, tcp->u_arg[0], "UFFD_???");

	return RVAL_DECODED | RVAL_FD;
}


#include "xlat/uffd_api_features.h"
#include "xlat/uffd_api_flags.h"
#include "xlat/uffd_continue_mode_flags.h"
#include "xlat/uffd_copy_flags.h"
#include "xlat/uffd_poison_mode_flags.h"
#include "xlat/uffd_register_ioctl_flags.h"
#include "xlat/uffd_register_mode_flags.h"
#include "xlat/uffd_writeprotect_mode_flags.h"
#include "xlat/uffd_zeropage_flags.h"

static void
tprintf_uffdio_range(const struct uffdio_range *range)
{
	tprint_struct_begin();
	PRINT_FIELD_X(*range, start);
	tprint_struct_next();
	PRINT_FIELD_X(*range, len);
	tprint_struct_end();
}

int
uffdio_ioctl(struct tcb *const tcp, const unsigned int code,
	     const kernel_ulong_t arg)
{
	switch (code) {
	case UFFDIO_API: {
		uint64_t *entering_features;
		struct uffdio_api ua;

		if (entering(tcp)) {
			tprint_arg_next();
			if (umove_or_printaddr(tcp, arg, &ua))
				break;
			tprint_struct_begin();
			PRINT_FIELD_X(ua, api);
			tprint_struct_next();
			PRINT_FIELD_FLAGS(ua, features, uffd_api_features,
					  "UFFD_FEATURE_???");
			entering_features = malloc(sizeof(*entering_features));
			if (entering_features) {
				*entering_features = ua.features;
				set_tcb_priv_data(tcp, entering_features, free);
			}

			return 0;
		}

		if (!syserror(tcp) && !umove(tcp, arg, &ua)) {
			entering_features = get_tcb_priv_data(tcp);

			if (!entering_features
			    || *entering_features != ua.features) {
				tprint_value_changed();
				PRINT_FIELD_FLAGS(ua, features,
						  uffd_api_features,
						  "UFFD_FEATURE_???");
			}

			tprint_struct_next();
			PRINT_FIELD_FLAGS(ua, ioctls, uffd_api_flags,
					  "_UFFDIO_???");
		}

		tprint_struct_end();

		break;
	}

	case UFFDIO_COPY: {
		struct uffdio_copy uc;

		if (entering(tcp)) {
			tprint_arg_next();
			if (umove_or_printaddr(tcp, arg, &uc))
				return RVAL_IOCTL_DECODED;
			tprint_struct_begin();
			PRINT_FIELD_X(uc, dst);
			tprint_struct_next();
			PRINT_FIELD_X(uc, src);
			tprint_struct_next();
			PRINT_FIELD_X(uc, len);
			tprint_struct_next();
			PRINT_FIELD_FLAGS(uc, mode, uffd_copy_flags,
					  "UFFDIO_COPY_???");

			return 0;
		}

		if (!syserror(tcp) && !umove(tcp, arg, &uc)) {
			tprint_struct_next();
			PRINT_FIELD_X(uc, copy);
		}

		tprint_struct_end();

		break;
	}

	case UFFDIO_REGISTER: {
		struct uffdio_register ur;

		if (entering(tcp)) {
			tprint_arg_next();
			if (umove_or_printaddr(tcp, arg, &ur))
				return RVAL_IOCTL_DECODED;
			tprint_struct_begin();
			PRINT_FIELD_OBJ_PTR(ur, range,
					    tprintf_uffdio_range);
			tprint_struct_next();
			PRINT_FIELD_FLAGS(ur, mode,
					  uffd_register_mode_flags,
					  "UFFDIO_REGISTER_MODE_???");

			return 0;
		}

		if (!syserror(tcp) && !umove(tcp, arg, &ur)) {
			tprint_struct_next();
			PRINT_FIELD_FLAGS(ur, ioctls,
					  uffd_register_ioctl_flags,
					  "UFFDIO_???");
		}

		tprint_struct_end();

		break;
	}

	case UFFDIO_UNREGISTER:
	case UFFDIO_WAKE: {
		struct uffdio_range ura;

		tprint_arg_next();
		if (!umove_or_printaddr(tcp, arg, &ura))
			tprintf_uffdio_range(&ura);

		break;
	}

	case UFFDIO_ZEROPAGE: {
		struct uffdio_zeropage uz;

		if (entering(tcp)) {
			tprint_arg_next();
			if (umove_or_printaddr(tcp, arg, &uz))
				return RVAL_IOCTL_DECODED;
			tprint_struct_begin();
			PRINT_FIELD_OBJ_PTR(uz, range,
					    tprintf_uffdio_range);
			tprint_struct_next();
			PRINT_FIELD_FLAGS(uz, mode, uffd_zeropage_flags,
					  "UFFDIO_ZEROPAGE_???");

			return 0;
		}

		if (!syserror(tcp) && !umove(tcp, arg, &uz)) {
			tprint_struct_next();
			PRINT_FIELD_X(uz, zeropage);
		}

		tprint_struct_end();

		break;
	}

	case UFFDIO_WRITEPROTECT: {
		struct uffdio_writeprotect uwp;

		tprint_arg_next();
		if (!umove_or_printaddr(tcp, arg, &uwp)) {
			tprint_struct_begin();
			PRINT_FIELD_OBJ_PTR(uwp, range,
					    tprintf_uffdio_range);
			tprint_struct_next();
			PRINT_FIELD_FLAGS(uwp, mode,
					  uffd_writeprotect_mode_flags,
					  "UFFDIO_WRITEPROTECT_MODE_WP???");
			tprint_struct_end();
		}

		break;
	}

	case UFFDIO_CONTINUE: {
		struct uffdio_continue uc;

		if (entering(tcp)) {
			tprint_arg_next();
			if (umove_or_printaddr(tcp, arg, &uc))
				return RVAL_IOCTL_DECODED;
			tprint_struct_begin();
			PRINT_FIELD_OBJ_PTR(uc, range,
					    tprintf_uffdio_range);
			tprint_struct_next();
			PRINT_FIELD_FLAGS(uc, mode, uffd_continue_mode_flags,
					  "UFFDIO_CONTINUE_MODE_???");

			return 0;
		}

		if (!syserror(tcp) && !umove(tcp, arg, &uc)) {
			tprint_struct_next();
			PRINT_FIELD_U(uc, mapped);
		}

		tprint_struct_end();

		break;
	}

	case UFFDIO_POISON: {
		struct uffdio_poison up;

		if (entering(tcp)) {
			tprint_arg_next();
			if (umove_or_printaddr(tcp, arg, &up))
				return RVAL_IOCTL_DECODED;
			tprint_struct_begin();
			PRINT_FIELD_OBJ_PTR(up, range,
					    tprintf_uffdio_range);
			tprint_struct_next();
			PRINT_FIELD_FLAGS(up, mode, uffd_poison_mode_flags,
					  "UFFDIO_POISON_MODE_???");

			return 0;
		}

		if (!syserror(tcp) && !umove(tcp, arg, &up)) {
			tprint_struct_next();
			PRINT_FIELD_U(up, updated);
		}

		tprint_struct_end();

		break;
	}

	default:
		return RVAL_DECODED;
	}

	return RVAL_IOCTL_DECODED;
}
