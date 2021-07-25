/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2005-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2006-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/dqblk_xfs.h>

#define SUBCMDMASK  0x00ff
#define SUBCMDSHIFT 8
#define QCMD_CMD(cmd)	((uint32_t)(cmd) >> SUBCMDSHIFT)
#define QCMD_TYPE(cmd)	((uint32_t)(cmd) & SUBCMDMASK)

#define OLD_CMD(cmd)	((uint32_t)(cmd) << SUBCMDSHIFT)
#define NEW_CMD(cmd)	((uint32_t)(cmd) | 0x800000)

#include "xlat/quotacmds.h"
#include "xlat/quotatypes.h"
#include "xlat/quota_formats.h"
#include "xlat/xfs_quota_flags.h"
#include "xlat/xfs_dqblk_flags.h"
#include "xlat/if_dqblk_valid.h"
#include "xlat/if_dqinfo_flags.h"
#include "xlat/if_dqinfo_valid.h"

/*
 * We add attribute packed due to the fact that the structure is 8-byte aligned
 * on 64-bit systems and therefore has additional 4 bytes of padding, which
 * leads to problems when it is used on 32-bit tracee which does not have such
 * padding.
 */
struct if_dqblk {
	uint64_t dqb_bhardlimit;
	uint64_t dqb_bsoftlimit;
	uint64_t dqb_curspace;
	uint64_t dqb_ihardlimit;
	uint64_t dqb_isoftlimit;
	uint64_t dqb_curinodes;
	uint64_t dqb_btime;
	uint64_t dqb_itime;
	uint32_t dqb_valid;
} ATTRIBUTE_PACKED;

struct if_nextdqblk {
	uint64_t dqb_bhardlimit;
	uint64_t dqb_bsoftlimit;
	uint64_t dqb_curspace;
	uint64_t dqb_ihardlimit;
	uint64_t dqb_isoftlimit;
	uint64_t dqb_curinodes;
	uint64_t dqb_btime;
	uint64_t dqb_itime;
	uint32_t dqb_valid;
	uint32_t dqb_id;
};

struct if_dqinfo {
	uint64_t dqi_bgrace;
	uint64_t dqi_igrace;
	uint32_t dqi_flags;
	uint32_t dqi_valid;
};

static void
print_fs_qfilestat(const struct fs_qfilestat *const p)
{
	tprint_struct_begin();
	PRINT_FIELD_U(*p, qfs_ino);
	tprint_struct_next();
	PRINT_FIELD_U(*p, qfs_nblks);
	tprint_struct_next();
	PRINT_FIELD_U(*p, qfs_nextents);
	tprint_struct_end();
}

static void
print_fs_qfilestatv(const struct fs_qfilestatv *const p)
{
	tprint_struct_begin();
	PRINT_FIELD_U(*p, qfs_ino);
	tprint_struct_next();
	PRINT_FIELD_U(*p, qfs_nblks);
	tprint_struct_next();
	PRINT_FIELD_U(*p, qfs_nextents);
	tprint_struct_end();
}

static int
decode_cmd_data(struct tcb *tcp, uint32_t id, uint32_t cmd, kernel_ulong_t data)
{
	switch (cmd) {
	case Q_QUOTAOFF:
	case Q_SYNC:
	case Q_XQUOTASYNC:
		break;
	case Q_QUOTAON:
		tprint_arg_next();
		printxval(quota_formats, id, "QFMT_VFS_???");
		tprint_arg_next();
		printpath(tcp, data);
		break;
	case Q_GETQUOTA:
		if (entering(tcp)) {
			tprint_arg_next();
			printuid(id);
			tprint_arg_next();

			return 0;
		}

		ATTRIBUTE_FALLTHROUGH;
	case Q_SETQUOTA:
	{
		struct if_dqblk dq;

		if (entering(tcp)) {
			tprint_arg_next();
			printuid(id);
			tprint_arg_next();
		}

		if (umove_or_printaddr(tcp, data, &dq))
			break;
		tprint_struct_begin();
		PRINT_FIELD_U(dq, dqb_bhardlimit);
		tprint_struct_next();
		PRINT_FIELD_U(dq, dqb_bsoftlimit);
		tprint_struct_next();
		PRINT_FIELD_U(dq, dqb_curspace);
		tprint_struct_next();
		PRINT_FIELD_U(dq, dqb_ihardlimit);
		tprint_struct_next();
		PRINT_FIELD_U(dq, dqb_isoftlimit);
		tprint_struct_next();
		PRINT_FIELD_U(dq, dqb_curinodes);
		if (!abbrev(tcp)) {
			tprint_struct_next();
			PRINT_FIELD_U(dq, dqb_btime);
			tprint_struct_next();
			PRINT_FIELD_U(dq, dqb_itime);
			tprint_struct_next();
			PRINT_FIELD_FLAGS(dq, dqb_valid,
					  if_dqblk_valid, "QIF_???");
		} else {
			tprint_struct_next();
			tprint_more_data_follows();
		}
		tprint_struct_end();
		break;
	}
	case Q_GETNEXTQUOTA:
	{
		struct if_nextdqblk dq;

		if (entering(tcp)) {
			tprint_arg_next();
			printuid(id);
			tprint_arg_next();

			return 0;
		}

		if (umove_or_printaddr(tcp, data, &dq))
			break;
		tprint_struct_begin();
		PRINT_FIELD_U(dq, dqb_bhardlimit);
		tprint_struct_next();
		PRINT_FIELD_U(dq, dqb_bsoftlimit);
		tprint_struct_next();
		PRINT_FIELD_U(dq, dqb_curspace);
		tprint_struct_next();
		PRINT_FIELD_U(dq, dqb_ihardlimit);
		tprint_struct_next();
		PRINT_FIELD_U(dq, dqb_isoftlimit);
		tprint_struct_next();
		PRINT_FIELD_U(dq, dqb_curinodes);
		if (!abbrev(tcp)) {
			tprint_struct_next();
			PRINT_FIELD_U(dq, dqb_btime);
			tprint_struct_next();
			PRINT_FIELD_U(dq, dqb_itime);
			tprint_struct_next();
			PRINT_FIELD_FLAGS(dq, dqb_valid,
					  if_dqblk_valid, "QIF_???");
			tprint_struct_next();
			PRINT_FIELD_U(dq, dqb_id);
		} else {
			tprint_struct_next();
			PRINT_FIELD_U(dq, dqb_id);
			tprint_struct_next();
			tprint_more_data_follows();
		}
		tprint_struct_end();
		break;
	}
	case Q_XGETQUOTA:
	case Q_XGETNEXTQUOTA:
		if (entering(tcp)) {
			tprint_arg_next();
			printuid(id);
			tprint_arg_next();

			return 0;
		}

		ATTRIBUTE_FALLTHROUGH;
	case Q_XSETQLIM:
	{
		fs_disk_quota_t dq;

		if (entering(tcp)) {
			tprint_arg_next();
			printuid(id);
			tprint_arg_next();
		}

		if (umove_or_printaddr(tcp, data, &dq))
			break;
		tprint_struct_begin();
		PRINT_FIELD_D(dq, d_version);
		tprint_struct_next();
		PRINT_FIELD_FLAGS(dq, d_flags,
				  xfs_dqblk_flags, "FS_???_QUOTA");
		tprint_struct_next();
		PRINT_FIELD_X(dq, d_fieldmask);
		tprint_struct_next();
		PRINT_FIELD_U(dq, d_id);
		tprint_struct_next();
		PRINT_FIELD_U(dq, d_blk_hardlimit);
		tprint_struct_next();
		PRINT_FIELD_U(dq, d_blk_softlimit);
		tprint_struct_next();
		PRINT_FIELD_U(dq, d_ino_hardlimit);
		tprint_struct_next();
		PRINT_FIELD_U(dq, d_ino_softlimit);
		tprint_struct_next();
		PRINT_FIELD_U(dq, d_bcount);
		tprint_struct_next();
		PRINT_FIELD_U(dq, d_icount);
		if (!abbrev(tcp)) {
			tprint_struct_next();
			PRINT_FIELD_D(dq, d_itimer);
			tprint_struct_next();
			PRINT_FIELD_D(dq, d_btimer);
			tprint_struct_next();
			PRINT_FIELD_U(dq, d_iwarns);
			tprint_struct_next();
			PRINT_FIELD_U(dq, d_bwarns);
			tprint_struct_next();
			PRINT_FIELD_U(dq, d_rtb_hardlimit);
			tprint_struct_next();
			PRINT_FIELD_U(dq, d_rtb_softlimit);
			tprint_struct_next();
			PRINT_FIELD_U(dq, d_rtbcount);
			tprint_struct_next();
			PRINT_FIELD_D(dq, d_rtbtimer);
			tprint_struct_next();
			PRINT_FIELD_U(dq, d_rtbwarns);
		} else {
			tprint_struct_next();
			tprint_more_data_follows();
		}
		tprint_struct_end();
		break;
	}
	case Q_GETFMT:
	{
		uint32_t fmt;

		if (entering(tcp)) {
			tprint_arg_next();

			return 0;
		}

		if (umove_or_printaddr(tcp, data, &fmt))
			break;
		tprint_indirect_begin();
		printxval(quota_formats, fmt, "QFMT_VFS_???");
		tprint_indirect_end();
		break;
	}
	case Q_GETINFO:
		if (entering(tcp)) {
			tprint_arg_next();

			return 0;
		}

		ATTRIBUTE_FALLTHROUGH;
	case Q_SETINFO:
	{
		struct if_dqinfo dq;

		if (entering(tcp))
			tprint_arg_next();

		if (umove_or_printaddr(tcp, data, &dq))
			break;
		tprint_struct_begin();
		PRINT_FIELD_U(dq, dqi_bgrace);
		tprint_struct_next();
		PRINT_FIELD_U(dq, dqi_igrace);
		tprint_struct_next();
		PRINT_FIELD_FLAGS(dq, dqi_flags, if_dqinfo_flags, "DQF_???");
		tprint_struct_next();
		PRINT_FIELD_FLAGS(dq, dqi_valid, if_dqinfo_valid, "IIF_???");
		tprint_struct_end();
		break;
	}
	case Q_XGETQSTAT:
	{
		fs_quota_stat_t dq;

		if (entering(tcp)) {
			tprint_arg_next();

			return 0;
		}
		if (fetch_struct_quotastat(tcp, data, &dq)) {
			tprint_struct_begin();
			PRINT_FIELD_D(dq, qs_version);
			if (!abbrev(tcp)) {
				tprint_struct_next();
				PRINT_FIELD_FLAGS(dq, qs_flags,
						  xfs_quota_flags, "FS_QUOTA_???");
				tprint_struct_next();
				PRINT_FIELD_OBJ_PTR(dq, qs_uquota,
						    print_fs_qfilestat);
				tprint_struct_next();
				PRINT_FIELD_OBJ_PTR(dq, qs_gquota,
						    print_fs_qfilestat);
				tprint_struct_next();
				PRINT_FIELD_U(dq, qs_incoredqs);
				tprint_struct_next();
				PRINT_FIELD_D(dq, qs_btimelimit);
				tprint_struct_next();
				PRINT_FIELD_D(dq, qs_itimelimit);
				tprint_struct_next();
				PRINT_FIELD_D(dq, qs_rtbtimelimit);
				tprint_struct_next();
				PRINT_FIELD_U(dq, qs_bwarnlimit);
				tprint_struct_next();
				PRINT_FIELD_U(dq, qs_iwarnlimit);
			} else {
				tprint_struct_next();
				tprint_more_data_follows();
			}
			tprint_struct_end();
		}
		break;
	}
	case Q_XGETQSTATV:
	{
		struct fs_quota_statv dq;

		if (entering(tcp)) {
			tprint_arg_next();

			return 0;
		}

		if (umove_or_printaddr(tcp, data, &dq))
			break;
		tprint_struct_begin();
		PRINT_FIELD_D(dq, qs_version);
		if (!abbrev(tcp)) {
			tprint_struct_next();
			PRINT_FIELD_FLAGS(dq, qs_flags,
					  xfs_quota_flags, "FS_QUOTA_???");
			tprint_struct_next();
			PRINT_FIELD_U(dq, qs_incoredqs);
			tprint_struct_next();
			PRINT_FIELD_OBJ_PTR(dq, qs_uquota,
					    print_fs_qfilestatv);
			tprint_struct_next();
			PRINT_FIELD_OBJ_PTR(dq, qs_gquota,
					    print_fs_qfilestatv);
			tprint_struct_next();
			PRINT_FIELD_OBJ_PTR(dq, qs_pquota,
					    print_fs_qfilestatv);
			tprint_struct_next();
			PRINT_FIELD_D(dq, qs_btimelimit);
			tprint_struct_next();
			PRINT_FIELD_D(dq, qs_itimelimit);
			tprint_struct_next();
			PRINT_FIELD_D(dq, qs_rtbtimelimit);
			tprint_struct_next();
			PRINT_FIELD_U(dq, qs_bwarnlimit);
			tprint_struct_next();
			PRINT_FIELD_U(dq, qs_iwarnlimit);
		} else {
			tprint_struct_next();
			tprint_more_data_follows();
		}
		tprint_struct_end();
		break;
	}
	case Q_XQUOTAON:
	case Q_XQUOTAOFF:
	{
		uint32_t flag;

		tprint_arg_next();

		if (umove_or_printaddr(tcp, data, &flag))
			break;
		tprint_indirect_begin();
		printflags(xfs_quota_flags, flag, "FS_QUOTA_???");
		tprint_indirect_end();
		break;
	}
	case Q_XQUOTARM:
	{
		uint32_t flag;

		tprint_arg_next();

		if (umove_or_printaddr(tcp, data, &flag))
			break;
		tprint_indirect_begin();
		printflags(xfs_dqblk_flags, flag, "FS_???_QUOTA");
		tprint_indirect_end();
		break;
	}
	default:
		tprint_arg_next();
		printuid(id);
		tprint_arg_next();
		printaddr(data);
		break;
	}
	return RVAL_DECODED;
}

static void
print_qcmd(const uint32_t qcmd)
{
	const uint32_t cmd = QCMD_CMD(qcmd);
	const uint32_t type = QCMD_TYPE(qcmd);

	if (xlat_verbose(xlat_verbosity) != XLAT_STYLE_ABBREV)
		PRINT_VAL_U(qcmd);

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW)
		return;

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE)
		tprint_comment_begin();

	tprints_arg_begin("QCMD");
	printxvals_ex(cmd, "Q_???", XLAT_STYLE_ABBREV, quotacmds, NULL);
	tprint_arg_next();
	printxvals_ex(type, "???QUOTA", XLAT_STYLE_ABBREV, quotatypes, NULL);
	tprint_arg_end();

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE)
		tprint_comment_end();
}

SYS_FUNC(quotactl)
{
	/*
	 * The Linux kernel only looks at the low 32 bits of command and id
	 * arguments, but on some 64-bit architectures (s390x) this word
	 * will have been sign-extended when we see it.  The high 1 bits
	 * don't mean anything, so don't confuse the output with them.
	 */
	uint32_t qcmd = tcp->u_arg[0];
	uint32_t cmd = QCMD_CMD(qcmd);
	uint32_t id = tcp->u_arg[2];

	if (entering(tcp)) {
		/* cmd */
		print_qcmd(qcmd);
		tprint_arg_next();

		/* special */
		printpath(tcp, tcp->u_arg[1]);
	}
	return decode_cmd_data(tcp, id, cmd, tcp->u_arg[3]);
}

SYS_FUNC(quotactl_fd)
{
	const unsigned int fd = tcp->u_arg[0];
	const unsigned int qcmd = tcp->u_arg[1];
	const uint32_t id = tcp->u_arg[2];
	const kernel_ulong_t addr = tcp->u_arg[3];

	if (entering(tcp)) {
		/* fd */
		printfd(tcp, fd);
		tprint_arg_next();

		/* cmd */
		print_qcmd(qcmd);
	}

	return decode_cmd_data(tcp, id, QCMD_CMD(qcmd), addr);
}
