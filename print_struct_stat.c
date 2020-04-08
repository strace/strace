/*
 * Copyright (c) 1999-2003 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2004 David S. Miller <davem@nuts.davemloft.net>
 * Copyright (c) 2003-2005 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2007 Jan Kratochvil <jan.kratochvil@redhat.com>
 * Copyright (c) 2009 Denys Vlasenko <dvlasenk@redhat.com>
 * Copyright (c) 2009-2010 Andreas Schwab <schwab@linux-m68k.org>
 * Copyright (c) 2012 H.J. Lu <hongjiu.lu@intel.com>
 * Copyright (c) 2005-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <sys/stat.h>
#include "stat.h"

void
print_struct_stat(struct tcb *tcp, const struct strace_stat *const st)
{
	tprints("{");
	if (!abbrev(tcp)) {
		tprints("st_dev=");
		print_dev_t(st->dev);
		tprintf(", st_ino=%llu, st_mode=", st->ino);
		print_symbolic_mode_t(st->mode);
		tprintf(", st_nlink=%llu, st_uid=%llu, st_gid=%llu",
			st->nlink, st->uid, st->gid);
		tprintf(", st_blksize=%llu", st->blksize);
		tprintf(", st_blocks=%llu", st->blocks);
	} else {
		tprints("st_mode=");
		print_symbolic_mode_t(st->mode);
	}

	switch (st->mode & S_IFMT) {
	case S_IFCHR: case S_IFBLK:
		tprints(", st_rdev=");
		print_dev_t(st->rdev);
		break;
	default:
		tprintf(", st_size=%llu", st->size);
		break;
	}

	if (!abbrev(tcp)) {
#define PRINT_ST_TIME(field)						\
	do {								\
		tprintf(", st_" #field "=%lld", (long long) st->field);	\
		tprints_comment(sprinttime_nsec(st->field,		\
			zero_extend_signed_to_ull(st->field ## _nsec)));\
		if (st->has_nsec)					\
			tprintf(", st_" #field "_nsec=%llu",		\
				zero_extend_signed_to_ull(		\
					st->field ## _nsec));		\
	} while (0)

		PRINT_ST_TIME(atime);
		PRINT_ST_TIME(mtime);
		PRINT_ST_TIME(ctime);
	} else {
		tprints(", ...");
	}
	tprints("}");
}
