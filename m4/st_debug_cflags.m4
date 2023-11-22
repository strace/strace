#!/usr/bin/m4
#
# Copyright (c) 2023 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: LGPL-2.1-or-later

AC_DEFUN([st_DEBUG_CFLAGS],[dnl
gl_COMPILER_OPTION_IF([-gdwarf-aranges],
                      [accept_dwarf_aranges_option=yes],
                      [accept_dwarf_aranges_option=no])
AM_CONDITIONAL([ACCEPT_GDWARF_ARANGES],
               [test ${accept_dwarf_aranges_option} = yes])
])
