#!/bin/sh -ex

case "$CC" in
	gcc)
		ENABLE_GCC_WERROR=--enable-gcc-Werror
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
		DISTCHECK_CONFIGURE_FLAGS='--build=i686-pc-linux-gnu --target=i686-pc-linux-gnu'
		export DISTCHECK_CONFIGURE_FLAGS
		;;
esac

case "${CHECK-}" in
	coverage)
		CHECK_CONFIGURE_FLAGS=--enable-code-coverage
		CFLAGS='-g -O0'
		CFLAGS_FOR_BUILD="$CFLAGS"
		export CFLAGS CFLAGS_FOR_BUILD
		;;
esac

$CC --version
export CC_FOR_BUILD="$CC"

./git-set-file-times
./bootstrap
./configure --enable-maintainer-mode \
	${ENABLE_GCC_WERROR-} \
	${DISTCHECK_CONFIGURE_FLAGS-} \
	${CHECK_CONFIGURE_FLAGS-} \
	#

j=-j`getconf _NPROCESSORS_ONLN 2> /dev/null` || j=

case "${CHECK-}" in
	coverage)
		make -k $j all VERBOSE=${VERBOSE-}
		make -k $j check VERBOSE=${VERBOSE-}
		codecov --gcov-args=-abcp ||:
		;;
	*)
		make -k $j distcheck VERBOSE=${VERBOSE-}
		;;
esac
