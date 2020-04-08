/*
 * Copyright (c) 2019 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#define TIMESPEC_T kernel_timeval64_t
#define TIMESPEC_NSEC tv_usec
#define PRINT_TIMESPEC_DATA_SIZE print_timeval64_data_size

#include "kernel_timeval.h"
#include "print_timespec.h"
