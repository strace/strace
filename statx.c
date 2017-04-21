/*
 * Copyright (c) 2017 The strace developers.
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
#include "statx.h"

#include <sys/stat.h>

#include "xlat/statx_masks.h"
#include "xlat/statx_attrs.h"
#include "xlat/at_statx_sync_types.h"

SYS_FUNC(statx)
{
	if (entering(tcp)) {
		print_dirfd(tcp, tcp->u_arg[0]);
		printpath(tcp, tcp->u_arg[1]);
		tprints(", ");

		unsigned int flags = tcp->u_arg[2];
		printflags(at_statx_sync_types, flags & AT_STATX_SYNC_TYPE,
			   NULL);
		flags &= ~AT_STATX_SYNC_TYPE;
		if (flags) {
			tprints("|");
			printflags(at_flags, flags, NULL);
		}

		tprints(", ");
		printflags(statx_masks, tcp->u_arg[3], "STATX_???");
		tprints(", ");
	} else {
#define PRINT_FIELD_U(field) \
	tprintf(", %s=%llu", #field, (unsigned long long) stx.field)

#define PRINT_FIELD_TIME(field)						\
	do {								\
		tprintf(", " #field "={tv_sec=%" PRId64			\
			", tv_nsec=%" PRIu32 "}",			\
			stx.field.sec, stx.field.nsec);			\
		tprints_comment(sprinttime_nsec(stx.field.sec,		\
			zero_extend_signed_to_ull(stx.field.nsec)));	\
	} while (0)

		struct_statx stx;
		if (umove_or_printaddr(tcp, tcp->u_arg[4], &stx))
			return 0;

		tprints("{stx_mask=");
		printflags(statx_masks, stx.stx_mask, "STATX_???");

		if (!abbrev(tcp))
			PRINT_FIELD_U(stx_blksize);

		tprints(", stx_attributes=");
		printflags(statx_attrs, stx.stx_attributes, "STATX_ATTR_???");

		if (!abbrev(tcp)) {
			PRINT_FIELD_U(stx_nlink);
			printuid(", stx_uid=", stx.stx_uid);
			printuid(", stx_gid=", stx.stx_gid);
		}

		tprints(", stx_mode=");
		print_symbolic_mode_t(stx.stx_mode);

		if (!abbrev(tcp))
			PRINT_FIELD_U(stx_ino);

		PRINT_FIELD_U(stx_size);

		if (!abbrev(tcp)) {
			PRINT_FIELD_U(stx_blocks);

			tprints(", stx_attributes_mask=");
			printflags(statx_attrs, stx.stx_attributes_mask,
				   "STATX_ATTR_???");

			PRINT_FIELD_TIME(stx_atime);
			PRINT_FIELD_TIME(stx_btime);
			PRINT_FIELD_TIME(stx_ctime);
			PRINT_FIELD_TIME(stx_mtime);
			PRINT_FIELD_U(stx_rdev_major);
			PRINT_FIELD_U(stx_rdev_minor);
			PRINT_FIELD_U(stx_dev_major);
			PRINT_FIELD_U(stx_dev_minor);
		} else {
			tprints(", ...");
		}
		tprints("}");
	}
	return 0;
}
