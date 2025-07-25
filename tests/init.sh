#!/bin/sh
#
# Copyright (c) 2025 Dmitry V. Levin <ldv@strace.io>
# All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0-or-later

if [ -z "${TESTS_INIT_LOADED-}" ]; then
	TESTS_INIT_LOADED=1
	. "${srcdir=.}/init-once.sh"
fi
