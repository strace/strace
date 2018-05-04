#!/bin/sh -ex

DISTCHECK_CONFIGURE_FLAGS='--disable-dependency-tracking'
export DISTCHECK_CONFIGURE_FLAGS

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

case "${STACKTRACE-}" in
	libdw|libunwind)
		DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --with-$STACKTRACE"
		;;
	no)
		DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --disable-stacktrace"
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
	valgrind)
		DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --enable-valgrind"
		;;
esac

echo 'BEGIN OF BUILD ENVIRONMENT INFORMATION'
uname -a |head -1
libc="$(ldd /bin/sh |sed -n 's|^[^/]*\(/[^ ]*/libc\.so[^ ]*\).*|\1|p' |head -1)"
$libc |head -1
file -L /bin/sh
$CC --version |head -1
$CC -print-multi-lib ||:
make --version |head -1
autoconf --version |head -1
automake --version |head -1
kver="$(printf '%s\n%s\n' '#include <linux/version.h>' 'LINUX_VERSION_CODE' | $CC -E -P -)"
printf 'kernel-headers %s.%s.%s\n' $(($kver/65536)) $(($kver/256%256)) $(($kver%256))
echo 'END OF BUILD ENVIRONMENT INFORMATION'

export CC_FOR_BUILD="$CC"

./git-set-file-times
./bootstrap
./configure --enable-maintainer-mode \
	${DISTCHECK_CONFIGURE_FLAGS-} \
	|| {
	rc=$?
	cat config.log
	echo "$CC -dumpspecs follows"
	$CC -dumpspecs
	exit $rc
}

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
	valgrind)
		make -k $j all VERBOSE=${VERBOSE-}
		rc=$?
		for n in ${VALGRIND_TOOLS:-memcheck helgrind drd}; do
			make -k $j -C "${VALGRIND_TESTDIR:-.}" \
				check-valgrind-$n VERBOSE=${VERBOSE-} ||
					rc=$?
		done
		echo 'BEGIN OF TEST SUITE INFORMATION'
		tail -n 99999 -- tests*/test-suite*.log tests*/ksysent.log ||
			rc=$?
		echo 'END OF TEST SUITE INFORMATION'
		[ "$rc" -eq 0 ]
		;;
	*)
		make -k $j distcheck VERBOSE=${VERBOSE-}
		;;
esac
