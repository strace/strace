/*
 * Copyright (c) 2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "print_fields.h"

#include <linux/types.h>
#include <linux/random.h>

#define XLAT_MACROS_ONLY
#include "xlat/random_ioctl_cmds.h"
#undef XLAT_MACROS_ONLY

/*
 * RNDGETPOOL was removed in 2.6.9, so non-ancient kernels always
 * return -EINVAL for that.
 */

int
random_ioctl(struct tcb *const tcp, const unsigned int code,
	   const kernel_ulong_t arg)
{
	struct rand_pool_info info;
	kernel_ulong_t buf;

	switch (code) {
	case RNDGETENTCNT:
		if (entering(tcp))
			return 0;
		ATTRIBUTE_FALLTHROUGH;
	case RNDADDTOENTCNT:
		tprints(", ");
		printnum_int(tcp, arg, "%d");
		break;

	case RNDADDENTROPY:
		tprints(", ");
		if (!umove_or_printaddr(tcp, arg, &info)) {
			PRINT_FIELD_D("{", info, entropy_count);
			PRINT_FIELD_D(", ", info, buf_size);
			tprints(", buf=");
			buf = arg + offsetof(struct rand_pool_info, buf);
			printstrn(tcp, buf, info.buf_size);
			tprints("}");
		}
		break;

	/* ioctls with no parameters */
	case RNDZAPENTCNT:
	case RNDCLEARPOOL:
	case RNDRESEEDCRNG:
		break;
	default:
		return RVAL_DECODED;
	}
	return RVAL_IOCTL_DECODED;
}
