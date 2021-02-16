/*
 * Copyright (c) 2004-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_WAIT_H
# define STRACE_WAIT_H

# include "defs.h"

# include <sys/wait.h>

# include "static_assert.h"

/*
 * On Linux, the "core dumped" flag is hard-coded to 0x80:
 * fs/coredump.c:coredump_finish() after v3.10-rc1~143^2~41,
 * fs/coredump.c:do_coredump() between v3.7-rc1~134^2~4 and v3.10-rc1~143^2~41,
 * or fs/exec.c:do_coredump() before v3.7-rc1~134^2~4
 */
# ifndef WCOREFLAG
#  define WCOREFLAG 0x80
# else
static_assert((WCOREFLAG) == 0x80, "WCOREFLAG != 0x80");
# endif
# ifndef WCOREDUMP
#  define WCOREDUMP(status) ((status) & (WCOREFLAG))
# endif

# ifndef W_STOPCODE
#  define W_STOPCODE(sig)  ((sig) << 8 | 0x7f)
# endif
# ifndef W_EXITCODE
#  define W_EXITCODE(ret, sig)  ((ret) << 8 | (sig))
# endif
# ifndef W_CONTINUED
#  define W_CONTINUED 0xffff
# endif

#endif /* STRACE_WAIT_H */
