#!/usr/bin/m4
#
# Copyright (c) 2018 The strace developers.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
