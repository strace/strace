/*
 * Copyright (c) 2026 Jonas Jelten <jj@sft.lol>
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_COLOR_H
# define STRACE_COLOR_H

# include <stdbool.h>

enum color_mode_t {
	COLOR_NEVER,
	COLOR_AUTO,
	COLOR_ALWAYS,
};

enum color_kind_t {
	COLOR_ARGNAME,
	COLOR_ARGVAL,
	COLOR_COMMENT,
	COLOR_CONST,
	COLOR_ERROR,
	COLOR_PUNCT,
	COLOR_RETVAL,
	COLOR_SYSCALL,

	COLOR_RESET,
	COLOR_KIND_MAX
};

extern enum color_mode_t color_mode;
extern const char *color_seq_table[COLOR_KIND_MAX];
extern bool color_is_enabled;

void color_init(int out_fd, bool output_separately);

static inline void
tprint_color_seq(enum color_kind_t kind)
{
	if (color_is_enabled)
		tprints_string_uncol(color_seq_table[kind]);
}

#endif /* STRACE_COLOR_H */
