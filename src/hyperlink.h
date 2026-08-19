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

extern bool hyperlink_is_enabled;

char *make_hyperlink_url_for_syscall(const char *name);
void tprint_hyperlink_begin(const char *url);
void tprint_hyperlink_end(void);

#endif /* STRACE_HYPERLINK_H */
