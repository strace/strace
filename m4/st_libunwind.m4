#!/usr/bin/m4
#
# Copyright (c) 2013-2020 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: LGPL-2.1-or-later

AC_DEFUN([st_ARG_LIBUNWIND], [dnl

libunwind_CPPFLAGS=
libunwind_LDFLAGS=
libunwind_LIBS=

AC_ARG_WITH([libunwind],
	    [AS_HELP_STRING([--with-libunwind],
			    [use libunwind to implement stack tracing support])],
	    [case "${withval}" in
	     yes|no|check) ;;
	     *) with_libunwind=yes
		libunwind_CPPFLAGS="-I${withval}/include"
		libunwind_LDFLAGS="-L${withval}/lib" ;;
	     esac],
	    [with_libunwind=check]
)

])

AC_DEFUN([st_LIBUNWIND], [dnl

AS_IF([test "x$with_libunwind" != xno && test "x$use_unwinder" = x],
      [saved_CPPFLAGS="$CPPFLAGS"
       CPPFLAGS="$CPPFLAGS $libunwind_CPPFLAGS"

       AC_CHECK_HEADERS([libunwind-ptrace.h],
	 [saved_LDFLAGS="$LDFLAGS"
	  LDFLAGS="$LDFLAGS $libunwind_LDFLAGS"

	  AC_CHECK_LIB([unwind], [backtrace],
	    [libunwind_LIBS="-lunwind $libunwind_LIBS"

	     AC_MSG_CHECKING([for unw_create_addr_space in libunwind-generic])
	     saved_LIBS="$LIBS"
	     LIBS="-lunwind-generic $libunwind_LIBS $LIBS"

	     AC_LINK_IFELSE(
	       [AC_LANG_PROGRAM([[#include <libunwind-ptrace.h>]],
				[[return !unw_create_addr_space(0, 0)]])
	       ],
	       [AC_MSG_RESULT([yes])
		libunwind_LIBS="-lunwind-generic $libunwind_LIBS"

		AC_CHECK_LIB([unwind-ptrace], [_UPT_create],
		  [libunwind_LIBS="-lunwind-ptrace $libunwind_LIBS"
		   use_unwinder=libunwind
		  ],
		  [if test "x$with_libunwind" != xcheck; then
		     AC_MSG_FAILURE([failed to find _UPT_create in libunwind-ptrace])
		   fi
		  ],
		  [$libunwind_LIBS]
		)
	       ],
	       [AC_MSG_RESULT([no])
		if test "x$with_libunwind" != xcheck; then
		  AC_MSG_FAILURE([failed to find unw_create_addr_space in libunwind-generic])
		fi
	       ]
	     )

	     LIBS="$saved_LIBS"
	    ],
	    [if test "x$with_libunwind" != xcheck; then
	       AC_MSG_FAILURE([failed to find libunwind])
	     fi
	    ],
	    [$libunwind_LIBS]
	  )

	  LDFLAGS="$saved_LDFLAGS"
	 ],
	 [if test "x$with_libunwind" != xcheck; then
	    AC_MSG_FAILURE([failed to find libunwind-ptrace.h])
	  fi
	 ]
       )

       CPPFLAGS="$saved_CPPFLAGS"
      ]
)

if test "x$use_unwinder" = xlibunwind; then
	AC_DEFINE([USE_LIBUNWIND], 1,
		  [Whether to use libunwind for stack tracing])
	AC_SUBST(libunwind_LIBS)
	AC_SUBST(libunwind_LDFLAGS)
	AC_SUBST(libunwind_CPPFLAGS)
fi

])
