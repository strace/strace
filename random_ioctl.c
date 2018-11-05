/*
 * Copyright (c) 2018 The strace developers.
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

#include "defs.h"
#include "print_fields.h"

#include <linux/types.h>
#include <linux/random.h>

#define XLAT_MACROS_ONLY
# include "xlat/random_ioctl_cmds.h"
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
