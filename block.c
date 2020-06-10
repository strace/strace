/*
 * Copyright (c) 2009, 2010 Jeff Mahoney <jeffm@suse.com>
 * Copyright (c) 2011-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2011-2020 The strace developers.
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

typedef struct {
	int op;
	int flags;
	int datalen;
	void *data;
} struct_blkpg_ioctl_arg;

#define BLKPG_DEVNAMELTH	64
#define BLKPG_VOLNAMELTH	64
typedef struct {
	int64_t start;			/* starting offset in bytes */
	int64_t length;			/* length in bytes */
	int pno;			/* partition number */
	char devname[BLKPG_DEVNAMELTH];	/* partition name, like sda5 or c0d1p2,
					   to be used in kernel messages */
	char volname[BLKPG_VOLNAMELTH];	/* volume label */
} struct_blkpg_partition;

#define BLKTRACE_BDEV_SIZE      32
typedef struct blk_user_trace_setup {
	char name[BLKTRACE_BDEV_SIZE];	/* output */
	uint16_t act_mask;		/* input */
	uint32_t buf_size;		/* input */
	uint32_t buf_nr;		/* input */
	uint64_t start_lba;
	uint64_t end_lba;
	uint32_t pid;
} struct_blk_user_trace_setup;

/* Provide fall-back definitions for BLK* ioctls */
#define XLAT_MACROS_ONLY
#include "xlat/block_ioctl_cmds.h"
#undef XLAT_MACROS_ONLY

#include MPERS_DEFS

#include "print_fields.h"

#include "xlat/blkpg_ops.h"

static void
print_blkpg_req(struct tcb *tcp, const struct_blkpg_ioctl_arg *blkpg)
{
	struct_blkpg_partition p;

	PRINT_FIELD_XVAL("{", *blkpg, op, blkpg_ops, "BLKPG_???");
	PRINT_FIELD_D(", ", *blkpg, flags);
	PRINT_FIELD_D(", ", *blkpg, datalen);

	tprints(", data=");
	if (!umove_or_printaddr(tcp, ptr_to_kulong(blkpg->data), &p)) {
		PRINT_FIELD_D("{", p, start);
		PRINT_FIELD_D(", ", p, length);
		PRINT_FIELD_D(", ", p, pno);
		PRINT_FIELD_CSTRING(", ", p, devname);
		PRINT_FIELD_CSTRING(", ", p, volname);
		tprints("}");
	}
	tprints("}");
}

MPERS_PRINTER_DECL(int, block_ioctl, struct tcb *const tcp,
		   const unsigned int code, const kernel_ulong_t arg)
{
	switch (code) {
	/* take arg as a value, not as a pointer */
	case BLKRASET:
	case BLKFRASET:
		tprintf(", %" PRI_klu, arg);
		break;

	/* return an unsigned short */
	case BLKSECTGET:
	case BLKROTATIONAL:
		if (entering(tcp))
			return 0;
		tprints(", ");
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
		tprints(", ");
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
		tprints(", ");
		printnum_int(tcp, arg, "%u");
		break;

	/* return a signed long */
	case BLKRAGET:
	case BLKFRAGET:
		if (entering(tcp))
			return 0;
		tprints(", ");
		printnum_slong(tcp, arg);
		break;

	/* returns an unsigned long */
	case BLKGETSIZE:
		if (entering(tcp))
			return 0;
		tprints(", ");
		printnum_ulong(tcp, arg);
		break;

	/* returns an uint64_t */
	case BLKGETSIZE64:
		if (entering(tcp))
			return 0;
		tprints(", ");
		printnum_int64(tcp, arg, "%" PRIu64);
		break;

	/* takes a pair of uint64_t */
	case BLKDISCARD:
	case BLKSECDISCARD:
	case BLKZEROOUT:
		tprints(", ");
		printpair_int64(tcp, arg, "%" PRIu64);
		break;

	/* More complex types */
	case BLKPG: {
		struct_blkpg_ioctl_arg blkpg;

		tprints(", ");
		if (!umove_or_printaddr(tcp, arg, &blkpg))
			print_blkpg_req(tcp, &blkpg);
		break;
	}

	case BLKTRACESETUP:
		if (entering(tcp)) {
			struct_blk_user_trace_setup buts;

			tprints(", ");
			if (umove_or_printaddr(tcp, arg, &buts))
				break;
			PRINT_FIELD_U("{", buts, act_mask);
			PRINT_FIELD_U(", ", buts, buf_size);
			PRINT_FIELD_U(", ", buts, buf_nr);
			PRINT_FIELD_U(", ", buts, start_lba);
			PRINT_FIELD_U(", ", buts, end_lba);
			PRINT_FIELD_TGID(", ", buts, pid, tcp);
			return 0;
		} else {
			struct_blk_user_trace_setup buts;

			if (!syserror(tcp) && !umove(tcp, arg, &buts))
				PRINT_FIELD_CSTRING(", ", buts, name);
			tprints("}");
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
