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

AC_DEFUN([st_ARG_LIBDW], [dnl

AC_ARG_WITH([libdw],
	    [AS_HELP_STRING([--with-libdw],
			    [use libdw to implement stack tracing support]
			   )
	    ],
	    [case "${withval}" in
	     yes|no|check) ;;
	     *)
	     AC_MSG_ERROR([Use pkg-config variables instead of giving path to --with-libdw])
	     ;;
	     esac
	    ],
	    [with_libdw=check]
	   )

])

AC_DEFUN([st_LIBDW], [dnl

: ${libdw_CPPFLAGS=}
: ${libdw_CFLAGS=}
: ${libdw_LDFLAGS=}
: ${libdw_LIBS=}
have_libdw=

AS_IF([test "x$with_libdw" != xno],
      [
       dnl If libdw.pc is not available, then libdw is not new enough
       dnl to be used for stack tracing.
       AS_IF([test "x$with_libdw" = xyes],
	     [PKG_CHECK_MODULES([libdw], [libdw], [have_libdw=yes])],
	     [PKG_CHECK_MODULES([libdw], [libdw], [have_libdw=yes], [:])]
	    )
      ]
     )

AS_IF([test "x$have_libdw" = xyes],
      [
       dnl If libdw.pc is available, check whether libdw can be used
       dnl for stack tracing.
       saved_CPPFLAGS="$CPPFLAGS"
       saved_CFLAGS="$CFLAGS"
       CPPFLAGS="$CPPFLAGS $libdw_CPPFLAGS"
       CFLAGS="$CFLAGS $libdw_CFLAGS"

       AC_CHECK_HEADERS([elfutils/libdwfl.h],
			[
			 AC_MSG_CHECKING([for dwfl_linux_proc_attach in libdw])
			 saved_LDFLAGS="$LDFLAGS"
			 saved_LIBS="$LIBS"
			 LDFLAGS="$LDFLAGS $libdw_LDFLAGS"
			 LIBS="$LIBS $libdw_LIBS"

			 AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <elfutils/libdwfl.h>]],
							 [[return dwfl_linux_proc_attach(0, 0, 0)]]
							)
					],
					[AC_MSG_RESULT([yes])],
					[AC_MSG_RESULT([no])
					 AS_IF([test "x$with_libdw" = xyes],
					       [AC_MSG_FAILURE([failed to find dwfl_linux_proc_attach in libdw])],
					      )
					 have_libdw=
					]
				       )

			 LIBS="$saved_LIBS"
			 LDFLAGS="$saved_LDFLAGS"
			],
			[AS_IF([test "x$with_libdw" = xyes],
			       [AC_MSG_FAILURE([failed to find elfutils/libdwfl.h])]
			      )
			 have_libdw=
			]
		       )

       CFLAGS="$saved_CFLAGS"
       CPPFLAGS="$saved_CPPFLAGS"
      ]
)

AS_IF([test "x$have_libdw" = xyes],
      [use_unwinder=libdw
       AC_DEFINE([USE_LIBDW], 1,
		 [Whether to use libdw for stack tracing]
		)
       AC_SUBST(libdw_CPPFLAGS)
       AC_SUBST(libdw_CFLAGS)
       AC_SUBST(libdw_LDFLAGS)
       AC_SUBST(libdw_LIBS)
      ]
     )

])
