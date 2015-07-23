AC_DEFUN([st_MPERS],[

pushdef([MPERS_NAME], translit([$1], [a-z], [A-Z]))
pushdef([HAVE_RUNTIME], [HAVE_]MPERS_NAME[_RUNTIME])
pushdef([CFLAG], [-$1])
pushdef([st_cv_cc], [st_cv_$1_cc])
pushdef([st_cv_runtime], [st_cv_$1_runtime])

case "$arch" in
	[$2])
	saved_CFLAGS="$CFLAGS"
	CFLAGS="$CFLAGS CFLAG"
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
	fi
	CFLAGS="$saved_CFLAGS"
	;;

	*)
	st_cv_runtime=no
	;;
esac

AM_CONDITIONAL(HAVE_RUNTIME, [test "$st_cv_runtime" = yes])

popdef([st_cv_runtime])
popdef([st_cv_cc])
popdef([CFLAG])
popdef([HAVE_RUNTIME])
popdef([MPERS_NAME])

])
