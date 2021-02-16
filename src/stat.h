/*
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_STAT_H
# define STRACE_STAT_H

struct strace_stat {
	unsigned long long	dev;
	unsigned long long	ino;
	unsigned long long	rdev;
	unsigned long long	size;
	unsigned long long	blocks;
	unsigned long long	blksize;
	unsigned long long	mode;
	unsigned long long	nlink;
	unsigned long long	uid;
	unsigned long long	gid;
	long long		atime;
	long long		ctime;
	long long		mtime;
	unsigned long long	atime_nsec;
	unsigned long long	ctime_nsec;
	unsigned long long	mtime_nsec;
	bool			has_nsec;
};

#endif /* !STRACE_STAT_H */
