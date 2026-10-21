/*
 * Check setns(2) namespace ID for a mount namespace descriptor.
 *
 * Copyright (c) 2026 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#define TEST_NS_NAME "mnt"
#define TEST_NS_FLAG CLONE_NEWNS
#define TEST_NS_FLAG_STR "CLONE_NEWNS"
#include "setns-report-ns-id.c"
