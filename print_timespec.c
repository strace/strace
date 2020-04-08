/*
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include DEF_MPERS_TYPE(timespec_t)

#include "kernel_timespec.h"

#if defined MPERS_IS_mx32
# define TIMESPEC_IS_32BIT 0
#elif defined MPERS_IS_m32
# define TIMESPEC_IS_32BIT 1
#elif ARCH_TIMESIZE == 4
# define TIMESPEC_IS_32BIT 1
#else
# define TIMESPEC_IS_32BIT 0
#endif

#if TIMESPEC_IS_32BIT
typedef kernel_timespec32_t timespec_t;
# define PRINT_TIMESPEC_DATA_SIZE print_timespec32_data_size
# define PRINT_TIMESPEC_ARRAY_DATA_SIZE print_timespec32_array_data_size
#else
typedef kernel_timespec64_t timespec_t;
# define PRINT_TIMESPEC_DATA_SIZE print_timespec64_data_size
# define PRINT_TIMESPEC_ARRAY_DATA_SIZE print_timespec64_array_data_size
#endif

#include MPERS_DEFS

MPERS_PRINTER_DECL(bool, print_struct_timespec_data_size,
		   const void *arg, const size_t size)
{
	return PRINT_TIMESPEC_DATA_SIZE(arg, size);
}

MPERS_PRINTER_DECL(bool, print_struct_timespec_array_data_size,
		   const void *arg, const unsigned int nmemb,
		   const size_t size)
{
	return PRINT_TIMESPEC_ARRAY_DATA_SIZE(arg, nmemb, size);
}
