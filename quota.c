/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2005-2016 Dmitry V. Levin <ldv@altlinux.org>
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

struct if_dqblk
{
	uint64_t dqb_bhardlimit;
	uint64_t dqb_bsoftlimit;
	uint64_t dqb_curspace;
	uint64_t dqb_ihardlimit;
	uint64_t dqb_isoftlimit;
	uint64_t dqb_curinodes;
	uint64_t dqb_btime;
	uint64_t dqb_itime;
	uint32_t dqb_valid;
};

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

struct v1_dqblk
{
	uint32_t dqb_bhardlimit;	/* absolute limit on disk blks alloc */
	uint32_t dqb_bsoftlimit;	/* preferred limit on disk blks */
	uint32_t dqb_curblocks;		/* current block count */
	uint32_t dqb_ihardlimit;	/* maximum # allocated inodes */
	uint32_t dqb_isoftlimit;	/* preferred inode limit */
	uint32_t dqb_curinodes;		/* current # allocated inodes */
	time_t  dqb_btime;		/* time limit for excessive disk use */
	time_t  dqb_itime;		/* time limit for excessive files */
};

struct v2_dqblk
{
	unsigned int dqb_ihardlimit;
	unsigned int dqb_isoftlimit;
	unsigned int dqb_curinodes;
	unsigned int dqb_bhardlimit;
	unsigned int dqb_bsoftlimit;
	uint64_t dqb_curspace;
	time_t  dqb_btime;
	time_t  dqb_itime;
};

struct xfs_dqblk
{
	int8_t  d_version;		/* version of this structure */
	int8_t  d_flags;		/* XFS_{USER,PROJ,GROUP}_QUOTA */
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

struct if_dqinfo
{
	uint64_t dqi_bgrace;
	uint64_t dqi_igrace;
	uint32_t dqi_flags;
	uint32_t dqi_valid;
};

struct v2_dqinfo
{
	unsigned int dqi_bgrace;
	unsigned int dqi_igrace;
	unsigned int dqi_flags;
	unsigned int dqi_blocks;
	unsigned int dqi_free_blk;
	unsigned int dqi_free_entry;
};

struct v1_dqstats
{
	uint32_t lookups;
	uint32_t drops;
	uint32_t reads;
	uint32_t writes;
	uint32_t cache_hits;
	uint32_t allocated_dquots;
	uint32_t free_dquots;
	uint32_t syncs;
};

struct v2_dqstats
{
	uint32_t lookups;
	uint32_t drops;
	uint32_t reads;
	uint32_t writes;
	uint32_t cache_hits;
	uint32_t allocated_dquots;
	uint32_t free_dquots;
	uint32_t syncs;
	uint32_t version;
};

typedef struct fs_qfilestat
{
	uint64_t qfs_ino;	/* inode number */
	uint64_t qfs_nblks;	/* number of BBs 512-byte-blks */
	uint32_t qfs_nextents;	/* number of extents */
} fs_qfilestat_t;

struct xfs_dqstats
{
	int8_t  qs_version;		/* version number for future changes */
	uint16_t qs_flags;		/* XFS_QUOTA_{U,P,G}DQ_{ACCT,ENFD} */
	int8_t  qs_pad;			/* unused */
	fs_qfilestat_t qs_uquota;	/* user quota storage information */
	fs_qfilestat_t qs_gquota;	/* group quota storage information */
	uint32_t qs_incoredqs;		/* number of dquots incore */
	int32_t qs_btimelimit;		/* limit for blks timer */
	int32_t qs_itimelimit;		/* limit for inodes timer */
	int32_t qs_rtbtimelimit;	/* limit for rt blks timer */
	uint16_t qs_bwarnlimit;		/* limit for num warnings */
	uint16_t qs_iwarnlimit;		/* limit for num warnings */
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
decode_cmd_data(struct tcb *tcp, uint32_t cmd, unsigned long data)
{
	switch (cmd) {
		case Q_GETQUOTA:
			if (entering(tcp))
				return 0;
		case Q_SETQUOTA:
		{
			struct if_dqblk dq;

			if (umove_or_printaddr(tcp, data, &dq))
				break;
			tprintf("{bhardlimit=%" PRIu64 ", ", dq.dqb_bhardlimit);
			tprintf("bsoftlimit=%" PRIu64 ", ", dq.dqb_bsoftlimit);
			tprintf("curspace=%" PRIu64 ", ", dq.dqb_curspace);
			tprintf("ihardlimit=%" PRIu64 ", ", dq.dqb_ihardlimit);
			tprintf("isoftlimit=%" PRIu64 ", ", dq.dqb_isoftlimit);
			tprintf("curinodes=%" PRIu64 ", ", dq.dqb_curinodes);
			if (!abbrev(tcp)) {
				tprintf("btime=%" PRIu64 ", ", dq.dqb_btime);
				tprintf("itime=%" PRIu64 ", ", dq.dqb_itime);
				tprints("valid=");
				printflags(if_dqblk_valid,
					   dq.dqb_valid, "QIF_???");
				tprints("}");
			} else
				tprints("...}");
			break;
		}
		case Q_GETNEXTQUOTA:
		{
			struct if_nextdqblk dq;

			if (entering(tcp))
				return 0;
			if (umove_or_printaddr(tcp, data, &dq))
				break;
			tprintf("{bhardlimit=%" PRIu64 ", ", dq.dqb_bhardlimit);
			tprintf("bsoftlimit=%" PRIu64 ", ", dq.dqb_bsoftlimit);
			tprintf("curspace=%" PRIu64 ", ", dq.dqb_curspace);
			tprintf("ihardlimit=%" PRIu64 ", ", dq.dqb_ihardlimit);
			tprintf("isoftlimit=%" PRIu64 ", ", dq.dqb_isoftlimit);
			tprintf("curinodes=%" PRIu64 ", ", dq.dqb_curinodes);
			if (!abbrev(tcp)) {
				tprintf("btime=%" PRIu64 ", ", dq.dqb_btime);
				tprintf("itime=%" PRIu64 ", ", dq.dqb_itime);
				tprints("valid=");
				printflags(if_dqblk_valid,
					   dq.dqb_valid, "QIF_???");
				tprintf(", id=%u}", dq.dqb_id);
			} else
				tprintf("id=%u, ...}", dq.dqb_id);
			break;
		}
		case Q_V1_GETQUOTA:
			if (entering(tcp))
				return 0;
		case Q_V1_SETQUOTA:
		{
			struct v1_dqblk dq;

			if (umove_or_printaddr(tcp, data, &dq))
				break;
			tprintf("{bhardlimit=%u, ", dq.dqb_bhardlimit);
			tprintf("bsoftlimit=%u, ", dq.dqb_bsoftlimit);
			tprintf("curblocks=%u, ", dq.dqb_curblocks);
			tprintf("ihardlimit=%u, ", dq.dqb_ihardlimit);
			tprintf("isoftlimit=%u, ", dq.dqb_isoftlimit);
			tprintf("curinodes=%u, ", dq.dqb_curinodes);
			tprintf("btime=%lu, ", (long) dq.dqb_btime);
			tprintf("itime=%lu}", (long) dq.dqb_itime);
			break;
		}
		case Q_V2_GETQUOTA:
			if (entering(tcp))
				return 0;
		case Q_V2_SETQUOTA:
		{
			struct v2_dqblk dq;

			if (umove_or_printaddr(tcp, data, &dq))
				break;
			tprintf("{ihardlimit=%u, ", dq.dqb_ihardlimit);
			tprintf("isoftlimit=%u, ", dq.dqb_isoftlimit);
			tprintf("curinodes=%u, ", dq.dqb_curinodes);
			tprintf("bhardlimit=%u, ", dq.dqb_bhardlimit);
			tprintf("bsoftlimit=%u, ", dq.dqb_bsoftlimit);
			tprintf("curspace=%" PRIu64 ", ", dq.dqb_curspace);
			tprintf("btime=%lu, ", (long) dq.dqb_btime);
			tprintf("itime=%lu}", (long) dq.dqb_itime);
			break;
		}
		case Q_XGETQUOTA:
		case Q_XGETNEXTQUOTA:
			if (entering(tcp))
				return 0;
		case Q_XSETQLIM:
		{
			struct xfs_dqblk dq;

			if (umove_or_printaddr(tcp, data, &dq))
				break;
			tprintf("{version=%d, ", dq.d_version);
			tprints("flags=");
			printflags(xfs_dqblk_flags,
				   (uint8_t) dq.d_flags, "XFS_???_QUOTA");
			tprintf(", fieldmask=%#x, ", dq.d_fieldmask);
			tprintf("id=%u, ", dq.d_id);
			tprintf("blk_hardlimit=%" PRIu64 ", ", dq.d_blk_hardlimit);
			tprintf("blk_softlimit=%" PRIu64 ", ", dq.d_blk_softlimit);
			tprintf("ino_hardlimit=%" PRIu64 ", ", dq.d_ino_hardlimit);
			tprintf("ino_softlimit=%" PRIu64 ", ", dq.d_ino_softlimit);
			tprintf("bcount=%" PRIu64 ", ", dq.d_bcount);
			tprintf("icount=%" PRIu64 ", ", dq.d_icount);
			if (!abbrev(tcp)) {
				tprintf("itimer=%d, ", dq.d_itimer);
				tprintf("btimer=%d, ", dq.d_btimer);
				tprintf("iwarns=%u, ", dq.d_iwarns);
				tprintf("bwarns=%u, ", dq.d_bwarns);
				tprintf("rtbcount=%" PRIu64 ", ", dq.d_rtbcount);
				tprintf("rtbtimer=%d, ", dq.d_rtbtimer);
				tprintf("rtbwarns=%u}", dq.d_rtbwarns);
			} else
				tprints("...}");
			break;
		}
		case Q_GETFMT:
		{
			uint32_t fmt;

			if (entering(tcp))
				return 0;
			if (umove_or_printaddr(tcp, data, &fmt))
				break;
			tprints("[");
			printxval(quota_formats, fmt, "QFMT_VFS_???");
			tprints("]");
			break;
		}
		case Q_GETINFO:
			if (entering(tcp))
				return 0;
		case Q_SETINFO:
		{
			struct if_dqinfo dq;

			if (umove_or_printaddr(tcp, data, &dq))
				break;
			tprintf("{bgrace=%" PRIu64 ", ", dq.dqi_bgrace);
			tprintf("igrace=%" PRIu64 ", ", dq.dqi_igrace);
			tprints("flags=");
			printflags(if_dqinfo_flags, dq.dqi_flags, "DQF_???");
			tprints(", valid=");
			printflags(if_dqinfo_valid, dq.dqi_valid, "IIF_???");
			tprints("}");
			break;
		}
		case Q_V2_GETINFO:
			if (entering(tcp))
				return 0;
		case Q_V2_SETINFO:
		{
			struct v2_dqinfo dq;

			if (umove_or_printaddr(tcp, data, &dq))
				break;
			tprintf("{bgrace=%u, ", dq.dqi_bgrace);
			tprintf("igrace=%u, ", dq.dqi_igrace);
			tprints("flags=");
			printflags(if_dqinfo_flags, dq.dqi_flags, "DQF_???");
			tprintf(", blocks=%u, ", dq.dqi_blocks);
			tprintf("free_blk=%u, ", dq.dqi_free_blk);
			tprintf("free_entry=%u}", dq.dqi_free_entry);
			break;
		}
		case Q_V1_GETSTATS:
		{
			struct v1_dqstats dq;

			if (entering(tcp))
				return 0;
			if (umove_or_printaddr(tcp, data, &dq))
				break;
			tprintf("{lookups=%u, ", dq.lookups);
			tprintf("drops=%u, ", dq.drops);
			tprintf("reads=%u, ", dq.reads);
			tprintf("writes=%u, ", dq.writes);
			tprintf("cache_hits=%u, ", dq.cache_hits);
			tprintf("allocated_dquots=%u, ", dq.allocated_dquots);
			tprintf("free_dquots=%u, ", dq.free_dquots);
			tprintf("syncs=%u}", dq.syncs);
			break;
		}
		case Q_V2_GETSTATS:
		{
			struct v2_dqstats dq;

			if (entering(tcp))
				return 0;
			if (umove_or_printaddr(tcp, data, &dq))
				break;
			tprintf("{lookups=%u, ", dq.lookups);
			tprintf("drops=%u, ", dq.drops);
			tprintf("reads=%u, ", dq.reads);
			tprintf("writes=%u, ", dq.writes);
			tprintf("cache_hits=%u, ", dq.cache_hits);
			tprintf("allocated_dquots=%u, ", dq.allocated_dquots);
			tprintf("free_dquots=%u, ", dq.free_dquots);
			tprintf("syncs=%u, ", dq.syncs);
			tprintf("version=%u}", dq.version);
			break;
		}
		case Q_XGETQSTAT:
		{
			struct xfs_dqstats dq;

			if (entering(tcp))
				return 0;
			if (umove_or_printaddr(tcp, data, &dq))
				break;
			tprintf("{version=%d, ", dq.qs_version);
			if (abbrev(tcp)) {
				tprints("...}");
				break;
			}
			tprints("flags=");
			printflags(xfs_quota_flags,
				   dq.qs_flags, "XFS_QUOTA_???");
			tprintf(", incoredqs=%u, ", dq.qs_incoredqs);
			tprintf("u_ino=%" PRIu64 ", ", dq.qs_uquota.qfs_ino);
			tprintf("u_nblks=%" PRIu64 ", ", dq.qs_uquota.qfs_nblks);
			tprintf("u_nextents=%u, ", dq.qs_uquota.qfs_nextents);
			tprintf("g_ino=%" PRIu64 ", ", dq.qs_gquota.qfs_ino);
			tprintf("g_nblks=%" PRIu64 ", ", dq.qs_gquota.qfs_nblks);
			tprintf("g_nextents=%u, ", dq.qs_gquota.qfs_nextents);
			tprintf("btimelimit=%d, ", dq.qs_btimelimit);
			tprintf("itimelimit=%d, ", dq.qs_itimelimit);
			tprintf("rtbtimelimit=%d, ", dq.qs_rtbtimelimit);
			tprintf("bwarnlimit=%u, ", dq.qs_bwarnlimit);
			tprintf("iwarnlimit=%u}", dq.qs_iwarnlimit);
			break;
		}
		case Q_XGETQSTATV:
		{
			struct fs_quota_statv dq;

			if (entering(tcp))
				return 0;
			if (umove_or_printaddr(tcp, data, &dq))
				break;
			tprintf("{version=%d, ", dq.qs_version);
			if (abbrev(tcp)) {
				tprints("...}");
				break;
			}
			tprints("flags=");
			printflags(xfs_quota_flags,
				   dq.qs_flags, "XFS_QUOTA_???");
			tprintf(", incoredqs=%u, ", dq.qs_incoredqs);
			tprintf("u_ino=%" PRIu64 ", ", dq.qs_uquota.qfs_ino);
			tprintf("u_nblks=%" PRIu64 ", ", dq.qs_uquota.qfs_nblks);
			tprintf("u_nextents=%u, ", dq.qs_uquota.qfs_nextents);
			tprintf("g_ino=%" PRIu64 ", ", dq.qs_gquota.qfs_ino);
			tprintf("g_nblks=%" PRIu64 ", ", dq.qs_gquota.qfs_nblks);
			tprintf("g_nextents=%u, ", dq.qs_gquota.qfs_nextents);
			tprintf("p_ino=%" PRIu64 ", ", dq.qs_pquota.qfs_ino);
			tprintf("p_nblks=%" PRIu64 ", ", dq.qs_pquota.qfs_nblks);
			tprintf("p_nextents=%u, ", dq.qs_pquota.qfs_nextents);
			tprintf("btimelimit=%d, ", dq.qs_btimelimit);
			tprintf("itimelimit=%d, ", dq.qs_itimelimit);
			tprintf("rtbtimelimit=%d, ", dq.qs_rtbtimelimit);
			tprintf("bwarnlimit=%u, ", dq.qs_bwarnlimit);
			tprintf("iwarnlimit=%u}", dq.qs_iwarnlimit);
			break;
		}
		case Q_XQUOTAON:
		case Q_XQUOTAOFF:
		{
			uint32_t flag;

			if (umove_or_printaddr(tcp, data, &flag))
				break;
			tprints("[");
			printflags(xfs_quota_flags, flag, "XFS_QUOTA_???");
			tprints("]");
			break;
		}
		default:
			printaddr(data);
			break;
	}
	return RVAL_DECODED;
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
	uint32_t type = QCMD_TYPE(qcmd);
	uint32_t id = tcp->u_arg[2];

	if (entering(tcp)) {
		printxval(quotacmds, cmd, "Q_???");
		tprints("|");
		printxval(quotatypes, type, "???QUOTA");
		tprints(", ");
		printpath(tcp, tcp->u_arg[1]);
		tprints(", ");
		switch (cmd) {
			case Q_QUOTAON:
			case Q_V1_QUOTAON:
				printxval(quota_formats, id, "QFMT_VFS_???");
				tprints(", ");
				printpath(tcp, tcp->u_arg[3]);
				return RVAL_DECODED;
		}
		tprintf("%u, ", id);
	}
	return decode_cmd_data(tcp, cmd, tcp->u_arg[3]);
}
