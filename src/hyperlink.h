/*
 * Hyperlink support.
 *
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_HYPERLINK_H
# define STRACE_HYPERLINK_H

# include <stdbool.h>
# include <stdio.h>

enum hyperlink_mode_t {
	HYPERLINK_NEVER,
	HYPERLINK_AUTO,
	HYPERLINK_ALWAYS,
};

extern enum hyperlink_mode_t hyperlink_mode;

void hyperlink_init(int out_fd, bool output_separately);
char *make_hyperlink_url_for_syscall(const char *name);
void tprint_hyperlink_begin(const char *url);
void tprint_hyperlink_end(void);
void fprint_hyperlink_begin(FILE *outf, const char *url);
void fprint_hyperlink_end(FILE *outf);

#endif /* STRACE_HYPERLINK_H */
