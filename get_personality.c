/*
 * Copyright (c) 2018 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2018-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#if SUPPORTED_PERSONALITIES > 1
# include "get_personality.h"
# include <linux/audit.h>
# define XLAT_MACROS_ONLY
#  include "xlat/elf_em.h"
#  include "xlat/audit_arch.h"
# undef XLAT_MACROS_ONLY
# include "arch_get_personality.c"
#endif
