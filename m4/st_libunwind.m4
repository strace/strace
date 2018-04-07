#!/usr/bin/m4
#
# Copyright (c) 2013-2018 The strace developers.
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

AC_DEFUN([st_ARG_LIBUNWIND], [dnl

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

libunwind_CPPFLAGS=
libunwind_LDFLAGS=
libunwind_LIBS=

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
