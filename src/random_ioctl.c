/*
 * Copyright (c) 2018-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

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
		tprint_arg_next();
		printnum_int(tcp, arg, "%d");
		break;

	case RNDADDENTROPY:
		tprint_arg_next();
		if (!umove_or_printaddr(tcp, arg, &info)) {
			tprint_struct_begin();
			PRINT_FIELD_D(info, entropy_count);
			tprint_struct_next();
			PRINT_FIELD_D(info, buf_size);
			tprint_struct_next();
			tprints_field_name("buf");
			buf = arg + offsetof(struct rand_pool_info, buf);
			printstrn(tcp, buf, info.buf_size);
			tprint_struct_end();
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
