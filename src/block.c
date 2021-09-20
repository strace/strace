/*
 * Copyright (c) 2009, 2010 Jeff Mahoney <jeffm@suse.com>
 * Copyright (c) 2011-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2011-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include DEF_MPERS_TYPE(struct_blk_user_trace_setup)
#include DEF_MPERS_TYPE(struct_blkpg_ioctl_arg)
#include DEF_MPERS_TYPE(struct_blkpg_partition)

#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/blkpg.h>
#include <linux/blkzoned.h>
#include <linux/blktrace_api.h>

typedef struct blkpg_ioctl_arg struct_blkpg_ioctl_arg;
typedef struct blkpg_partition struct_blkpg_partition;
typedef struct blk_user_trace_setup struct_blk_user_trace_setup;

#include MPERS_DEFS

#include "xlat/blkpg_ops.h"

static void
print_blkpg_req(struct tcb *tcp, const struct_blkpg_ioctl_arg *blkpg)
{
	struct_blkpg_partition p;

	tprint_struct_begin();
	PRINT_FIELD_XVAL(*blkpg, op, blkpg_ops, "BLKPG_???");
	tprint_struct_next();
	PRINT_FIELD_D(*blkpg, flags);
	tprint_struct_next();
	PRINT_FIELD_D(*blkpg, datalen);

	tprint_struct_next();
	tprints_field_name("data");
	if (!umove_or_printaddr(tcp, ptr_to_kulong(blkpg->data), &p)) {
		tprint_struct_begin();
		PRINT_FIELD_D(p, start);
		tprint_struct_next();
		PRINT_FIELD_D(p, length);
		tprint_struct_next();
		PRINT_FIELD_D(p, pno);
		tprint_struct_next();
		PRINT_FIELD_CSTRING(p, devname);
		tprint_struct_next();
		PRINT_FIELD_CSTRING(p, volname);
		tprint_struct_end();
	}
	tprint_struct_end();
}

MPERS_PRINTER_DECL(int, block_ioctl, struct tcb *const tcp,
		   const unsigned int code, const kernel_ulong_t arg)
{
	switch (code) {
	/* take arg as a value, not as a pointer */
	case BLKRASET:
	case BLKFRASET:
		tprint_arg_next();
		PRINT_VAL_U(arg);
		break;

	/* return an unsigned short */
	case BLKSECTGET:
	case BLKROTATIONAL:
		if (entering(tcp))
			return 0;
		tprint_arg_next();
		printnum_short(tcp, arg, "%hu");
		break;

	/* return a signed int */
	case BLKROGET:
	case BLKBSZGET:
	case BLKSSZGET:
	case BLKALIGNOFF:
		if (entering(tcp))
			return 0;
		ATTRIBUTE_FALLTHROUGH;
	/* take a signed int */
	case BLKROSET:
	case BLKBSZSET:
		tprint_arg_next();
		printnum_int(tcp, arg, "%d");
		break;

	/* return an unsigned int */
	case BLKPBSZGET:
	case BLKIOMIN:
	case BLKIOOPT:
	case BLKDISCARDZEROES:
	case BLKGETZONESZ:
	case BLKGETNRZONES:
		if (entering(tcp))
			return 0;
		tprint_arg_next();
		printnum_int(tcp, arg, "%u");
		break;

	/* return a signed long */
	case BLKRAGET:
	case BLKFRAGET:
		if (entering(tcp))
			return 0;
		tprint_arg_next();
		printnum_slong(tcp, arg);
		break;

	/* returns an unsigned long */
	case BLKGETSIZE:
		if (entering(tcp))
			return 0;
		tprint_arg_next();
		printnum_ulong(tcp, arg);
		break;

	/* returns an uint64_t */
	case BLKGETSIZE64:
	case BLKGETDISKSEQ:
		if (entering(tcp))
			return 0;
		tprint_arg_next();
		printnum_int64(tcp, arg, "%" PRIu64);
		break;

	/* takes a pair of uint64_t */
	case BLKDISCARD:
	case BLKSECDISCARD:
	case BLKZEROOUT:
		tprint_arg_next();
		printpair_int64(tcp, arg, "%" PRIu64);
		break;

	/* More complex types */
	case BLKPG: {
		struct_blkpg_ioctl_arg blkpg;

		tprint_arg_next();
		if (!umove_or_printaddr(tcp, arg, &blkpg))
			print_blkpg_req(tcp, &blkpg);
		break;
	}

	case BLKTRACESETUP:
		if (entering(tcp)) {
			struct_blk_user_trace_setup buts;

			tprint_arg_next();
			if (umove_or_printaddr(tcp, arg, &buts))
				break;
			tprint_struct_begin();
			PRINT_FIELD_U(buts, act_mask);
			tprint_struct_next();
			PRINT_FIELD_U(buts, buf_size);
			tprint_struct_next();
			PRINT_FIELD_U(buts, buf_nr);
			tprint_struct_next();
			PRINT_FIELD_U(buts, start_lba);
			tprint_struct_next();
			PRINT_FIELD_U(buts, end_lba);
			tprint_struct_next();
			PRINT_FIELD_TGID(buts, pid, tcp);
			return 0;
		} else {
			struct_blk_user_trace_setup buts;

			if (!syserror(tcp) && !umove(tcp, arg, &buts)) {
				tprint_struct_next();
				PRINT_FIELD_CSTRING(buts, name);
			}
			tprint_struct_end();
			break;
		}

	/* No arguments */
	case BLKRRPART:
	case BLKFLSBUF:
	case BLKTRACESTART:
	case BLKTRACESTOP:
	case BLKTRACETEARDOWN:
		break;
	default:
		return RVAL_DECODED;
	}

	return RVAL_IOCTL_DECODED;
}
