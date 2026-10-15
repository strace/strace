/*
 * Check setns(2) namespace IDs for future children in a PID namespace.
 * Keep this case separate so unavailable PID namespaces produce a skip.
 *
 * Copyright (c) 2026 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#define TEST_NS_PID
#include "setns-report-ns-id-children.c"
