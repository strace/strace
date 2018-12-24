/*
 * Copyright (c) 2017-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <stdbool.h>

extern bool
test_process_vm_readv(void);

extern bool
test_ptrace_peekdata(void);

extern void
test_printpath(unsigned int test_max_size);

extern void
test_printstrn(unsigned int test_max_size);
