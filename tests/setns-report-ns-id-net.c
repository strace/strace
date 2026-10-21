/*
 * Check setns(2) namespace ID for a network namespace descriptor.
 *
 * Copyright (c) 2026 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#define TEST_NS_NAME "net"
#define TEST_NS_FLAG CLONE_NEWNET
#define TEST_NS_FLAG_STR "CLONE_NEWNET"
#include "setns-report-ns-id.c"
