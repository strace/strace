/*
 * Copyright (c) 2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdint.h>
#include "kernel_types.h"

typedef struct fs_qfilestat {
	uint64_t qfs_ino;	/* inode number */
	uint64_t qfs_nblks;	/* number of BBs 512-byte-blks */
	uint32_t qfs_nextents;	/* number of extents */
} fs_qfilestat_t;

struct xfs_dqstats {
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
