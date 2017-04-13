#!/bin/sh -ex

case "$CC" in
	gcc*)
		DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --enable-gcc-Werror"
		;;
	clang-*)
		# clang -mx32 fails with the following error:
		# clang: error: clang frontend command failed with exit code 70 (use -v to see invocation)
		export st_cv_mx32_runtime=no
		;;
esac

case "${TARGET-}" in
	x32)
		CC="$CC -mx32"
		;;
	x86)
		CC="$CC -m32"
		DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --build=i686-pc-linux-gnu --target=i686-pc-linux-gnu"
		;;
esac

case "$KHEADERS" in
	*/*)
		CPPFLAGS='-isystem /opt/kernel/include'
		export CPPFLAGS
		;;
esac

case "${CHECK-}" in
	coverage)
		DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --enable-code-coverage"
		CFLAGS='-g -O0'
		CFLAGS_FOR_BUILD="$CFLAGS"
		export CFLAGS CFLAGS_FOR_BUILD
		;;
esac

$CC --version
export CC_FOR_BUILD="$CC"

[ -z "${DISTCHECK_CONFIGURE_FLAGS-}" ] ||
	export DISTCHECK_CONFIGURE_FLAGS

./git-set-file-times
./bootstrap
./configure --enable-maintainer-mode \
	${DISTCHECK_CONFIGURE_FLAGS-} \
	#

j=-j`getconf _NPROCESSORS_ONLN 2> /dev/null` || j=

case "${CHECK-}" in
	coverage)
		make -k $j all VERBOSE=${VERBOSE-}
		make -k $j check VERBOSE=${VERBOSE-}
		codecov --gcov-args=-abcp ||:
		echo 'BEGIN OF TEST SUITE INFORMATION'
		tail -n 99999 -- tests*/test-suite.log tests*/ksysent.log
		echo 'END OF TEST SUITE INFORMATION'
		;;
	*)
		make -k $j distcheck VERBOSE=${VERBOSE-}
		;;
esac
