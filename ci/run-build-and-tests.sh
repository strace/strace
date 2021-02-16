#!/bin/sh -ex
#
# Copyright (c) 2018-2021 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0-or-later

DISTCHECK_CONFIGURE_FLAGS='--disable-dependency-tracking --enable-gcc-Werror'
export DISTCHECK_CONFIGURE_FLAGS

case "$CC" in
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

CPPFLAGS=
case "$KHEADERS" in
	*/*)
		CPPFLAGS='-isystem /opt/kernel/include'
		export CPPFLAGS
		;;
esac

case "${CHECK-}" in
	coverage)
		DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --enable-code-coverage"
		ac_cv_prog_LCOV=lcov
		ac_cv_prog_GENHTML=genhtml
		export ac_cv_prog_LCOV ac_cv_prog_GENHTML
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
kver="$(printf '%s\n%s\n' '#include <linux/version.h>' 'LINUX_VERSION_CODE' | $CC $CPPFLAGS -E -P -)"
printf 'kernel-headers %s.%s.%s\n' $(($kver/65536)) $(($kver/256%256)) $(($kver%256))
echo 'END OF BUILD ENVIRONMENT INFORMATION'

export CC_FOR_BUILD="$CC"

./build-aux/git-set-file-times
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

nproc="$(nproc)" || nproc=1
j="-j$nproc"
j2="-j$((2*$nproc))"

case "${CHECK-}" in
	coverage)
		make -k $j all VERBOSE=${VERBOSE-} CFLAGS='-g -Og'
		make -k $j2 check VERBOSE=${VERBOSE-}
		echo 'BEGIN OF TEST SUITE INFORMATION'
		tail -n 99999 -- tests*/test-suite.log tests*/ksysent.gen.log
		echo 'END OF TEST SUITE INFORMATION'
		case "$CC" in
			gcc*) GCOV="gcov${CC#gcc}" ;;
			clang*) GCOV="llvm-cov${CC#clang} gcov" ;;
			*) GCOV=gcov ;;
		esac
		cd src
		../codecov.bash -Z -x "$GCOV" -a -abc
		rm ../codecov.bash
		cd -
		;;
	valgrind)
		make -k $j all VERBOSE=${VERBOSE-}
		rc=$?
		for n in ${VALGRIND_TOOLS:-memcheck helgrind drd}; do
			make -k $j2 -C "${VALGRIND_TESTDIR:-.}" \
				check-valgrind-$n VERBOSE=${VERBOSE-} ||
					rc=$?
		done
		echo 'BEGIN OF TEST SUITE INFORMATION'
		tail -n 99999 -- tests*/test-suite*.log tests*/ksysent.gen.log ||
			rc=$?
		echo 'END OF TEST SUITE INFORMATION'
		[ "$rc" -eq 0 ]
		;;
	*)
		make -k $j2 distcheck VERBOSE=${VERBOSE-}
		;;
esac

if git status --porcelain |grep ^.; then
	echo >&2 'git status reported uncleanness'
	exit 1
fi
