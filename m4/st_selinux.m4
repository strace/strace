#!/usr/bin/m4
#
# Copyright (c) 2020-2021 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: LGPL-2.1-or-later

AC_DEFUN([st_SELINUX], [dnl

libselinux_CPPFLAGS=
libselinux_LDFLAGS=
libselinux_LIBS=
enable_secontext=no

AC_ARG_WITH([libselinux],
	    [AS_HELP_STRING([--with-libselinux],
			    [use libselinux to collect security contexts])],
	    [case "${withval}" in
	     yes|no|check) ;;
	     *) with_libselinux=yes
		libselinux_CPPFLAGS="-I${withval}/include"
		libselinux_LDFLAGS="-L${withval}/lib" ;;
	     esac],
	    [with_libselinux=check]
)

AS_IF([test "x$with_libselinux" != xno],
      [saved_CPPFLAGS="$CPPFLAGS"
       CPPFLAGS="$CPPFLAGS $libselinux_CPPFLAGS"
       found_selinux_h=no
       AC_CHECK_HEADERS([selinux/selinux.h],
			[found_selinux_h=yes])
       CPPFLAGS="$saved_CPPFLAGS"
       AS_IF([test "x$found_selinux_h" = xyes],
	     [saved_LDFLAGS="$LDFLAGS"
	      LDFLAGS="$LDFLAGS $libselinux_LDFLAGS"
	      missing=
	      for func in getpidcon getfilecon selabel_open selabel_lookup; do
		AC_CHECK_LIB([selinux], [$func], [:],
			     [missing="$missing $func"])
	      done
	      AS_IF([test "x$missing" = x],
	            [libselinux_LIBS="-lselinux"
		     enable_secontext=yes
		    ],
		    [AS_IF([test "x$with_libselinux" != xcheck],
			   [AC_MSG_FAILURE([failed to find in libselinux:$missing])]
		     )
		    ]
	      )
	      LDFLAGS="$saved_LDFLAGS"
	     ],
	     [AS_IF([test "x$with_libselinux" != xcheck],
		    [AC_MSG_FAILURE([failed to find selinux.h])]
	      )
	     ]
       )
      ]
)

AC_MSG_CHECKING([whether to enable security contexts support])
AS_IF([test "x$enable_secontext" = xyes],
      [AC_DEFINE([ENABLE_SECONTEXT], [1],
			  [Define to enable SELinux security contexts support])
       AC_DEFINE([HAVE_SELINUX_RUNTIME], [1],
			  [Define to enable SELinux security contexts testing])
       AC_SUBST(libselinux_LIBS)
       AC_SUBST(libselinux_LDFLAGS)
       AC_SUBST(libselinux_CPPFLAGS)
       AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])])

AM_CONDITIONAL([ENABLE_SECONTEXT], [test "x$enable_secontext" = xyes])
AM_CONDITIONAL([HAVE_SELINUX_RUNTIME], [test "x$enable_secontext" = xyes])

])
