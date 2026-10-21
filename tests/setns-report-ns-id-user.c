/*
 * Check setns(2) with zero nstype for a user namespace descriptor.
 *
 * Copyright (c) 2026 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#define TEST_NS_FLAG 0
#define TEST_NS_FLAG_STR "0"
#include "setns-report-ns-id-user-common.c"
