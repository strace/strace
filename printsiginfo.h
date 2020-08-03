/*
 * Copyright (c) 2015-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_PRINTSIGINFO_H
# define STRACE_PRINTSIGINFO_H

extern void printsiginfo(struct tcb *, const siginfo_t *);

#endif /* !STRACE_PRINTSIGINFO_H */
