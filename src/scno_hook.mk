# scno.h make hooks for scno.am
#
# Copyright (c) 2026 Dmitry V. Levin <ldv@strace.io>
# All rights reserved.
#
# SPDX-License-Identifier: LGPL-2.1-or-later

# Filter out anything containing "clean" or starting with "dist".
ifeq ($(filter %clean% dist%,$(MAKECMDGOALS)),)
Makefile: scno.h
endif
