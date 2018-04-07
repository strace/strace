#!/usr/bin/m4
#
# Copyright (c) 2017-2018 The strace developers.
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

AC_DEFUN([st_DEMANGLE], [dnl

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

libiberty_CPPFLAGS=
libiberty_LDFLAGS=
libiberty_LIBS=
use_libiberty=no

AS_IF([test "x$with_libiberty" != xno],
      [saved_CPPFLAGS="$CPPFLAGS"
       CPPFLAGS="$CPPFLAGS $libiberty_CPPFLAGS"
       AC_CHECK_HEADERS([demangle.h libiberty/demangle.h],
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
       CPPFLAGS="$saved_CPPFLAGS"
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
