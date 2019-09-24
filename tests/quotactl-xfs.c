/*
 * Check decoding of quotactl xfs subcommands.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include "scno.h"

#if defined(__NR_quotactl) && \
	(defined(HAVE_LINUX_QUOTA_H) || defined(HAVE_SYS_QUOTA_H)) && \
	defined(HAVE_LINUX_DQBLK_XFS_H)

# include <stdio.h>
# include <string.h>
# include <unistd.h>

# include <linux/dqblk_xfs.h>

# include "quotactl.h"

# ifndef Q_GETNEXTQUOTA
#  define Q_XGETNEXTQUOTA	XQM_CMD(0x9)
# endif /* !Q_GETNEXTQUOTA */

# ifndef Q_XGETQSTATV

#  define Q_XGETQSTATV		XQM_CMD(8)
#  define FS_QSTATV_VERSION1	1

struct fs_qfilestatv {
	uint64_t	qfs_ino;	/* inode number */
	uint64_t	qfs_nblks;	/* number of BBs 512-byte-blks */
	uint32_t	qfs_nextents;	/* number of extents */
	uint32_t	qfs_pad;	/* pad for 8-byte alignment */
};

struct fs_quota_statv {
	int8_t		qs_version;		/* version for future changes */
	uint8_t		qs_pad1;		/* pad for 16bit alignment */
	uint16_t	qs_flags;		/* XFS_QUOTA_.* flags */
	uint32_t	qs_incoredqs;		/* number of dquots incore */
	struct fs_qfilestatv	qs_uquota;	/* user quota information */
	struct fs_qfilestatv	qs_gquota;	/* group quota information */
	struct fs_qfilestatv	qs_pquota;	/* project quota information */
	int32_t		qs_btimelimit;		/* limit for blks timer */
	int32_t		qs_itimelimit;		/* limit for inodes timer */
	int32_t		qs_rtbtimelimit;	/* limit for rt blks timer */
	uint16_t	qs_bwarnlimit;		/* limit for num warnings */
	uint16_t	qs_iwarnlimit;		/* limit for num warnings */
	uint64_t	qs_pad2[8];		/* for future proofing */
};

# endif /* !Q_XGETQSTATV */

# include "xlat.h"
# include "xlat/xfs_dqblk_flags.h"
# if VERBOSE
#  include "xlat/xfs_quota_flags.h"
# endif


void
print_xdisk_quota(int rc, void *ptr, void *arg)
{
	struct fs_disk_quota *dq = ptr;
	long out_arg = (long) arg;

	if (((rc < 0) && out_arg) || (out_arg > 1)) {
		printf("%p", dq);
		return;
	}

	PRINT_FIELD_D("{", *dq, d_version);
	printf(", d_flags=");
	printflags(xfs_dqblk_flags, (uint8_t) dq->d_flags, "XFS_???_QUOTA");

	PRINT_FIELD_X(", ", *dq, d_fieldmask);
	PRINT_FIELD_U(", ", *dq, d_id);
	PRINT_FIELD_U(", ", *dq, d_blk_hardlimit);
	PRINT_FIELD_U(", ", *dq, d_blk_softlimit);
	PRINT_FIELD_U(", ", *dq, d_ino_hardlimit);
	PRINT_FIELD_U(", ", *dq, d_ino_softlimit);
	PRINT_FIELD_U(", ", *dq, d_bcount);
	PRINT_FIELD_U(", ", *dq, d_icount);

# if VERBOSE
	PRINT_FIELD_D(", ", *dq, d_itimer);
	PRINT_FIELD_D(", ", *dq, d_btimer);
	PRINT_FIELD_U(", ", *dq, d_iwarns);
	PRINT_FIELD_U(", ", *dq, d_bwarns);
	PRINT_FIELD_U(", ", *dq, d_rtb_hardlimit);
	PRINT_FIELD_U(", ", *dq, d_rtb_softlimit);
	PRINT_FIELD_U(", ", *dq, d_rtbcount);
	PRINT_FIELD_D(", ", *dq, d_rtbtimer);
	PRINT_FIELD_U(", ", *dq, d_rtbwarns);
# else
	printf(", ...");
# endif /* !VERBOSE */
	printf("}");
}

void
print_xquota_stat(int rc, void *ptr, void *arg)
{
	struct fs_quota_stat *qs = ptr;
	long out_arg = (long) arg;

	if (((rc < 0) && out_arg) || (out_arg > 1)) {
		printf("%p", qs);
		return;
	}

	PRINT_FIELD_D("{", *qs, qs_version);

# if VERBOSE
	printf(", qs_flags=");
	printflags(xfs_quota_flags, qs->qs_flags, "XFS_QUOTA_???");
	PRINT_FIELD_U(", qs_uquota={", qs->qs_uquota, qfs_ino);
	PRINT_FIELD_U(", ", qs->qs_uquota, qfs_nblks);
	PRINT_FIELD_U(", ", qs->qs_uquota, qfs_nextents);
	PRINT_FIELD_U("}, qs_gquota={", qs->qs_gquota, qfs_ino);
	PRINT_FIELD_U(", ", qs->qs_gquota, qfs_nblks);
	PRINT_FIELD_U(", ", qs->qs_gquota, qfs_nextents);
	PRINT_FIELD_U("}, ", *qs, qs_incoredqs);
	PRINT_FIELD_D(", ", *qs, qs_btimelimit);
	PRINT_FIELD_D(", ", *qs, qs_itimelimit);
	PRINT_FIELD_D(", ", *qs, qs_rtbtimelimit);
	PRINT_FIELD_U(", ", *qs, qs_bwarnlimit);
	PRINT_FIELD_U(", ", *qs, qs_iwarnlimit);
# else
	printf(", ...");
# endif /* !VERBOSE */
	printf("}");
}

void
print_xquota_statv(int rc, void *ptr, void *arg)
{
	struct fs_quota_statv *qs = ptr;
	long out_arg = (long) arg;

	if (((rc < 0) && out_arg) || (out_arg > 1)) {
		printf("%p", qs);
		return;
	}

	PRINT_FIELD_D("{", *qs, qs_version);

# if VERBOSE
	printf(", qs_flags=");
	printflags(xfs_quota_flags, qs->qs_flags, "XFS_QUOTA_???");
	PRINT_FIELD_U(", ", *qs, qs_incoredqs);
	PRINT_FIELD_U(", qs_uquota={", qs->qs_uquota, qfs_ino);
	PRINT_FIELD_U(", ", qs->qs_uquota, qfs_nblks);
	PRINT_FIELD_U(", ", qs->qs_uquota, qfs_nextents);
	PRINT_FIELD_U("}, qs_gquota={", qs->qs_gquota, qfs_ino);
	PRINT_FIELD_U(", ", qs->qs_gquota, qfs_nblks);
	PRINT_FIELD_U(", ", qs->qs_gquota, qfs_nextents);
	PRINT_FIELD_U("}, qs_pquota={", qs->qs_pquota, qfs_ino);
	PRINT_FIELD_U(", ", qs->qs_pquota, qfs_nblks);
	PRINT_FIELD_U(", ", qs->qs_pquota, qfs_nextents);
	PRINT_FIELD_D("}, ", *qs, qs_btimelimit);
	PRINT_FIELD_D(", ", *qs, qs_itimelimit);
	PRINT_FIELD_D(", ", *qs, qs_rtbtimelimit);
	PRINT_FIELD_U(", ", *qs, qs_bwarnlimit);
	PRINT_FIELD_U(", ", *qs, qs_iwarnlimit);
# else
	printf(", ...");
# endif /* !VERBOSE */
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
		    "[XFS_QUOTA_UDQ_ACCT|XFS_QUOTA_UDQ_ENFD"
		    "|XFS_QUOTA_GDQ_ACCT|XFS_QUOTA_GDQ_ENFD"
		    "|XFS_QUOTA_PDQ_ENFD|0xdeadbec0]");

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
		    "[XFS_QUOTA_UDQ_ACCT|XFS_QUOTA_UDQ_ENFD"
		    "|XFS_QUOTA_GDQ_ACCT|XFS_QUOTA_GDQ_ENFD"
		    "|XFS_QUOTA_PDQ_ENFD|0xdeadbec0]");


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
		    "[XFS_USER_QUOTA|XFS_PROJ_QUOTA"
		    "|XFS_GROUP_QUOTA|0xdeadbee8]");


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

#else

SKIP_MAIN_UNDEFINED("__NR_quotactl && "
	"(HAVE_LINUX_QUOTA_H || HAVE_SYS_QUOTA_H) && "
	"HAVE_LINUX_DQBLK_XFS_H");

#endif
