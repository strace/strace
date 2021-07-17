#!/usr/bin/m4
#
# Copyright (c) 2016-2021 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: LGPL-2.1-or-later

AC_DEFUN([st_WARN_CFLAGS], [dnl
AC_CACHE_CHECK([whether $[]_AC_CC[] is fresh enough for -Werror],
	       [st_cv_cc_enable_Werror],
               [AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#if defined __GNUC__ && defined __GNUC_MINOR__
# define GNUC_PREREQ(maj, min) ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
#else
# define GNUC_PREREQ(maj, min) 0
#endif

#if defined __clang__ && defined __clang_major__ && defined __clang_minor__
# define CLANG_PREREQ(maj, min) ((__clang_major__ << 16) + __clang_minor__ >= ((maj) << 16) + (min))
#else
# define CLANG_PREREQ(maj, min) 0
#endif
]],
			       [[int i[GNUC_PREREQ(4, 8) + CLANG_PREREQ(6, 0) > 0 ? 1 : - 1];]])],
			       [st_cv_cc_enable_Werror=yes],
			       [st_cv_cc_enable_Werror=no])])
AC_CACHE_CHECK([whether to try $[]_AC_CC[] with -Werror by default],
	       [st_cv_enable_Werror],
	       [if test "$st_cv_cc_enable_Werror" != yes; then
		  st_cv_enable_Werror='no, the compiler is too old'
		elif test "$arch_enable_Werror" != yes; then
		  st_cv_enable_Werror='no, architecture is not ready'
		else
		  st_cv_enable_Werror=yes
		fi])
gl_WARN_ADD([-Wall])
gl_WARN_ADD([-Wextra])
gl_WARN_ADD([-Wno-missing-field-initializers])
gl_WARN_ADD([-Wno-unused-parameter])

gl_WARN_ADD([-Wdate-time])
gl_WARN_ADD([-Wformat-security])
gl_WARN_ADD([-Wimplicit-fallthrough=5])
gl_WARN_ADD([-Winit-self])
gl_WARN_ADD([-Winitializer-overrides])
gl_WARN_ADD([-Wlogical-op])
gl_WARN_ADD([-Wmissing-prototypes])
gl_WARN_ADD([-Wnested-externs])
gl_WARN_ADD([-Wold-style-definition])
gl_WARN_ADD([-Wtrampolines])
gl_WARN_ADD([-Wundef])
gl_WARN_ADD([-Wwrite-strings])
AC_ARG_ENABLE([gcc-Werror],
	      [AS_HELP_STRING([--enable-gcc-Werror],
			      [turn on gcc's -Werror option])],
	      [case "$enable_gcc_Werror" in
		yes|no|no,*) ;;
		*) AC_MSG_ERROR([bad value $enable_gcc_Werror for gcc-Werror option]) ;;
	       esac],
	      [enable_gcc_Werror="$st_cv_enable_Werror"])
if test "$enable_gcc_Werror" = yes; then
  gl_WARN_ADD([-Werror])
fi
AC_SUBST([WARN_CFLAGS])
])
