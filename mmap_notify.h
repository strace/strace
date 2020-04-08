/*
 * Copyright (c) 2018 The strace developers.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_MMAP_NOTIFY_H
# define STRACE_MMAP_NOTIFY_H

# include "defs.h"

typedef void (*mmap_notify_fn)(struct tcb *, void *);

extern void
mmap_notify_register_client(mmap_notify_fn, void *);

extern void
mmap_notify_report (struct tcb *);

#endif /* !STRACE_MMAP_NOTIFY_H */
