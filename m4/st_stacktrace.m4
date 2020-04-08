#!/usr/bin/m4
#
# Copyright (c) 2018 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: LGPL-2.1-or-later

AC_DEFUN([st_STACKTRACE], [dnl

AC_ARG_ENABLE([stacktrace],
	[AS_HELP_STRING([--enable-stacktrace=yes|no|check],
		[whether to enable stack tracing support, default is check])],
	[case "$enableval" in
		yes|no|check) enable_stacktrace="$enableval" ;;
		*) AC_MSG_ERROR([bad value $enableval for enable-stacktrace option.
				 Valid options are: yes, no, check.])
		;;
	 esac],
	[enable_stacktrace=check])

use_unwinder=
dnl Whether to enable stack tracing support?

AS_IF([test x"$enable_stacktrace" != xno],
      [st_ARG_LIBDW
       st_ARG_LIBUNWIND
       AS_IF([test "x$with_libdw" = xyes && test "x$with_libunwind" = xyes],
	     [AC_MSG_ERROR([--with-libdw=yes and --with-libunwind=yes are mutually exclusive])],
	     [test "x$with_libdw" = xyes], [with_libunwind=no],
	     [test "x$with_libunwind" = xyes], [with_libdw=no]
	    )
       AS_IF([test "x$use_unwinder" = x], [st_LIBDW])
       AS_IF([test "x$use_unwinder" = x], [st_LIBUNWIND])
       AS_IF([test x"$enable_stacktrace$use_unwinder" = xyes],
	     [AC_MSG_ERROR([stack tracing support requires an unwinder])]
	    )
      ]
     )

AC_MSG_CHECKING([whether to enable stack tracing support])
AM_CONDITIONAL([ENABLE_STACKTRACE], [test "x$use_unwinder" != x])
AM_CONDITIONAL([USE_LIBDW], [test "x$use_unwinder" = xlibdw])
AM_CONDITIONAL([USE_LIBUNWIND], [test "x$use_unwinder" = xlibunwind])

use_libiberty=
AS_IF([test "x$use_unwinder" != x],
      [AC_DEFINE([ENABLE_STACKTRACE], [1],
			  [Define to enable stack tracing support])
       AC_DEFINE_UNQUOTED([USE_UNWINDER], ["$use_unwinder"],
			  [The unwinder to use for stack tracing support])
       AC_MSG_RESULT([yes, using $use_unwinder])
       dnl As stack tracing support is enabled, check for a demangler.
       st_DEMANGLE],
      [AC_MSG_RESULT([no])])

AM_CONDITIONAL([USE_DEMANGLE], [test "x$use_libiberty" = xyes])

])
