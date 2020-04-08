#!/usr/bin/m4
#
# Copyright (c) 2016-2018 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: LGPL-2.1-or-later

AC_DEFUN([st_SAVE_VAR], [dnl
AS_IF([test -n "${$1+set}"], [st_saved_$1="${$1}"; unset $1])
])

AC_DEFUN([st_RESTORE_VAR], [dnl
AS_IF([test -n "${st_saved_$1+set}"], [$1="${st_saved_$1}"; unset st_saved_$1])
])
