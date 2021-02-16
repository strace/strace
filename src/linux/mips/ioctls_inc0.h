/*
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef LINUX_MIPSN64
# include "../64/ioctls_inc.h"
#else
# include "../32/ioctls_inc.h"
#endif
