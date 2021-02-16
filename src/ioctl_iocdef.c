/*
 * Copyright (c) 2017 Alexey Neyman <stilor@att.net>
 * Copyright (c) 2017-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/*
 * This file is *PREPROCESSED*, not *COMPILED* for host and the result
 * is included into ioctlsort (which is compiled for build). Since some
 * of these values are used in structure initializers, they cannot be
 * defined as 'const unsigned int' - instead, they have to be macros.
 * Hence, the result of preprocessing will be run through sed to change
 * 'DEFINE' into '#define'
 */
#include <linux/ioctl.h>

DEFINE HOST_IOC_NONE _IOC_NONE
DEFINE HOST_IOC_READ _IOC_READ
DEFINE HOST_IOC_WRITE _IOC_WRITE

DEFINE HOST_IOC_SIZESHIFT _IOC_SIZESHIFT
DEFINE HOST_IOC_DIRSHIFT _IOC_DIRSHIFT
