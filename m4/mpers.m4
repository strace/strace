#!/usr/bin/m4
#
# Copyright (c) 2015-2017 Dmitry V. Levin <ldv@altlinux.org>
# Copyright (c) 2015 Elvira Khabirova <lineprinter0@gmail.com>
# Copyright (c) 2015-2020 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: LGPL-2.1-or-later

AC_DEFUN([st_MPERS_LOAD_AC_CV], [

pushdef([var], [ac_cv_$1])
pushdef([saved], [saved_ac_cv_$1])
pushdef([mpers], [ac_cv_]mpers_name[_$1])

AS_IF([test -n "${var+set}"], [saved="${var}"; unset var])
AS_IF([test -n "${mpers+set}"], [var="${mpers}"])

popdef([mpers])
popdef([saved])
popdef([var])

])

AC_DEFUN([st_MPERS_SAVE_AC_CV], [

pushdef([var], [ac_cv_$1])
pushdef([saved], [saved_ac_cv_$1])
pushdef([mpers], [ac_cv_]mpers_name[_$1])

AS_IF([test -n "${var+set}"], [mpers="${var}"])
AS_IF([test -n "${saved+set}"], [var="${saved}"; unset saved])

popdef([mpers])
popdef([saved])
popdef([var])

])

AC_DEFUN([st_MPERS_STRUCT_STAT], [

st_MPERS_LOAD_AC_CV([type_struct_stat$1])
AC_CHECK_TYPE([struct stat$1],
	      AC_DEFINE([HAVE_]MPERS_NAME[_STRUCT_STAT$1], [1],
			[Define to 1 if ]mpers_name[ has the type 'struct stat$1'.]),,
[#include <sys/types.h>
#include <asm/stat.h>])
st_MPERS_SAVE_AC_CV([type_struct_stat$1])

st_MPERS_LOAD_AC_CV([member_struct_stat$1_st_mtime_nsec])
AC_CHECK_MEMBER([struct stat$1.st_mtime_nsec],
		AC_DEFINE([HAVE_]MPERS_NAME[_STRUCT_STAT$1_ST_MTIME_NSEC], [1],
			  [Define to 1 if 'st_mtime_nsec' is a member of ]mpers_name[ 'struct stat$1'.]),,
[#include <sys/types.h>
#include <asm/stat.h>])
st_MPERS_SAVE_AC_CV([member_struct_stat$1_st_mtime_nsec])

])

AC_DEFUN([st_MPERS],[

pushdef([mpers_name], [$1])
pushdef([MPERS_NAME], translit([$1], [a-z], [A-Z]))
pushdef([HAVE_MPERS], [HAVE_]MPERS_NAME[_MPERS])
pushdef([HAVE_RUNTIME], [HAVE_]MPERS_NAME[_RUNTIME])
pushdef([MPERS_CFLAGS], [$cc_flags_$1])
pushdef([st_cv_cc], [st_cv_$1_cc])
pushdef([st_cv_runtime], [st_cv_$1_runtime])
pushdef([st_cv_mpers], [st_cv_$1_mpers])

pushdef([EXEEXT], MPERS_NAME[_EXEEXT])dnl
pushdef([OBJEXT], MPERS_NAME[_OBJEXT])dnl
pushdef([LDFLAGS], [LDFLAGS_FOR_]MPERS_NAME)dnl
pushdef([WARN_CFLAGS], [WARN_CFLAGS_FOR_]MPERS_NAME)dnl

st_SAVE_VAR([CC])
st_SAVE_VAR([CPP])
st_SAVE_VAR([CFLAGS])
st_SAVE_VAR([CPPFLAGS])

CC=[$CC_FOR_]MPERS_NAME
CPP=[$CPP_FOR_]MPERS_NAME
CFLAGS=[$CFLAGS_FOR_]MPERS_NAME
CPPFLAGS=[$CPPFLAGS_FOR_]MPERS_NAME

case "$arch" in
	[$2])
	case "$enable_mpers" in
	yes|check|[$1])

	AH_TEMPLATE([HAVE_GNU_STUBS_32_H],
		    [Define to 1 if you have the <gnu/stubs-32.h> header file.])
	AH_TEMPLATE([HAVE_GNU_STUBS_X32_H],
		    [Define to 1 if you have the <gnu/stubs-x32.h> header file.])
	pushdef([gnu_stubs], [gnu/stubs-][m4_substr([$1], 1)][.h])
	AC_CHECK_HEADERS([gnu_stubs], [IFLAG=],
			 [mkdir -p gnu
			  : > gnu_stubs
			  AC_MSG_NOTICE([Created empty gnu_stubs])
			  IFLAG=-I.])
	popdef([gnu_stubs])
	saved_CPPFLAGS="$CPPFLAGS"
	CPPFLAGS="$CPPFLAGS${IFLAG:+ }$IFLAG"
	saved_CFLAGS="$CFLAGS"
	CFLAGS="$CFLAGS MPERS_CFLAGS"
	AC_CACHE_CHECK([for mpers_name personality compile support (using $CC $CPPFLAGS $CFLAGS)],
		[st_cv_cc],
		[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <stdint.h>]],
						    [[return 0]])],
				   [st_cv_cc=yes],
				   [st_cv_cc=no])])
	if test $st_cv_cc = yes; then
		AC_CACHE_CHECK([for mpers_name personality runtime support],
			[st_cv_runtime],
			[AC_RUN_IFELSE([AC_LANG_PROGRAM([[#include <stdint.h>]],
							[[return 0]])],
				       [st_cv_runtime=yes],
				       [st_cv_runtime=no],
				       [st_cv_runtime=no])])
		AC_CACHE_CHECK([whether mpers.sh mpers_name MPERS_CFLAGS works],
			[st_cv_mpers],
			[if READELF="$READELF" \
			    CC="$CC" CPP="$CPP" CPPFLAGS="$CPPFLAGS" \
			    $srcdir/mpers_test.sh [$1] "MPERS_CFLAGS"; then
				st_cv_mpers=yes
			 else
				st_cv_mpers=no
			 fi])
		if test $st_cv_mpers = yes; then
			AC_DEFINE(HAVE_MPERS, [1],
				  [Define to 1 if you have mpers_name mpers support])
			st_MPERS_STRUCT_STAT([])
			st_MPERS_STRUCT_STAT([64])

			if test $st_cv_runtime = yes; then
				pushdef([SIZEOF_LONG],
					MPERS_NAME[_SIZEOF_LONG])
				st_MPERS_LOAD_AC_CV([sizeof_long])
				AC_CHECK_SIZEOF([long])
				st_MPERS_SAVE_AC_CV([sizeof_long])
				popdef([SIZEOF_LONG])

				pushdef([SIZEOF_KERNEL_LONG_T],
					MPERS_NAME[_SIZEOF_KERNEL_LONG_T])
				st_MPERS_LOAD_AC_CV([sizeof_kernel_long_t])
				AC_CHECK_SIZEOF([kernel_long_t],,
						[#include "$srcdir/kernel_types.h"])
				st_MPERS_SAVE_AC_CV([sizeof_kernel_long_t])
				popdef([SIZEOF_KERNEL_LONG_T])

				pushdef([SIZEOF_STRUCT_MSQID64_DS],
					MPERS_NAME[_SIZEOF_STRUCT_MSQID64_DS])
				st_MPERS_LOAD_AC_CV([sizeof_struct_msqid64_ds])
				AC_CHECK_SIZEOF([struct msqid64_ds],,
						[#include <linux/msg.h>])
				st_MPERS_SAVE_AC_CV([sizeof_struct_msqid64_ds])
				popdef([SIZEOF_STRUCT_MSQID64_DS])
			fi
		fi
	fi
	CPPFLAGS="$saved_CPPFLAGS"
	CFLAGS="$saved_CFLAGS"
	;;

	*) # case "$enable_mpers"
	st_cv_runtime=no
	st_cv_mpers=no
	;;
	esac

	test "$st_cv_mpers" = yes ||
		st_cv_mpers=no
	AC_MSG_CHECKING([whether to enable $1 personality support])
	AC_MSG_RESULT([$st_cv_mpers])

	case "$enable_mpers,$st_cv_mpers" in
	yes,no|[$1],no)
		AC_MSG_ERROR([Cannot enable $1 personality support])
		;;
	esac
	;;

	*) # case "$arch"
	st_cv_runtime=no
	st_cv_mpers=no
	;;
esac

AM_CONDITIONAL(HAVE_RUNTIME, [test "$st_cv_mpers$st_cv_runtime" = yesyes])
AM_CONDITIONAL(HAVE_MPERS, [test "$st_cv_mpers" = yes])

st_RESTORE_VAR([CC])
st_RESTORE_VAR([CPP])
st_RESTORE_VAR([CFLAGS])
st_RESTORE_VAR([CPPFLAGS])

popdef([WARN_CFLAGS])dnl
popdef([LDFLAGS])dnl
popdef([OBJEXT])dnl
popdef([EXEEXT])dnl

popdef([st_cv_mpers])
popdef([st_cv_runtime])
popdef([st_cv_cc])
popdef([MPERS_CFLAGS])
popdef([HAVE_RUNTIME])
popdef([HAVE_MPERS])
popdef([MPERS_NAME])
popdef([mpers_name])

])
