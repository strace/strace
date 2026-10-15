/*
 * Check setns(2) namespace IDs for future children in a time namespace.
 * Keep this case separate so unavailable time namespaces produce a skip.
 *
 * Copyright (c) 2026 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#define TEST_NS_TIME
#include "setns-report-ns-id-children.c"
