/*
 * Copyright (c) 2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* retval to index and visa versa.  */
#ifndef STRACE_RETVAL_H
# define STRACE_RETVAL_H

uint16_t retval_new(kernel_long_t rval);
kernel_long_t retval_get(uint16_t rval_idx);

#endif /* !STRACE_RETVAL_H */
