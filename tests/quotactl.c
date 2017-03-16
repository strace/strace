/*
 * Check decoding of quotactl syscall.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
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

#include "tests.h"

#include <asm/unistd.h>

#if defined(__NR_quotactl) && \
	(defined(HAVE_LINUX_QUOTA_H) || defined(HAVE_SYS_QUOTA_H))

# include <inttypes.h>
# include <stdint.h>
# include <stdio.h>
# include <string.h>
# include <unistd.h>

# include "quotactl.h"

# ifndef HAVE_LINUX_QUOTA_H
/* Some dirty hacks in order to make sys/quota.h usable as a backup */

#  define if_dqblk dqblk
#  define if_nextdqblk nextdqblk
#  define if_dqinfo dqinfo

# endif /* !HAVE_LINUX_QUOTA_H */

# ifndef Q_GETNEXTQUOTA

#  define Q_GETNEXTQUOTA 0x800009

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
# endif /* !Q_GETNEXTQUOTA */

# include "xlat.h"
# include "xlat/quota_formats.h"
# include "xlat/if_dqblk_valid.h"
# include "xlat/if_dqinfo_flags.h"
# include "xlat/if_dqinfo_valid.h"

void
print_dqblk(long rc, void *ptr, void *arg)
{
	struct if_dqblk *db = ptr;
	long out_arg = (long) arg;

	if (((rc != 0) && out_arg) || (out_arg > 1)) {
		printf("%p", db);
		return;
	}

	PRINT_FIELD_U("{", db, dqb_bhardlimit);
	PRINT_FIELD_U(", ", db, dqb_bsoftlimit);
	PRINT_FIELD_U(", ", db, dqb_curspace);
	PRINT_FIELD_U(", ", db, dqb_ihardlimit);
	PRINT_FIELD_U(", ", db, dqb_isoftlimit);
	PRINT_FIELD_U(", ", db, dqb_curinodes);

# if VERBOSE
	PRINT_FIELD_U(", ", db, dqb_btime);
	PRINT_FIELD_U(", ", db, dqb_itime);

	printf(", dqb_valid=");
	printflags(if_dqblk_valid, db->dqb_valid, "QIF_???");
# else
	printf(", ...");
# endif /* !VERBOSE */
	printf("}");
}

void
print_nextdqblk(long rc, void *ptr, void *arg)
{
	struct if_nextdqblk *db = ptr;
	long out_arg = (long) arg;

	if (((rc != 0) && out_arg) || (out_arg > 1)) {
		printf("%p", db);
		return;
	}

	PRINT_FIELD_U("{", db, dqb_bhardlimit);
	PRINT_FIELD_U(", ", db, dqb_bsoftlimit);
	PRINT_FIELD_U(", ", db, dqb_curspace);
	PRINT_FIELD_U(", ", db, dqb_ihardlimit);
	PRINT_FIELD_U(", ", db, dqb_isoftlimit);
	PRINT_FIELD_U(", ", db, dqb_curinodes);

# if VERBOSE
	PRINT_FIELD_U(", ", db, dqb_btime);
	PRINT_FIELD_U(", ", db, dqb_itime);

	printf(", dqb_valid=");
	printflags(if_dqblk_valid, db->dqb_valid, "QIF_???");

	PRINT_FIELD_U(", ", db, dqb_id);
# else
	PRINT_FIELD_U(", ", db, dqb_id);
	printf(", ...");
# endif /* !VERBOSE */
	printf("}");
}

void
print_dqinfo(long rc, void *ptr, void *arg)
{
	struct if_dqinfo *di = ptr;
	long out_arg = (long) arg;

	if (((rc != 0) && out_arg) || (out_arg > 1)) {
		printf("%p", di);
		return;
	}

	PRINT_FIELD_U("{", di, dqi_bgrace);
	PRINT_FIELD_U(", ", di, dqi_igrace);

	printf(", dqi_flags=");
	printflags(if_dqinfo_flags, di->dqi_flags, "DQF_???");
	printf(", dqi_valid=");
	printflags(if_dqinfo_valid, di->dqi_valid, "IIF_???");
	printf("}");
}


int
main(void)
{
	char *bogus_special = (char *) tail_alloc(1) + 1;
	void *bogus_addr = (char *) tail_alloc(1) + 1;

	char bogus_special_str[sizeof(void *) * 2 + sizeof("0x")];
	char unterminated_str[sizeof(void *) * 2 + sizeof("0x")];

	long rc;
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

	rc = syscall(__NR_quotactl, bogus_cmd, bogus_special, bogus_id,
		     bogus_addr);
	printf("quotactl(QCMD(%#x /* Q_??? */, %#x /* ???QUOTA */)"
	       ", %p, %u, %p) = %s\n",
	       QCMD_CMD(bogus_cmd), QCMD_TYPE(bogus_cmd),
	       bogus_special, bogus_id, bogus_addr, sprintrc(rc));

	rc = syscall(__NR_quotactl, 0, NULL, -1, NULL);
	printf("quotactl(QCMD(0 /* Q_??? */, USRQUOTA), NULL, -1, NULL) = %s\n",
	       sprintrc(rc));


	/* Q_QUOTAON */

	check_quota(CQF_ID_STR | CQF_ADDR_STR,
		    ARG_STR(QCMD(Q_QUOTAON, USRQUOTA)),
		    ARG_STR("/dev/bogus/"), ARG_STR(QFMT_VFS_OLD),
		    ARG_STR("/tmp/bogus/"));

	rc = syscall(__NR_quotactl, QCMD(Q_QUOTAON, 0xfacefeed), bogus_dev,
		     bogus_id, bogus_addr);
	printf("quotactl(QCMD(Q_QUOTAON, %#x /* ???QUOTA */)"
	       ", %s, %#x /* QFMT_VFS_??? */, %p) = %s\n",
	       QCMD_TYPE(QCMD(Q_QUOTAON, 0xfacefeed)),
	       bogus_dev_str, bogus_id, bogus_addr, sprintrc(rc));


	/* Q_QUOTAOFF */

	check_quota(CQF_ID_SKIP | CQF_ADDR_SKIP,
		    ARG_STR(QCMD(Q_QUOTAOFF, USRQUOTA)),
		    bogus_special, bogus_special_str);
	check_quota(CQF_ID_SKIP | CQF_ADDR_SKIP,
		    ARG_STR(QCMD(Q_QUOTAOFF, GRPQUOTA)),
		    ARG_STR("/dev/bogus/"));
	check_quota(CQF_ID_SKIP | CQF_ADDR_SKIP,
		    ARG_STR(QCMD(Q_QUOTAOFF, PRJQUOTA)), ARG_STR(NULL));
	check_quota(CQF_ID_SKIP | CQF_ADDR_SKIP,
		    QCMD(Q_QUOTAOFF, 3), "QCMD(Q_QUOTAOFF, 0x3 /* ???QUOTA */)",
		    ARG_STR(NULL));


	/* Q_GETQUOTA */

	/* Trying our best to get successful result */
	check_quota(CQF_ADDR_CB, ARG_STR(QCMD(Q_GETQUOTA, USRQUOTA)),
		    ARG_STR("/dev/sda1"), getuid(), dqblk, print_dqblk,
		    (intptr_t) 1);

	check_quota(CQF_ADDR_CB, ARG_STR(QCMD(Q_GETQUOTA, GRPQUOTA)),
		    ARG_STR(NULL), -1, dqblk, print_dqblk, (intptr_t) 2);


	/* Q_GETNEXTQUOTA */

	check_quota(CQF_ADDR_CB, ARG_STR(QCMD(Q_GETNEXTQUOTA, USRQUOTA)),
		    ARG_STR("/dev/sda1"), 0, nextdqblk, print_nextdqblk,
		    (intptr_t) 1);


	/* Q_SETQUOTA */

	fill_memory(dqblk, sizeof(*dqblk));

	check_quota(CQF_NONE, ARG_STR(QCMD(Q_SETQUOTA, PRJQUOTA)),
		    bogus_special, bogus_special_str, 0, bogus_addr);

	check_quota(CQF_ADDR_CB, ARG_STR(QCMD(Q_SETQUOTA, PRJQUOTA)),
		    ARG_STR("/dev/bogus/"), 3141592653U, dqblk, print_dqblk,
		    (intptr_t) 0);


	/* Q_GETINFO */

	check_quota(CQF_ID_SKIP | CQF_ADDR_CB,
		    ARG_STR(QCMD(Q_GETINFO, GRPQUOTA)),
		    ARG_STR("/dev/sda1"), dqinfo, print_dqinfo, (intptr_t) 1);

	check_quota(CQF_ID_SKIP | CQF_ADDR_CB,
		    ARG_STR(QCMD(Q_GETINFO, GRPQUOTA)),
		    bogus_special, bogus_special_str, dqinfo,
		    print_dqinfo, (intptr_t) 2);

	/* Q_SETINFO */

	fill_memory(dqinfo, sizeof(*dqinfo));
	/* In order to check flag printing correctness */
	dqinfo->dqi_flags = 0xdeadabcd;

	check_quota(CQF_ID_SKIP | CQF_ADDR_STR,
		    ARG_STR(QCMD(Q_SETINFO, PRJQUOTA)),
		    bogus_special, bogus_special_str, ARG_STR(NULL));

	check_quota(CQF_ID_SKIP | CQF_ADDR_CB,
		    ARG_STR(QCMD(Q_SETINFO, USRQUOTA)),
		    ARG_STR("/dev/bogus/"), dqinfo, print_dqinfo, (intptr_t) 0);


	/* Q_GETFMT */

	check_quota(CQF_ID_SKIP | CQF_ADDR_STR,
		    ARG_STR(QCMD(Q_GETFMT, PRJQUOTA)),
		    bogus_special, bogus_special_str, ARG_STR(NULL));
	check_quota(CQF_ID_SKIP,
		    ARG_STR(QCMD(Q_GETFMT, USRQUOTA)),
		    unterminated, unterminated_str, fmt + 1);
	check_quota(CQF_ID_SKIP,
		    ARG_STR(QCMD(Q_GETFMT, GRPQUOTA)),
		    ARG_STR("/dev/sda1"), fmt);


	/* Q_SYNC */

	check_quota(CQF_ID_SKIP | CQF_ADDR_SKIP,
		    ARG_STR(QCMD(Q_SYNC, USRQUOTA)),
		    bogus_special, bogus_special_str);
	check_quota(CQF_ID_SKIP | CQF_ADDR_SKIP,
		    QCMD(Q_SYNC, 0xfff), "QCMD(Q_SYNC, 0xff /* ???QUOTA */)",
		    ARG_STR(NULL));

	puts("+++ exited with 0 +++");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_quotactl && "
	"(HAVE_LINUX_QUOTA_H || HAVE_SYS_QUOTA_H)");

#endif
