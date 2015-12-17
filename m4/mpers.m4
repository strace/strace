#!/usr/bin/m4
#
# Copyright (c) 2015 Dmitry V. Levin <ldv@altlinux.org>
# Copyright (c) 2015 Elvira Khabirova <lineprinter0@gmail.com>
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

AC_DEFUN([st_MPERS],[

pushdef([MPERS_NAME], translit([$1], [a-z], [A-Z]))
pushdef([HAVE_MPERS], [HAVE_]MPERS_NAME[_MPERS])
pushdef([HAVE_RUNTIME], [HAVE_]MPERS_NAME[_RUNTIME])
pushdef([CFLAG], [-$1])
pushdef([st_cv_cc], [st_cv_$1_cc])
pushdef([st_cv_runtime], [st_cv_$1_runtime])
pushdef([st_cv_mpers], [st_cv_$1_mpers])

case "$arch" in
	[$2])
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
	saved_CFLAGS="$CFLAGS"
	CFLAGS="$CFLAGS CFLAG $IFLAG"
	AC_CACHE_CHECK([for CFLAG compile support], [st_cv_cc],
		[AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include <stdint.h>
						     int main(){return 0;}]])],
				   [st_cv_cc=yes],
				   [st_cv_cc=no])])
	if test $st_cv_cc = yes; then
		AC_CACHE_CHECK([for CFLAG runtime support], [st_cv_runtime],
			[AC_RUN_IFELSE([AC_LANG_SOURCE([[#include <stdint.h>
							 int main(){return 0;}]])],
				       [st_cv_runtime=yes],
				       [st_cv_runtime=no],
				       [st_cv_runtime=no])])
		AC_CACHE_CHECK([whether mpers.sh CFLAG works], [st_cv_mpers],
			[if CC="$CC" CPP="$CPP" CPPFLAGS="$CPPFLAGS" \
			    $srcdir/mpers_test.sh [$1]; then
				st_cv_mpers=yes
			 else
				st_cv_mpers=no
			 fi])
		if test $st_cv_mpers = yes; then
			AC_DEFINE(HAVE_MPERS, [1],
				  [Define to 1 if you have CFLAG mpers support])
		fi
	fi
	CFLAGS="$saved_CFLAGS"
	;;

	*)
	st_cv_runtime=no
	st_cv_mpers=no
	;;
esac

AM_CONDITIONAL(HAVE_RUNTIME, [test "$st_cv_runtime" = yes])
AM_CONDITIONAL(HAVE_MPERS, [test "$st_cv_mpers" = yes])

popdef([st_cv_mpers])
popdef([st_cv_runtime])
popdef([st_cv_cc])
popdef([CFLAG])
popdef([HAVE_RUNTIME])
popdef([HAVE_MPERS])
popdef([MPERS_NAME])

])
