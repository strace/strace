#!/usr/bin/m4
#
# Copyright (c) 2018 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: LGPL-2.1-or-later

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
