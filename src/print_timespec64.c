/*
 * Copyright (c) 2019-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#define TIMESPEC_T kernel_timespec64_t
#define PRINT_TIMESPEC_DATA_SIZE print_timespec64_data_size
#define PRINT_TIMESPEC_ARRAY_DATA_SIZE print_timespec64_array_data_size
#define PRINT_TIMESPEC print_timespec64
#define SPRINT_TIMESPEC sprint_timespec64
#define PRINT_TIMESPEC_UTIME_PAIR print_timespec64_utime_pair
#define PRINT_ITIMERSPEC print_itimerspec64

#include "kernel_timespec.h"
#include "print_timespec.h"
