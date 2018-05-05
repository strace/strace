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

: ${libdw_CPPFLAGS=}
: ${libdw_CFLAGS=}
: ${libdw_LDFLAGS=}
: ${libdw_LIBS=}

AC_ARG_WITH([libdw],
	    [AS_HELP_STRING([--with-libdw],
			    [use libdw to implement stack tracing support]
			   )
	    ],
	    [case "${withval}" in
	     yes|no|check) ;;
	     *) libdw_CPPFLAGS="-I${withval}/include"
		libdw_LDFLAGS="-L${withval}/lib"
		with_libdw=yes ;;
	     esac
	    ],
	    [with_libdw=check]
	   )

])

AC_DEFUN([st_LIBDW], [dnl

have_libdw=

AS_IF([test "x$with_libdw" != xno && test "x$use_unwinder" = x],
      [saved_CPPFLAGS="$CPPFLAGS"
       saved_CFLAGS="$CFLAGS"
       CPPFLAGS="$CPPFLAGS $libdw_CPPFLAGS"
       CFLAGS="$CFLAGS $libdw_CFLAGS"

       AC_CHECK_HEADERS([elfutils/libdwfl.h],
			[AC_CHECK_LIB([dw], [dwfl_linux_proc_attach],
				      [libdw_LIBS="-ldw $libdw_LIBS"
				       AC_CACHE_CHECK([for elfutils version],
						      [st_cv_ELFUTILS_VERSION],
						      [[st_cv_ELFUTILS_VERSION="$(echo _ELFUTILS_VERSION |
										  $CPP $CPPFLAGS -P -imacros elfutils/version.h - |
										  grep '^[0-9]')"
							test -n "$st_cv_ELFUTILS_VERSION" ||
								st_cv_ELFUTILS_VERSION=0
						      ]]
						     )
				       AS_IF([test "$st_cv_ELFUTILS_VERSION" -ge 164],
					     [have_libdw=yes],
					     [AS_IF([test "x$with_libdw" = xyes],
						    [AC_MSG_ERROR([elfutils version >= 164 is required for stack tracing support])],
						    [AC_MSG_WARN([elfutils version >= 164 is required for stack tracing support])]
						   )
					     ]
					    )
				      ],
				      [AS_IF([test "x$with_libdw" = xyes],
					     [AC_MSG_FAILURE([failed to find dwfl_linux_proc_attach in libdw])],
					    )
				      ],
				      [$libdw_LDFLAGS $libdw_LIBS]
				     )
			],
			[AS_IF([test "x$with_libdw" = xyes],
			       [AC_MSG_FAILURE([failed to find elfutils/libdwfl.h])]
			      )
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
