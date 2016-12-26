/*
 * Copyright (c) 2009, 2010 Jeff Mahoney <jeffm@suse.com>
 * Copyright (c) 2011-2016 Dmitry V. Levin <ldv@altlinux.org>
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

#include MPERS_DEFS

#ifndef BLKPG
# define BLKPG      _IO(0x12,105)
#endif

/*
 * ioctl numbers <= 114 are present in Linux 2.4.  The following ones have been
 * added since then and headers containing them may not be available on every
 * system.
 */

#ifndef BLKTRACESETUP
# define BLKTRACESETUP _IOWR(0x12, 115, struct_blk_user_trace_setup)
#endif
#ifndef BLKTRACESTART
# define BLKTRACESTART _IO(0x12,116)
#endif
#ifndef BLKTRACESTOP
# define BLKTRACESTOP _IO(0x12,117)
#endif
#ifndef BLKTRACETEARDOWN
# define BLKTRACETEARDOWN _IO(0x12,118)
#endif
#ifndef BLKDISCARD
# define BLKDISCARD _IO(0x12,119)
#endif
#ifndef BLKIOMIN
# define BLKIOMIN _IO(0x12,120)
#endif
#ifndef BLKIOOPT
# define BLKIOOPT _IO(0x12,121)
#endif
#ifndef BLKALIGNOFF
# define BLKALIGNOFF _IO(0x12,122)
#endif
#ifndef BLKPBSZGET
# define BLKPBSZGET _IO(0x12,123)
#endif
#ifndef BLKDISCARDZEROES
# define BLKDISCARDZEROES _IO(0x12,124)
#endif
#ifndef BLKSECDISCARD
# define BLKSECDISCARD _IO(0x12,125)
#endif
#ifndef BLKROTATIONAL
# define BLKROTATIONAL _IO(0x12,126)
#endif
#ifndef BLKZEROOUT
# define BLKZEROOUT _IO(0x12,127)
#endif

#include "xlat/blkpg_ops.h"

static void
print_blkpg_req(struct tcb *tcp, const struct_blkpg_ioctl_arg *blkpg)
{
	struct_blkpg_partition p;

	tprints("{");
	printxval(blkpg_ops, blkpg->op, "BLKPG_???");

	tprintf(", flags=%d, datalen=%d, data=",
		blkpg->flags, blkpg->datalen);

	if (!umove_or_printaddr(tcp, ptr_to_kulong(blkpg->data), &p)) {
		tprintf("{start=%" PRId64 ", length=%" PRId64
			", pno=%d, devname=",
			p.start, p.length, p.pno);
		print_quoted_string(p.devname, sizeof(p.devname),
				    QUOTE_0_TERMINATED);
		tprints(", volname=");
		print_quoted_string(p.volname, sizeof(p.volname),
				    QUOTE_0_TERMINATED);
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
		/* fall through */
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

#ifdef HAVE_BLKGETSIZE64
	/* returns an uint64_t */
	case BLKGETSIZE64:
		if (entering(tcp))
			return 0;
		tprints(", ");
		printnum_int64(tcp, arg, "%" PRIu64);
		break;
#endif

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
			tprintf("{act_mask=%u, buf_size=%u, "
				"buf_nr=%u, start_lba=%" PRIu64 ", "
				"end_lba=%" PRIu64 ", pid=%u",
				(unsigned)buts.act_mask, buts.buf_size,
				buts.buf_nr, buts.start_lba,
				buts.end_lba, buts.pid);
			return 1;
		} else {
			struct_blk_user_trace_setup buts;

			if (!syserror(tcp) && !umove(tcp, arg, &buts)) {
				tprints(", name=");
				print_quoted_string(buts.name, sizeof(buts.name),
						    QUOTE_0_TERMINATED);
			}
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

	return RVAL_DECODED | 1;
}
