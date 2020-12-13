#!/usr/bin/m4
#
# Copyright (c) 2017-2020 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: LGPL-2.1-or-later

AC_DEFUN([st_DEMANGLE], [dnl

libiberty_CPPFLAGS=
libiberty_LDFLAGS=
libiberty_LIBS=

AC_ARG_WITH([libiberty],
	    [AS_HELP_STRING([--with-libiberty],
			    [use libiberty to demangle symbols in stack trace])],
	    [case "${withval}" in
	     yes|no|check) ;;
	     *) with_libiberty=yes
		libiberty_CPPFLAGS="-I${withval}/include"
		libiberty_LDFLAGS="-L${withval}/lib" ;;
	     esac],
	    [with_libiberty=check]
)

use_libiberty=no

AS_IF([test "x$with_libiberty" != xno],
      [saved_CPPFLAGS="$CPPFLAGS"
       CPPFLAGS="$CPPFLAGS $libiberty_CPPFLAGS"
       found_demangle_h=no
       AC_CHECK_HEADERS([demangle.h libiberty/demangle.h],
			[found_demangle_h=yes])
       CPPFLAGS="$saved_CPPFLAGS"
       AS_IF([test "x$found_demangle_h" = xyes],
	     [saved_LDFLAGS="$LDFLAGS"
	      LDFLAGS="$LDFLAGS $libiberty_LDFLAGS"
	      AC_CHECK_LIB([iberty],[cplus_demangle],
		[libiberty_LIBS="-liberty"
		 use_libiberty=yes
		],
		[if test "x$with_libiberty" != xcheck; then
		   AC_MSG_FAILURE([failed to find cplus_demangle in libiberty])
		 fi
		]
	      )
	      LDFLAGS="$saved_LDFLAGS"
	     ],
	     [if test "x$with_libiberty" != xcheck; then
		AC_MSG_FAILURE([failed to find demangle.h])
	      fi
	     ]
       )
      ]
)

AC_MSG_CHECKING([whether to enable symbols demangling in stack trace])
if test "x$use_libiberty" = xyes; then
	AC_DEFINE([USE_DEMANGLE], 1, [Do symbols demangling in stack trace])
	AC_SUBST(libiberty_LIBS)
	AC_SUBST(libiberty_LDFLAGS)
	AC_SUBST(libiberty_CPPFLAGS)
fi
AC_MSG_RESULT([$use_libiberty])

])
