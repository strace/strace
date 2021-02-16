/*
 * Copyright (c) 2016-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_NSIG_H
# define STRACE_NSIG_H

# include <signal.h>

# ifndef NSIG
#  warning NSIG is not defined, using 32
#  define NSIG  32
# elif NSIG < 32
#  error NSIG < 32
# endif

# define NSIG_BYTES	(NSIG / 8)

#endif /* !STRACE_NSIG_H */
