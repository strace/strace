/*
 * Copyright (c) 2018-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "syscallent_base_nr.h"

#define HAVE_ARCH_GETRVAL2 1
#define HAVE_ARCH_UID16_SYSCALLS 1
#define HAVE_ARCH_SA_RESTORER 0
#define HAVE_ARCH_DEDICATED_ERR_REG 1
#define PERSONALITY0_AUDIT_ARCH { AUDIT_ARCH_IA64, SYSCALLENT_BASE_NR }
