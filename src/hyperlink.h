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

/*
 * Terminal hyperlinks using OSC 8 escape sequences:
 * OSC 8 ; params ; URI ST text OSC 8 ;; ST
 */
# define HYPERLINK_FMT	"\033]8;;%s\033\\" "%s" "\033]8;;\033\\"

extern bool hyperlink_is_enabled;

char *make_hyperlink_url_for_syscall(const char *name);

#endif /* STRACE_HYPERLINK_H */
