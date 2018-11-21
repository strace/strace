/*
 * Copyright (c) 2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#if SUPPORTED_PERSONALITIES > 1
# include "get_personality.h"
# include <linux/audit.h>
# include "arch_get_personality.c"
#endif
