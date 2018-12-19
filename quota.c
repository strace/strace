/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2005-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2006-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "print_fields.h"
#include "xfs_quota_stat.h"

#define SUBCMDMASK  0x00ff
#define SUBCMDSHIFT 8
#define QCMD_CMD(cmd)	((uint32_t)(cmd) >> SUBCMDSHIFT)
#define QCMD_TYPE(cmd)	((uint32_t)(cmd) & SUBCMDMASK)

#define OLD_CMD(cmd)	((uint32_t)(cmd) << SUBCMDSHIFT)
#define NEW_CMD(cmd)	((uint32_t)(cmd) | 0x800000)
#define XQM_CMD(cmd)	((uint32_t)(cmd) | ('X' << SUBCMDSHIFT))

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

struct xfs_dqblk {
	int8_t  d_version;		/* version of this structure */
	uint8_t  d_flags;		/* XFS_{USER,PROJ,GROUP}_QUOTA */
	uint16_t d_fieldmask;		/* field specifier */
	uint32_t d_id;			/* user, project, or group ID */
	uint64_t d_blk_hardlimit;	/* absolute limit on disk blks */
	uint64_t d_blk_softlimit;	/* preferred limit on disk blks */
	uint64_t d_ino_hardlimit;	/* maximum # allocated inodes */
	uint64_t d_ino_softlimit;	/* preferred inode limit */
	uint64_t d_bcount;		/* # disk blocks owned by the user */
	uint64_t d_icount;		/* # inodes owned by the user */
	int32_t d_itimer;		/* zero if within inode limits */
	int32_t d_btimer;		/* similar to above; for disk blocks */
	uint16_t d_iwarns;		/* # warnings issued wrt num inodes */
	uint16_t d_bwarns;		/* # warnings issued wrt disk blocks */
	int32_t d_padding2;		/* padding2 - for future use */
	uint64_t d_rtb_hardlimit;	/* absolute limit on realtime blks */
	uint64_t d_rtb_softlimit;	/* preferred limit on RT disk blks */
	uint64_t d_rtbcount;		/* # realtime blocks owned */
	int32_t d_rtbtimer;		/* similar to above; for RT disk blks */
	uint16_t d_rtbwarns;		/* # warnings issued wrt RT disk blks */
	int16_t d_padding3;		/* padding3 - for future use */
	char    d_padding4[8];		/* yet more padding */
};

struct if_dqinfo {
	uint64_t dqi_bgrace;
	uint64_t dqi_igrace;
	uint32_t dqi_flags;
	uint32_t dqi_valid;
};

struct fs_qfilestatv {
	uint64_t qfs_ino, qfs_nblks;
	uint32_t qfs_nextents, qfs_pad;
};

struct fs_quota_statv {
	int8_t qs_version;
	uint8_t qs_pad1;
	uint16_t qs_flags;
	uint32_t qs_incoredqs;
	struct fs_qfilestatv qs_uquota;
	struct fs_qfilestatv qs_gquota;
	struct fs_qfilestatv qs_pquota;
	int32_t qs_btimelimit;
	int32_t qs_itimelimit;
	int32_t qs_rtbtimelimit;
	uint16_t qs_bwarnlimit;
	uint16_t qs_iwarnlimit;
	uint64_t qs_pad2[8];
};

static int
decode_cmd_data(struct tcb *tcp, uint32_t id, uint32_t cmd, kernel_ulong_t data)
{
	switch (cmd) {
	case Q_QUOTAOFF:
	case Q_SYNC:
	case Q_XQUOTASYNC:
		break;
	case Q_QUOTAON:
		tprints(", ");
		printxval(quota_formats, id, "QFMT_VFS_???");
		tprints(", ");
		printpath(tcp, data);
		break;
	case Q_GETQUOTA:
		if (entering(tcp)) {
			printuid(", ", id);
			tprints(", ");

			return 0;
		}

		ATTRIBUTE_FALLTHROUGH;
	case Q_SETQUOTA:
	{
		struct if_dqblk dq;

		if (entering(tcp)) {
			printuid(", ", id);
			tprints(", ");
		}

		if (umove_or_printaddr(tcp, data, &dq))
			break;
		PRINT_FIELD_U("{", dq, dqb_bhardlimit);
		PRINT_FIELD_U(", ", dq, dqb_bsoftlimit);
		PRINT_FIELD_U(", ", dq, dqb_curspace);
		PRINT_FIELD_U(", ", dq, dqb_ihardlimit);
		PRINT_FIELD_U(", ", dq, dqb_isoftlimit);
		PRINT_FIELD_U(", ", dq, dqb_curinodes);
		if (!abbrev(tcp)) {
			PRINT_FIELD_U(", ", dq, dqb_btime);
			PRINT_FIELD_U(", ", dq, dqb_itime);
			PRINT_FIELD_FLAGS(", ", dq, dqb_valid,
					  if_dqblk_valid, "QIF_???");
		} else {
			tprints(", ...");
		}
		tprints("}");
		break;
	}
	case Q_GETNEXTQUOTA:
	{
		struct if_nextdqblk dq;

		if (entering(tcp)) {
			printuid(", ", id);
			tprints(", ");

			return 0;
		}

		if (umove_or_printaddr(tcp, data, &dq))
			break;
		PRINT_FIELD_U("{", dq, dqb_bhardlimit);
		PRINT_FIELD_U(", ", dq, dqb_bsoftlimit);
		PRINT_FIELD_U(", ", dq, dqb_curspace);
		PRINT_FIELD_U(", ", dq, dqb_ihardlimit);
		PRINT_FIELD_U(", ", dq, dqb_isoftlimit);
		PRINT_FIELD_U(", ", dq, dqb_curinodes);
		if (!abbrev(tcp)) {
			PRINT_FIELD_U(", ", dq, dqb_btime);
			PRINT_FIELD_U(", ", dq, dqb_itime);
			PRINT_FIELD_FLAGS(", ", dq, dqb_valid,
					  if_dqblk_valid, "QIF_???");
			PRINT_FIELD_U(", ", dq, dqb_id);
		} else {
			PRINT_FIELD_U(", ", dq, dqb_id);
			tprints(", ...");
		}
		tprints("}");
		break;
	}
	case Q_XGETQUOTA:
	case Q_XGETNEXTQUOTA:
		if (entering(tcp)) {
			printuid(", ", id);
			tprints(", ");

			return 0;
		}

		ATTRIBUTE_FALLTHROUGH;
	case Q_XSETQLIM:
	{
		struct xfs_dqblk dq;

		if (entering(tcp)) {
			printuid(", ", id);
			tprints(", ");
		}

		if (umove_or_printaddr(tcp, data, &dq))
			break;
		PRINT_FIELD_D("{", dq, d_version);
		PRINT_FIELD_FLAGS(", ", dq, d_flags,
				  xfs_dqblk_flags, "XFS_???_QUOTA");
		PRINT_FIELD_X(", ", dq, d_fieldmask);
		PRINT_FIELD_U(", ", dq, d_id);
		PRINT_FIELD_U(", ", dq, d_blk_hardlimit);
		PRINT_FIELD_U(", ", dq, d_blk_softlimit);
		PRINT_FIELD_U(", ", dq, d_ino_hardlimit);
		PRINT_FIELD_U(", ", dq, d_ino_softlimit);
		PRINT_FIELD_U(", ", dq, d_bcount);
		PRINT_FIELD_U(", ", dq, d_icount);
		if (!abbrev(tcp)) {
			PRINT_FIELD_D(", ", dq, d_itimer);
			PRINT_FIELD_D(", ", dq, d_btimer);
			PRINT_FIELD_U(", ", dq, d_iwarns);
			PRINT_FIELD_U(", ", dq, d_bwarns);
			PRINT_FIELD_U(", ", dq, d_rtb_hardlimit);
			PRINT_FIELD_U(", ", dq, d_rtb_softlimit);
			PRINT_FIELD_U(", ", dq, d_rtbcount);
			PRINT_FIELD_D(", ", dq, d_rtbtimer);
			PRINT_FIELD_U(", ", dq, d_rtbwarns);
		} else {
			tprints(", ...");
		}
		tprints("}");
		break;
	}
	case Q_GETFMT:
	{
		uint32_t fmt;

		if (entering(tcp)) {
			tprints(", ");

			return 0;
		}

		if (umove_or_printaddr(tcp, data, &fmt))
			break;
		tprints("[");
		printxval(quota_formats, fmt, "QFMT_VFS_???");
		tprints("]");
		break;
	}
	case Q_GETINFO:
		if (entering(tcp)) {
			tprints(", ");

			return 0;
		}

		ATTRIBUTE_FALLTHROUGH;
	case Q_SETINFO:
	{
		struct if_dqinfo dq;

		if (entering(tcp))
			tprints(", ");

		if (umove_or_printaddr(tcp, data, &dq))
			break;
		PRINT_FIELD_U("{", dq, dqi_bgrace);
		PRINT_FIELD_U(", ", dq, dqi_igrace);
		PRINT_FIELD_FLAGS(", ", dq, dqi_flags, if_dqinfo_flags, "DQF_???");
		PRINT_FIELD_FLAGS(", ", dq, dqi_valid, if_dqinfo_valid, "IIF_???");
		tprints("}");
		break;
	}
	case Q_XGETQSTAT:
	{
		struct xfs_dqstats dq;

		if (entering(tcp)) {
			tprints(", ");

			return 0;
		}
		if (fetch_struct_quotastat(tcp, data, &dq)) {
			PRINT_FIELD_D("{", dq, qs_version);
			if (!abbrev(tcp)) {
				PRINT_FIELD_FLAGS(", ", dq, qs_flags,
						  xfs_quota_flags, "XFS_QUOTA_???");
				PRINT_FIELD_U(", qs_uquota={", dq.qs_uquota, qfs_ino);
				PRINT_FIELD_U(", ", dq.qs_uquota, qfs_nblks);
				PRINT_FIELD_U(", ", dq.qs_uquota, qfs_nextents);
				PRINT_FIELD_U("}, qs_gquota={", dq.qs_gquota, qfs_ino);
				PRINT_FIELD_U(", ", dq.qs_gquota, qfs_nblks);
				PRINT_FIELD_U(", ", dq.qs_gquota, qfs_nextents);
				PRINT_FIELD_U("}, ", dq, qs_incoredqs);
				PRINT_FIELD_D(", ", dq, qs_btimelimit);
				PRINT_FIELD_D(", ", dq, qs_itimelimit);
				PRINT_FIELD_D(", ", dq, qs_rtbtimelimit);
				PRINT_FIELD_U(", ", dq, qs_bwarnlimit);
				PRINT_FIELD_U(", ", dq, qs_iwarnlimit);
			} else {
				tprints(", ...");
			}
			tprints("}");
		}
		break;
	}
	case Q_XGETQSTATV:
	{
		struct fs_quota_statv dq;

		if (entering(tcp)) {
			tprints(", ");

			return 0;
		}

		if (umove_or_printaddr(tcp, data, &dq))
			break;
		PRINT_FIELD_D("{", dq, qs_version);
		if (!abbrev(tcp)) {
			PRINT_FIELD_FLAGS(", ", dq, qs_flags,
					  xfs_quota_flags, "XFS_QUOTA_???");
			PRINT_FIELD_U(", ", dq, qs_incoredqs);
			PRINT_FIELD_U(", qs_uquota={", dq.qs_uquota, qfs_ino);
			PRINT_FIELD_U(", ", dq.qs_uquota, qfs_nblks);
			PRINT_FIELD_U(", ", dq.qs_uquota, qfs_nextents);
			PRINT_FIELD_U("}, qs_gquota={", dq.qs_gquota, qfs_ino);
			PRINT_FIELD_U(", ", dq.qs_gquota, qfs_nblks);
			PRINT_FIELD_U(", ", dq.qs_gquota, qfs_nextents);
			PRINT_FIELD_U("}, qs_pquota={", dq.qs_pquota, qfs_ino);
			PRINT_FIELD_U(", ", dq.qs_pquota, qfs_nblks);
			PRINT_FIELD_U(", ", dq.qs_pquota, qfs_nextents);
			PRINT_FIELD_D("}, ", dq, qs_btimelimit);
			PRINT_FIELD_D(", ", dq, qs_itimelimit);
			PRINT_FIELD_D(", ", dq, qs_rtbtimelimit);
			PRINT_FIELD_U(", ", dq, qs_bwarnlimit);
			PRINT_FIELD_U(", ", dq, qs_iwarnlimit);
		} else {
			tprints(", ...");
		}
		tprints("}");
		break;
	}
	case Q_XQUOTAON:
	case Q_XQUOTAOFF:
	{
		uint32_t flag;

		tprints(", ");

		if (umove_or_printaddr(tcp, data, &flag))
			break;
		tprints("[");
		printflags(xfs_quota_flags, flag, "XFS_QUOTA_???");
		tprints("]");
		break;
	}
	case Q_XQUOTARM:
	{
		uint32_t flag;

		tprints(", ");

		if (umove_or_printaddr(tcp, data, &flag))
			break;
		tprints("[");
		printflags(xfs_dqblk_flags, flag, "XFS_???_QUOTA");
		tprints("]");
		break;
	}
	default:
		printuid(", ", id);
		tprints(", ");
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
		tprintf("%u", qcmd);

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW)
		return;

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE)
		tprints(" /* ");

	tprints("QCMD(");
	printxvals_ex(cmd, "Q_???", XLAT_STYLE_ABBREV, quotacmds, NULL);
	tprints(", ");
	printxvals_ex(type, "???QUOTA", XLAT_STYLE_ABBREV, quotatypes, NULL);
	tprints(")");

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE)
		tprints(" */");
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
		print_qcmd(qcmd);
		tprints(", ");
		printpath(tcp, tcp->u_arg[1]);
	}
	return decode_cmd_data(tcp, id, cmd, tcp->u_arg[3]);
}
