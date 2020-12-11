#!/usr/bin/m4
#
# Copyright (c) 2020 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: LGPL-2.1-or-later

AC_DEFUN([st_BROKEN_CC], [dnl
  AC_CACHE_CHECK([whether $[]_AC_CC[] is broken],
    [st_cv_broken_cc],
    [saved_CFLAGS="$CFLAGS"
    CFLAGS="$CFLAGS $WARN_CFLAGS"
    AC_COMPILE_IFELSE([dnl
      AC_LANG_SOURCE(dnl
        [[#include <stdlib.h>
          char *f(char c) {
            struct { char c, z; } *p = malloc(sizeof(p->c));
            p->c = c;
            return &p->c;
          }
        ]]
      )],
      [st_cv_broken_cc=no],
      [st_cv_broken_cc=yes])
    CFLAGS="$saved_CFLAGS"
    ])
  AS_IF([test x"$st_cv_broken_cc" = xyes], [dnl
        AC_DEFINE([HAVE_BROKEN_CC], [1],
          [Define to 1 if the C compiler is broken])
        cc_version="$(${_AC_CC} --version |head -1)"
        AC_MSG_WARN([The C compiler is: $cc_version])
        AC_MSG_WARN([This C compiler is broken, use it at your own risk!])])
])
