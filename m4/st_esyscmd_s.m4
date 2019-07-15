#!/usr/bin/m4
#
# Copyright (c) 2019 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: LGPL-2.1-or-later

AC_DEFUN([st_esyscmd_s], [dnl
m4_esyscmd_s([$1])dnl
m4_assert(m4_sysval == 0)])
