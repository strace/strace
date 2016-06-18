#!/bin/sh -ex

if [ "${COVERAGE-}" = true ]; then
	set -- strace-*.tar.xz
	tar -xf "$1"
	dir="${1%.tar.xz}"
	cd "$dir"
	export CC_FOR_BUILD="$CC"
	./configure --enable-code-coverage
	make -k $j all check VERBOSE=${VERBOSE-}
	codecov --gcov-args=-abcp ||:
fi
