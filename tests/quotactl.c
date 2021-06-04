/*
 * Check decoding of quotactl syscall.
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

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "quotactl.h"

#include "xlat.h"
#include "xlat/quota_formats.h"
#include "xlat/if_dqblk_valid.h"
#include "xlat/if_dqinfo_flags.h"
#include "xlat/if_dqinfo_valid.h"

#define QUOTA_STR(_arg) (_arg), gen_quotacmd(#_arg, _arg)
#define QUOTA_ID_STR(_arg) (_arg), gen_quotaid(#_arg, _arg)
#define QUOTA_STR_INVALID(_arg, str) (_arg), gen_quotacmd(str, _arg)

static void
print_dqblk(long rc, void *ptr, void *arg)
{
	struct if_dqblk *db = ptr;
	long out_arg = (long) arg;

	if (((rc < 0) && out_arg) || (out_arg > 1)) {
		printf("%p", db);
		return;
	}

	printf("{");
	PRINT_FIELD_U(*db, dqb_bhardlimit);
	printf(", ");
	PRINT_FIELD_U(*db, dqb_bsoftlimit);
	printf(", ");
	PRINT_FIELD_U(*db, dqb_curspace);
	printf(", ");
	PRINT_FIELD_U(*db, dqb_ihardlimit);
	printf(", ");
	PRINT_FIELD_U(*db, dqb_isoftlimit);
	printf(", ");
	PRINT_FIELD_U(*db, dqb_curinodes);

#if VERBOSE
	printf(", ");
	PRINT_FIELD_U(*db, dqb_btime);
	printf(", ");
	PRINT_FIELD_U(*db, dqb_itime);

	printf(", dqb_valid=");
	printflags(if_dqblk_valid, db->dqb_valid, "QIF_???");
#else
	printf(", ...");
#endif /* !VERBOSE */
	printf("}");
}

static void
print_nextdqblk(long rc, void *ptr, void *arg)
{
	struct if_nextdqblk *db = ptr;
	long out_arg = (long) arg;

	if (((rc < 0) && out_arg) || (out_arg > 1)) {
		printf("%p", db);
		return;
	}

	printf("{");
	PRINT_FIELD_U(*db, dqb_bhardlimit);
	printf(", ");
	PRINT_FIELD_U(*db, dqb_bsoftlimit);
	printf(", ");
	PRINT_FIELD_U(*db, dqb_curspace);
	printf(", ");
	PRINT_FIELD_U(*db, dqb_ihardlimit);
	printf(", ");
	PRINT_FIELD_U(*db, dqb_isoftlimit);
	printf(", ");
	PRINT_FIELD_U(*db, dqb_curinodes);

#if VERBOSE
	printf(", ");
	PRINT_FIELD_U(*db, dqb_btime);
	printf(", ");
	PRINT_FIELD_U(*db, dqb_itime);

	printf(", dqb_valid=");
	printflags(if_dqblk_valid, db->dqb_valid, "QIF_???");

	printf(", ");
	PRINT_FIELD_U(*db, dqb_id);
#else
	printf(", ");
	PRINT_FIELD_U(*db, dqb_id);
	printf(", ...");
#endif /* !VERBOSE */
	printf("}");
}

static void
print_dqinfo(long rc, void *ptr, void *arg)
{
	struct if_dqinfo *di = ptr;
	long out_arg = (long) arg;

	if (((rc < 0) && out_arg) || (out_arg > 1)) {
		printf("%p", di);
		return;
	}

	printf("{");
	PRINT_FIELD_U(*di, dqi_bgrace);
	printf(", ");
	PRINT_FIELD_U(*di, dqi_igrace);

	printf(", dqi_flags=");
#if XLAT_RAW
	printf("%#x", di->dqi_flags);
#elif XLAT_VERBOSE
	printf("%#x /* ", di->dqi_flags);
	printflags(if_dqinfo_flags, di->dqi_flags, "DQF_???");
	printf(" */");
#else /* XLAT_ABBREV */
	printflags(if_dqinfo_flags, di->dqi_flags, "DQF_???");
#endif
	printf(", dqi_valid=");
#if XLAT_RAW
	printf("%#x", di->dqi_valid);
#elif XLAT_VERBOSE
	printf("%#x /* ", di->dqi_valid);
	printflags(if_dqinfo_valid, di->dqi_valid, "IIF_???");
	printf(" */");
#else /* XLAT_ABBREV */
	printflags(if_dqinfo_valid, di->dqi_valid, "IIF_???");
#endif
	printf("}");
}

static void
print_dqfmt(long rc, void *ptr, void *arg)
{
	uint32_t *fmtval = ptr;
	long out_arg = (long) arg;
	const char *fmtstr;

	if (((rc < 0) && out_arg) || (out_arg > 1)) {
		printf("%p", fmtval);
		return;
	}
	printf("[");
#if XLAT_RAW
	printf("%#x]", *fmtval);
	return;
#else
	switch (*fmtval) {
	case 1:
		fmtstr = "QFMT_VFS_OLD";
		break;
	case 2:
		fmtstr = "QFMT_VFS_V0";
		break;
	case 3:
		fmtstr = "QFMT_OCFS2";
		break;
	case 4:
		fmtstr = "QFMT_VFS_V1";
		break;
	default:
		printf("%#x /* QFMT_VFS_??? */]", *fmtval);
		return;
	}
#endif
#if XLAT_VERBOSE
	printf("%#x /* %s */]", *fmtval, fmtstr);
#else
	printf("%s]", fmtstr);
#endif
}

static const char *
gen_quotacmd(const char *abbrev_str, const uint32_t cmd)
{
	static char quotacmd_str[2048];

#if XLAT_RAW
	snprintf(quotacmd_str, sizeof(quotacmd_str), "%u", cmd);
#elif XLAT_VERBOSE
	snprintf(quotacmd_str, sizeof(quotacmd_str), "%u /* %s */", cmd, abbrev_str);
#else
	return abbrev_str;
#endif
	return quotacmd_str;
}

static const char *
gen_quotaid(const char *abbrev_str, const uint32_t id)
{
	static char quotaid_str[1024];

#if XLAT_RAW
	snprintf(quotaid_str, sizeof(quotaid_str), "%#x", id);
#elif XLAT_VERBOSE
	snprintf(quotaid_str, sizeof(quotaid_str), "%#x /* %s */", id, abbrev_str);
#else
	return abbrev_str;
#endif
	return quotaid_str;
}

int
main(void)
{
	char *bogus_special = (char *) tail_alloc(1) + 1;
	void *bogus_addr = (char *) tail_alloc(1) + 1;

	char bogus_special_str[sizeof(void *) * 2 + sizeof("0x")];
	char unterminated_str[sizeof(void *) * 2 + sizeof("0x")];

	static char invalid_cmd_str[1024];
	static char invalid_id_str[1024];
	char *unterminated = tail_memdup(unterminated_data,
					 sizeof(unterminated_data));
	TAIL_ALLOC_OBJECT_CONST_PTR(struct if_dqblk, dqblk);
	TAIL_ALLOC_OBJECT_CONST_PTR(struct if_dqinfo, dqinfo);
	TAIL_ALLOC_OBJECT_CONST_PTR(uint32_t, fmt);
	TAIL_ALLOC_OBJECT_CONST_PTR(struct if_nextdqblk, nextdqblk);

	snprintf(bogus_special_str, sizeof(bogus_special_str), "%p",
		bogus_special);
	snprintf(unterminated_str, sizeof(unterminated_str), "%p",
		unterminated);


	/* Invalid commands */
	snprintf(invalid_cmd_str, sizeof(invalid_cmd_str),
		 "QCMD(%#x /* Q_??? */, %#x /* ???QUOTA */)",
		 QCMD_CMD(bogus_cmd), QCMD_TYPE(bogus_cmd));
	check_quota(CQF_NONE, bogus_cmd, gen_quotacmd(invalid_cmd_str, bogus_cmd),
		    bogus_special, bogus_special_str, bogus_id, bogus_addr);

	snprintf(invalid_cmd_str, sizeof(invalid_cmd_str),
		 "QCMD(0 /* Q_??? */, USRQUOTA)");
	check_quota(CQF_ADDR_STR, 0, gen_quotacmd(invalid_cmd_str, 0),
		    ARG_STR(NULL), -1, ARG_STR(NULL));


	/* Q_QUOTAON */

	check_quota(CQF_ID_STR | CQF_ADDR_STR,
		    QUOTA_STR(QCMD(Q_QUOTAON, USRQUOTA)),
		    ARG_STR("/dev/bogus/"), QUOTA_ID_STR(QFMT_VFS_OLD),
		    ARG_STR("/tmp/bogus/"));

	snprintf(invalid_cmd_str, sizeof(invalid_cmd_str),
		 "QCMD(Q_QUOTAON, %#x /* ???QUOTA */)",
		 QCMD_TYPE(QCMD(Q_QUOTAON, 0xfacefeed)));
#if XLAT_RAW
	snprintf(invalid_id_str, sizeof(invalid_id_str),
		 "%#x", bogus_id);
#else
	snprintf(invalid_id_str, sizeof(invalid_id_str),
		 "%#x /* QFMT_VFS_??? */", bogus_id);
#endif
	check_quota(CQF_ID_STR, QCMD(Q_QUOTAON, 0xfacefeed),
		    gen_quotacmd(invalid_cmd_str, QCMD(Q_QUOTAON, 0xfacefeed)),
		    bogus_dev, bogus_dev_str,
		    bogus_id, invalid_id_str, bogus_addr);


	/* Q_QUOTAOFF */

	check_quota(CQF_ID_SKIP | CQF_ADDR_SKIP,
		    QUOTA_STR(QCMD(Q_QUOTAOFF, USRQUOTA)),
		    bogus_special, bogus_special_str);
	check_quota(CQF_ID_SKIP | CQF_ADDR_SKIP,
		    QUOTA_STR(QCMD(Q_QUOTAOFF, GRPQUOTA)),
		    ARG_STR("/dev/bogus/"));
	check_quota(CQF_ID_SKIP | CQF_ADDR_SKIP,
		    QUOTA_STR(QCMD(Q_QUOTAOFF, PRJQUOTA)), ARG_STR(NULL));
	const char *cmd_str = "QCMD(Q_QUOTAOFF, 0x3 /* ???QUOTA */)";
	check_quota(CQF_ID_SKIP | CQF_ADDR_SKIP,
		    QUOTA_STR_INVALID(QCMD(Q_QUOTAOFF, 3), cmd_str),
		    ARG_STR(NULL));


	/* Q_GETQUOTA */

	/* Trying our best to get successful result */
	check_quota(CQF_ADDR_CB, QUOTA_STR(QCMD(Q_GETQUOTA, USRQUOTA)),
		    ARG_STR("/dev/sda1"), getuid(), dqblk, print_dqblk,
		    (intptr_t) 1);

	check_quota(CQF_ADDR_CB, QUOTA_STR(QCMD(Q_GETQUOTA, GRPQUOTA)),
		    ARG_STR(NULL), -1, dqblk, print_dqblk, (intptr_t) 1);


	/* Q_GETNEXTQUOTA */

	check_quota(CQF_ADDR_CB, QUOTA_STR(QCMD(Q_GETNEXTQUOTA, USRQUOTA)),
		    ARG_STR("/dev/sda1"), 0, nextdqblk, print_nextdqblk,
		    (intptr_t) 1);


	/* Q_SETQUOTA */

	fill_memory(dqblk, sizeof(*dqblk));

	check_quota(CQF_NONE, QUOTA_STR(QCMD(Q_SETQUOTA, PRJQUOTA)),
		    bogus_special, bogus_special_str, 0, bogus_addr);

	check_quota(CQF_ADDR_CB, QUOTA_STR(QCMD(Q_SETQUOTA, PRJQUOTA)),
		    ARG_STR("/dev/bogus/"), 3141592653U, dqblk, print_dqblk,
		    (intptr_t) 0);


	/* Q_GETINFO */

	check_quota(CQF_ID_SKIP | CQF_ADDR_CB,
		    QUOTA_STR(QCMD(Q_GETINFO, GRPQUOTA)),
		    ARG_STR("/dev/sda1"), dqinfo, print_dqinfo, (intptr_t) 1);

	check_quota(CQF_ID_SKIP | CQF_ADDR_CB,
		    QUOTA_STR(QCMD(Q_GETINFO, GRPQUOTA)),
		    bogus_special, bogus_special_str, dqinfo,
		    print_dqinfo, (intptr_t) 1);

	/* Q_SETINFO */

	fill_memory(dqinfo, sizeof(*dqinfo));
	/* In order to check flag printing correctness */
	dqinfo->dqi_flags = 0xdeadabcd;

	check_quota(CQF_ID_SKIP | CQF_ADDR_STR,
		    QUOTA_STR(QCMD(Q_SETINFO, PRJQUOTA)),
		    bogus_special, bogus_special_str, ARG_STR(NULL));

	check_quota(CQF_ID_SKIP | CQF_ADDR_CB,
		    QUOTA_STR(QCMD(Q_SETINFO, USRQUOTA)),
		    ARG_STR("/dev/bogus/"), dqinfo, print_dqinfo, (intptr_t) 0);


	/* Q_GETFMT */

	check_quota(CQF_ID_SKIP | CQF_ADDR_STR,
		    QUOTA_STR(QCMD(Q_GETFMT, PRJQUOTA)),
		    bogus_special, bogus_special_str, ARG_STR(NULL));
	check_quota(CQF_ID_SKIP,
		    QUOTA_STR(QCMD(Q_GETFMT, USRQUOTA)),
		    unterminated, unterminated_str, fmt + 1);
	check_quota(CQF_ID_SKIP | CQF_ADDR_CB,
		    QUOTA_STR(QCMD(Q_GETFMT, GRPQUOTA)),
		    ARG_STR("/dev/sda1"), fmt, print_dqfmt, (uintptr_t) 1);
	/* Try to check valid quota format */
	*fmt = QFMT_VFS_OLD;
	check_quota(CQF_ID_SKIP | CQF_ADDR_CB,
		    QUOTA_STR(QCMD(Q_GETFMT, GRPQUOTA)),
		    ARG_STR("/dev/sda1"), fmt, print_dqfmt, (uintptr_t) 1);


	/* Q_SYNC */

	check_quota(CQF_ID_SKIP | CQF_ADDR_SKIP,
		    QUOTA_STR(QCMD(Q_SYNC, USRQUOTA)),
		    bogus_special, bogus_special_str);
	cmd_str = "QCMD(Q_SYNC, 0xff /* ???QUOTA */)";
	check_quota(CQF_ID_SKIP | CQF_ADDR_SKIP,
		    QUOTA_STR_INVALID(QCMD(Q_SYNC, 0xfff), cmd_str),
		    ARG_STR(NULL));

	puts("+++ exited with 0 +++");

	return 0;
}
