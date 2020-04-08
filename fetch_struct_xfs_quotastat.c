/*
 * Copyright (c) 2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include DEF_MPERS_TYPE(struct_xfs_dqstats)

#include "xfs_quota_stat.h"
typedef struct xfs_dqstats struct_xfs_dqstats;

#include MPERS_DEFS

MPERS_PRINTER_DECL(bool, fetch_struct_quotastat, struct tcb *const tcp,
		   const kernel_ulong_t data, void *p)
{
	struct xfs_dqstats *dq = p;
	struct_xfs_dqstats dqstat;

	if (umove_or_printaddr(tcp, data, &dqstat))
		return false;

	dq->qs_version = dqstat.qs_version;
	dq->qs_flags = dqstat.qs_flags;
	dq->qs_pad = dqstat.qs_pad;
	dq->qs_uquota.qfs_ino = dqstat.qs_uquota.qfs_ino;
	dq->qs_uquota.qfs_nblks = dqstat.qs_uquota.qfs_nblks;
	dq->qs_uquota.qfs_nextents = dqstat.qs_uquota.qfs_nextents;
	dq->qs_gquota.qfs_ino = dqstat.qs_gquota.qfs_ino;
	dq->qs_gquota.qfs_nblks = dqstat.qs_gquota.qfs_nblks;
	dq->qs_gquota.qfs_nextents = dqstat.qs_gquota.qfs_nextents;
	dq->qs_incoredqs = dqstat.qs_incoredqs;
	dq->qs_btimelimit = dqstat.qs_btimelimit;
	dq->qs_itimelimit = dqstat.qs_itimelimit;
	dq->qs_rtbtimelimit = dqstat.qs_rtbtimelimit;
	dq->qs_bwarnlimit = dqstat.qs_bwarnlimit;
	dq->qs_iwarnlimit = dqstat.qs_iwarnlimit;
	return true;
}
