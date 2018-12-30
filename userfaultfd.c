/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "print_fields.h"
#include <fcntl.h>

#include "xlat/uffd_flags.h"

SYS_FUNC(userfaultfd)
{
	printflags(uffd_flags, tcp->u_arg[0], "UFFD_???");

	return RVAL_DECODED | RVAL_FD;
}

#ifdef HAVE_LINUX_USERFAULTFD_H
# include <linux/ioctl.h>
# include <linux/userfaultfd.h>

# include "xlat/uffd_api_features.h"
# include "xlat/uffd_api_flags.h"
# include "xlat/uffd_copy_flags.h"
# include "xlat/uffd_register_ioctl_flags.h"
# include "xlat/uffd_register_mode_flags.h"
# include "xlat/uffd_zeropage_flags.h"

static void
tprintf_uffdio_range(const struct uffdio_range *range)
{
	PRINT_FIELD_X("{", *range, start);
	PRINT_FIELD_X(", ", *range, len);
	tprints("}");
}

# define PRINT_FIELD_UFFDIO_RANGE(prefix_, where_, field_)		\
	do {								\
		STRACE_PRINTF("%s%s=", (prefix_), #field_);		\
		tprintf_uffdio_range(&(where_).field_);			\
	} while (0)

int
uffdio_ioctl(struct tcb *const tcp, const unsigned int code,
	     const kernel_ulong_t arg)
{
	switch (code) {
	case UFFDIO_API: {
		uint64_t *entering_features;
		struct uffdio_api ua;

		if (entering(tcp)) {
			tprints(", ");
			if (umove_or_printaddr(tcp, arg, &ua))
				break;
			PRINT_FIELD_X("{", ua, api);
			PRINT_FIELD_FLAGS(", ", ua, features, uffd_api_features,
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
				PRINT_FIELD_FLAGS(" => ", ua, features,
						  uffd_api_features,
						  "UFFD_FEATURE_???");
			}

			PRINT_FIELD_FLAGS(", ", ua, ioctls, uffd_api_flags,
					  "_UFFDIO_???");
		}

		tprints("}");

		break;
	}

	case UFFDIO_COPY: {
		struct uffdio_copy uc;

		if (entering(tcp)) {
			tprints(", ");
			if (umove_or_printaddr(tcp, arg, &uc))
				return RVAL_IOCTL_DECODED;
			PRINT_FIELD_X("{", uc, dst);
			PRINT_FIELD_X(", ", uc, src);
			PRINT_FIELD_X(", ", uc, len);
			PRINT_FIELD_FLAGS(", ", uc, mode, uffd_copy_flags,
					  "UFFDIO_COPY_???");

			return 0;
		}

		if (!syserror(tcp) && !umove(tcp, arg, &uc))
			PRINT_FIELD_X(", ", uc, copy);

		tprints("}");

		break;
	}

	case UFFDIO_REGISTER: {
		struct uffdio_register ur;

		if (entering(tcp)) {
			tprints(", ");
			if (umove_or_printaddr(tcp, arg, &ur))
				return RVAL_IOCTL_DECODED;
			PRINT_FIELD_UFFDIO_RANGE("{", ur, range);
			PRINT_FIELD_FLAGS(", ", ur, mode,
					  uffd_register_mode_flags,
					  "UFFDIO_REGISTER_MODE_???");

			return 0;
		}

		if (!syserror(tcp) && !umove(tcp, arg, &ur)) {
			PRINT_FIELD_FLAGS(", ", ur, ioctls,
					  uffd_register_ioctl_flags,
					  "UFFDIO_???");
		}

		tprints("}");

		break;
	}

	case UFFDIO_UNREGISTER:
	case UFFDIO_WAKE: {
		struct uffdio_range ura;

		tprints(", ");

		if (!umove_or_printaddr(tcp, arg, &ura))
			tprintf_uffdio_range(&ura);

		break;
	}

	case UFFDIO_ZEROPAGE: {
		struct uffdio_zeropage uz;

		if (entering(tcp)) {
			tprints(", ");
			if (umove_or_printaddr(tcp, arg, &uz))
				return RVAL_IOCTL_DECODED;
			PRINT_FIELD_UFFDIO_RANGE("{", uz, range);
			PRINT_FIELD_FLAGS(", ", uz, mode, uffd_zeropage_flags,
					  "UFFDIO_ZEROPAGE_???");

			return 0;
		}

		if (!syserror(tcp) && !umove(tcp, arg, &uz))
			PRINT_FIELD_X(", ", uz, zeropage);

		tprints("}");

		break;
	}

	default:
		return RVAL_DECODED;
	}

	return RVAL_IOCTL_DECODED;
}
#endif /* HAVE_LINUX_USERFAULTFD_H */
