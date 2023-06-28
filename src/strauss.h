/*
 * Strauss awareness interface declarations.
 *
 * Copyright (c) 2018-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_STRAUSS_H
# define STRACE_STRAUSS_H

enum { STRAUSS_START_VERBOSITY = 5 };


enum tips_fmt {
	TIPS_NONE,
	TIPS_COMPACT,
	TIPS_FULL,
};

enum tip_ids {
	TIP_ID_RANDOM = -1,
};

extern const size_t strauss_lines;
extern enum tips_fmt show_tips;
extern int tip_id;

extern void print_strauss(size_t verbosity);
extern void print_totd(void);

#endif /* STRACE_STRAUSS_H */
