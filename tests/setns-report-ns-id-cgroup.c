/*
 * Check setns(2) namespace ID for a cgroup namespace descriptor.
 *
 * Copyright (c) 2026 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#define TEST_NS_NAME "cgroup"
#define TEST_NS_FLAG CLONE_NEWCGROUP
#define TEST_NS_FLAG_STR "CLONE_NEWCGROUP"
#include "setns-report-ns-id.c"
