/*
 * Copyright (c) 2026 Jonas Jelten <jj@sft.lol>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_COLOR_H
# define STRACE_COLOR_H

#ifdef ENABLE_COLORS
# include <stdbool.h>
# include <stdio.h>

enum color_mode_t {
	COLOR_NEVER,
	COLOR_AUTO,
	COLOR_ALWAYS,
};

enum color_kind_t {
	COLOR_SYSCALL,
	COLOR_ARGNAME,
	COLOR_ARGVAL,
	COLOR_CONST,
	COLOR_COMMENT,
	COLOR_PUNCT,
	COLOR_RETVAL,
	COLOR_ERROR,
	COLOR_RESET,
	COLOR_KIND_MAX
};

extern enum color_mode_t color_mode;
extern const char *color_seq_table[COLOR_KIND_MAX];
extern bool color_is_enabled;

void color_init(FILE *outf, bool output_separately);

/*
 * if colors are disabled, return empty string. 
 * otherwise, look up the coloring sequence.
 */
static inline const char *
color_seq(enum color_kind_t kind)
{
	return color_is_enabled ? color_seq_table[kind] : "";
}
#endif

#endif /* STRACE_COLOR_H */
