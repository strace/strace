/*
 * Copyright (c) 1999-2003 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2004 David S. Miller <davem@nuts.davemloft.net>
 * Copyright (c) 2003-2005 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2007 Jan Kratochvil <jan.kratochvil@redhat.com>
 * Copyright (c) 2009 Denys Vlasenko <dvlasenk@redhat.com>
 * Copyright (c) 2009-2010 Andreas Schwab <schwab@linux-m68k.org>
 * Copyright (c) 2012 H.J. Lu <hongjiu.lu@intel.com>
 * Copyright (c) 2005-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <sys/stat.h>
#include "stat.h"

/*
 * This series of #undef/#define was produced by the following script:
 * sed -n 's/.*[[:space:]]\([[:alpha:]_]\+\);$/\1/p' stat.h |
 * while read n; do
 * 	printf '#undef st_%s\n#define st_%s %s\n\n' "$n" "$n" "$n"
 * done
 */

#undef st_dev
#define st_dev dev

#undef st_ino
#define st_ino ino

#undef st_rdev
#define st_rdev rdev

#undef st_size
#define st_size size

#undef st_blocks
#define st_blocks blocks

#undef st_blksize
#define st_blksize blksize

#undef st_mode
#define st_mode mode

#undef st_nlink
#define st_nlink nlink

#undef st_uid
#define st_uid uid

#undef st_gid
#define st_gid gid

#undef st_atime
#define st_atime atime

#undef st_ctime
#define st_ctime ctime

#undef st_mtime
#define st_mtime mtime

#undef st_atime_nsec
#define st_atime_nsec atime_nsec

#undef st_ctime_nsec
#define st_ctime_nsec ctime_nsec

#undef st_mtime_nsec
#define st_mtime_nsec mtime_nsec

void
print_struct_stat(struct tcb *tcp, const struct strace_stat *const st)
{
	tprint_struct_begin();
	if (!abbrev(tcp)) {
		PRINT_FIELD_DEV(*st, st_dev);
		tprint_struct_next();
		PRINT_FIELD_U(*st, st_ino);
		tprint_struct_next();
		PRINT_FIELD_OBJ_VAL(*st, st_mode, print_symbolic_mode_t);
		tprint_struct_next();
		PRINT_FIELD_U(*st, st_nlink);
		tprint_struct_next();
		PRINT_FIELD_U(*st, st_uid);
		tprint_struct_next();
		PRINT_FIELD_U(*st, st_gid);
		tprint_struct_next();
		PRINT_FIELD_U(*st, st_blksize);
		tprint_struct_next();
		PRINT_FIELD_U(*st, st_blocks);
	} else {
		PRINT_FIELD_OBJ_VAL(*st, st_mode, print_symbolic_mode_t);
	}

	switch (st->st_mode & S_IFMT) {
	case S_IFCHR: case S_IFBLK:
		tprint_struct_next();
		PRINT_FIELD_DEV(*st, st_rdev);
		break;
	default:
		tprint_struct_next();
		PRINT_FIELD_U(*st, st_size);
		break;
	}

	if (!abbrev(tcp)) {
		tprint_struct_next();
		PRINT_FIELD_D(*st, st_atime);
		tprints_comment(sprinttime_nsec(st->st_atime,
						st->st_atime_nsec));
		if (st->has_nsec) {
			tprint_struct_next();
			PRINT_FIELD_U(*st, st_atime_nsec);
		}

		tprint_struct_next();
		PRINT_FIELD_D(*st, st_mtime);
		tprints_comment(sprinttime_nsec(st->st_mtime,
						st->st_mtime_nsec));
		if (st->has_nsec) {
			tprint_struct_next();
			PRINT_FIELD_U(*st, st_mtime_nsec);
		}

		tprint_struct_next();
		PRINT_FIELD_D(*st, st_ctime);
		tprints_comment(sprinttime_nsec(st->st_ctime,
						st->st_ctime_nsec));
		if (st->has_nsec) {
			tprint_struct_next();
			PRINT_FIELD_U(*st, st_ctime_nsec);
		}
	} else {
		tprint_struct_next();
		tprint_more_data_follows();
	}
	tprint_struct_end();
}
