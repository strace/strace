/*
 * Check setns(2) with CLONE_NEWUSER for a user namespace descriptor.
 *
 * Copyright (c) 2026 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#define TEST_NS_FLAG CLONE_NEWUSER
#define TEST_NS_FLAG_STR "CLONE_NEWUSER"
#include "setns-report-ns-id-user-common.c"
