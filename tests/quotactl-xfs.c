/*
 * Check decoding of quotactl xfs subcommands.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <linux/dqblk_xfs.h>

#include "quotactl.h"

#include "xlat.h"
#include "xlat/xfs_dqblk_flags.h"
#if VERBOSE
# include "xlat/xfs_quota_flags.h"
#endif


static void
print_xdisk_quota(int rc, void *ptr, void *arg)
{
	struct fs_disk_quota *dq = ptr;
	long out_arg = (long) arg;

	if (((rc < 0) && out_arg) || (out_arg > 1)) {
		printf("%p", dq);
		return;
	}

	printf("{");
	PRINT_FIELD_D(*dq, d_version);
	printf(", d_flags=");
	printflags(xfs_dqblk_flags, (uint8_t) dq->d_flags, "FS_???_QUOTA");

	printf(", ");
	PRINT_FIELD_X(*dq, d_fieldmask);
	printf(", ");
	PRINT_FIELD_U(*dq, d_id);
	printf(", ");
	PRINT_FIELD_U(*dq, d_blk_hardlimit);
	printf(", ");
	PRINT_FIELD_U(*dq, d_blk_softlimit);
	printf(", ");
	PRINT_FIELD_U(*dq, d_ino_hardlimit);
	printf(", ");
	PRINT_FIELD_U(*dq, d_ino_softlimit);
	printf(", ");
	PRINT_FIELD_U(*dq, d_bcount);
	printf(", ");
	PRINT_FIELD_U(*dq, d_icount);

#if VERBOSE
	printf(", ");
	PRINT_FIELD_D(*dq, d_itimer);
	printf(", ");
	PRINT_FIELD_D(*dq, d_btimer);
	printf(", ");
	PRINT_FIELD_U(*dq, d_iwarns);
	printf(", ");
	PRINT_FIELD_U(*dq, d_bwarns);
	printf(", ");
	PRINT_FIELD_U(*dq, d_rtb_hardlimit);
	printf(", ");
	PRINT_FIELD_U(*dq, d_rtb_softlimit);
	printf(", ");
	PRINT_FIELD_U(*dq, d_rtbcount);
	printf(", ");
	PRINT_FIELD_D(*dq, d_rtbtimer);
	printf(", ");
	PRINT_FIELD_U(*dq, d_rtbwarns);
#else
	printf(", ...");
#endif /* !VERBOSE */
	printf("}");
}

static void
print_xquota_stat(int rc, void *ptr, void *arg)
{
	struct fs_quota_stat *qs = ptr;
	long out_arg = (long) arg;

	if (((rc < 0) && out_arg) || (out_arg > 1)) {
		printf("%p", qs);
		return;
	}

	printf("{");
	PRINT_FIELD_D(*qs, qs_version);

#if VERBOSE
	printf(", qs_flags=");
	printflags(xfs_quota_flags, qs->qs_flags, "FS_QUOTA_???");
	printf(", qs_uquota={");
	PRINT_FIELD_U(qs->qs_uquota, qfs_ino);
	printf(", ");
	PRINT_FIELD_U(qs->qs_uquota, qfs_nblks);
	printf(", ");
	PRINT_FIELD_U(qs->qs_uquota, qfs_nextents);
	printf("}, qs_gquota={");
	PRINT_FIELD_U(qs->qs_gquota, qfs_ino);
	printf(", ");
	PRINT_FIELD_U(qs->qs_gquota, qfs_nblks);
	printf(", ");
	PRINT_FIELD_U(qs->qs_gquota, qfs_nextents);
	printf("}, ");
	PRINT_FIELD_U(*qs, qs_incoredqs);
	printf(", ");
	PRINT_FIELD_D(*qs, qs_btimelimit);
	printf(", ");
	PRINT_FIELD_D(*qs, qs_itimelimit);
	printf(", ");
	PRINT_FIELD_D(*qs, qs_rtbtimelimit);
	printf(", ");
	PRINT_FIELD_U(*qs, qs_bwarnlimit);
	printf(", ");
	PRINT_FIELD_U(*qs, qs_iwarnlimit);
#else
	printf(", ...");
#endif /* !VERBOSE */
	printf("}");
}

static void
print_xquota_statv(int rc, void *ptr, void *arg)
{
	struct fs_quota_statv *qs = ptr;
	long out_arg = (long) arg;

	if (((rc < 0) && out_arg) || (out_arg > 1)) {
		printf("%p", qs);
		return;
	}

	printf("{");
	PRINT_FIELD_D(*qs, qs_version);

#if VERBOSE
	printf(", qs_flags=");
	printflags(xfs_quota_flags, qs->qs_flags, "FS_QUOTA_???");
	printf(", ");
	PRINT_FIELD_U(*qs, qs_incoredqs);
	printf(", qs_uquota={");
	PRINT_FIELD_U(qs->qs_uquota, qfs_ino);
	printf(", ");
	PRINT_FIELD_U(qs->qs_uquota, qfs_nblks);
	printf(", ");
	PRINT_FIELD_U(qs->qs_uquota, qfs_nextents);
	printf("}, qs_gquota={");
	PRINT_FIELD_U(qs->qs_gquota, qfs_ino);
	printf(", ");
	PRINT_FIELD_U(qs->qs_gquota, qfs_nblks);
	printf(", ");
	PRINT_FIELD_U(qs->qs_gquota, qfs_nextents);
	printf("}, qs_pquota={");
	PRINT_FIELD_U(qs->qs_pquota, qfs_ino);
	printf(", ");
	PRINT_FIELD_U(qs->qs_pquota, qfs_nblks);
	printf(", ");
	PRINT_FIELD_U(qs->qs_pquota, qfs_nextents);
	printf("}, ");
	PRINT_FIELD_D(*qs, qs_btimelimit);
	printf(", ");
	PRINT_FIELD_D(*qs, qs_itimelimit);
	printf(", ");
	PRINT_FIELD_D(*qs, qs_rtbtimelimit);
	printf(", ");
	PRINT_FIELD_U(*qs, qs_bwarnlimit);
	printf(", ");
	PRINT_FIELD_U(*qs, qs_iwarnlimit);
#else
	printf(", ...");
#endif /* !VERBOSE */
	printf("}");
}

int
main(void)
{
	char *bogus_special = (char *) tail_alloc(1) + 1;
	void *bogus_addr = (char *) tail_alloc(1) + 1;

	char bogus_special_str[sizeof(void *) * 2 + sizeof("0x")];
	char bogus_addr_str[sizeof(void *) * 2 + sizeof("0x")];
	char unterminated_str[sizeof(void *) * 2 + sizeof("0x")];

	static char invalid_cmd_str[1024];
	TAIL_ALLOC_OBJECT_CONST_PTR(struct fs_disk_quota, xdq);
	TAIL_ALLOC_OBJECT_CONST_PTR(struct fs_quota_stat, xqstat);
	TAIL_ALLOC_OBJECT_CONST_PTR(struct fs_quota_statv, xqstatv);
	TAIL_ALLOC_OBJECT_CONST_PTR(uint32_t, flags);
	char *unterminated = tail_memdup(unterminated_data,
					 sizeof(unterminated_data));

	snprintf(bogus_special_str, sizeof(bogus_special_str), "%p",
		 bogus_special);
	snprintf(bogus_addr_str, sizeof(bogus_addr_str), "%p",
		 bogus_addr);
	snprintf(unterminated_str, sizeof(unterminated_str), "%p",
		 unterminated);


	/* Q_XQUOTAON */

	*flags = 0xdeadbeef;

	check_quota(CQF_ID_SKIP | CQF_ADDR_STR,
		    ARG_STR(QCMD(Q_XQUOTAON, USRQUOTA)),
		    ARG_STR("/dev/bogus/"), flags,
		    "[FS_QUOTA_UDQ_ACCT|FS_QUOTA_UDQ_ENFD"
		    "|FS_QUOTA_GDQ_ACCT|FS_QUOTA_GDQ_ENFD"
		    "|FS_QUOTA_PDQ_ENFD|0xdeadbec0]");

	snprintf(invalid_cmd_str, sizeof(invalid_cmd_str),
		 "QCMD(Q_XQUOTAON, %#x /* ???QUOTA */)",
		 QCMD_TYPE(QCMD(Q_XQUOTAON, 0xfacefeed)));
	check_quota(CQF_ID_SKIP,
		    QCMD(Q_XQUOTAON, 0xfacefeed), invalid_cmd_str,
		    bogus_dev, bogus_dev_str, bogus_addr);


	/* Q_XQUOTAOFF */

	check_quota(CQF_ID_SKIP | CQF_ADDR_STR,
		    ARG_STR(QCMD(Q_XQUOTAOFF, USRQUOTA)),
		    bogus_special, bogus_special_str,
		    bogus_addr, bogus_addr_str);
	check_quota(CQF_ID_SKIP | CQF_ADDR_STR,
		    ARG_STR(QCMD(Q_XQUOTAOFF, GRPQUOTA)),
		    ARG_STR("/dev/bogus/"),
		    ARG_STR(NULL));
	check_quota(CQF_ID_SKIP | CQF_ADDR_STR,
		    QCMD(Q_XQUOTAOFF, 3),
		    "QCMD(Q_XQUOTAOFF, 0x3 /* ???QUOTA */)",
		    ARG_STR("/dev/bogus/"), flags,
		    "[FS_QUOTA_UDQ_ACCT|FS_QUOTA_UDQ_ENFD"
		    "|FS_QUOTA_GDQ_ACCT|FS_QUOTA_GDQ_ENFD"
		    "|FS_QUOTA_PDQ_ENFD|0xdeadbec0]");


	/* Q_XGETQUOTA */

	/* Trying our best to get successful result */
	check_quota(CQF_ADDR_CB, ARG_STR(QCMD(Q_XGETQUOTA, USRQUOTA)),
		    ARG_STR("/dev/sda1"), getuid(), xdq, print_xdisk_quota,
		    (intptr_t) 1);

	check_quota(CQF_ADDR_CB, ARG_STR(QCMD(Q_XGETQUOTA, GRPQUOTA)),
		    ARG_STR(NULL), -1, xdq, print_xdisk_quota, (intptr_t) 1);


	/* Q_XGETNEXTQUOTA */

	check_quota(CQF_ADDR_CB, ARG_STR(QCMD(Q_XGETNEXTQUOTA, USRQUOTA)),
		    ARG_STR("/dev/sda1"), 0, xdq, print_xdisk_quota,
		    (intptr_t) 1);


	/* Q_XSETQLIM */

	check_quota(CQF_NONE, ARG_STR(QCMD(Q_XSETQLIM, PRJQUOTA)),
		    bogus_special, bogus_special_str, 0, bogus_addr);

	fill_memory_ex(xdq, sizeof(*xdq), 0x8e, 0x80);

	check_quota(CQF_ADDR_CB, ARG_STR(QCMD(Q_XSETQLIM, PRJQUOTA)),
		    bogus_dev, bogus_dev_str, 3141592653U,
		    xdq, print_xdisk_quota, (intptr_t) 0);


	/* Q_XGETQSTAT */

	check_quota(CQF_ID_SKIP | CQF_ADDR_CB,
		    ARG_STR(QCMD(Q_XGETQSTAT, USRQUOTA)),
		    ARG_STR("/dev/sda1"), xqstat, print_xquota_stat, (intptr_t) 1);

	check_quota(CQF_ID_SKIP | CQF_ADDR_CB,
		    ARG_STR(QCMD(Q_XGETQSTAT, USRQUOTA)),
		    ARG_STR("NULL"), xqstat, print_xquota_stat, (intptr_t) 1);

	check_quota(CQF_ID_SKIP,
		    ARG_STR(QCMD(Q_XGETQSTAT, PRJQUOTA)),
		    unterminated, unterminated_str,
		    xqstat + 1);


	/* Q_XGETQSTATV */

	check_quota(CQF_ID_SKIP | CQF_ADDR_CB,
		    ARG_STR(QCMD(Q_XGETQSTATV, USRQUOTA)),
		    ARG_STR("/dev/sda1"), xqstatv, print_xquota_statv, (intptr_t) 1);

	check_quota(CQF_ID_SKIP | CQF_ADDR_CB,
		    ARG_STR(QCMD(Q_XGETQSTATV, GRPQUOTA)),
		    ARG_STR(NULL), xqstatv, print_xquota_statv, (intptr_t) 1);

	check_quota(CQF_ID_SKIP,
		    ARG_STR(QCMD(Q_XGETQSTATV, PRJQUOTA)),
		    unterminated, unterminated_str,
		    xqstatv + 1);


	/* Q_XQUOTARM */

	check_quota(CQF_ID_SKIP | CQF_ADDR_STR,
		    ARG_STR(QCMD(Q_XQUOTARM, PRJQUOTA)),
		    bogus_special, bogus_special_str, ARG_STR(NULL));
	check_quota(CQF_ID_SKIP,
		    ARG_STR(QCMD(Q_XQUOTARM, USRQUOTA)),
		    unterminated, unterminated_str, flags + 1);

	*flags = 0xdeadbeef;
	check_quota(CQF_ID_SKIP | CQF_ADDR_STR,
		    ARG_STR(QCMD(Q_XQUOTARM, GRPQUOTA)),
		    ARG_STR(NULL), flags,
		    "[FS_USER_QUOTA|FS_PROJ_QUOTA"
		    "|FS_GROUP_QUOTA|0xdeadbee8]");


	/* Q_XQUOTASYNC */

	check_quota(CQF_ID_SKIP | CQF_ADDR_SKIP,
		    ARG_STR(QCMD(Q_XQUOTASYNC, USRQUOTA)),
		    bogus_special, bogus_special_str);
	check_quota(CQF_ID_SKIP | CQF_ADDR_SKIP,
		    QCMD(Q_XQUOTASYNC, 0xfff),
		    "QCMD(Q_XQUOTASYNC, 0xff /* ???QUOTA */)",
		    ARG_STR(NULL));

	puts("+++ exited with 0 +++");

	return 0;
}
