#!/bin/sh -e

j=-j`getconf _NPROCESSORS_ONLN 2> /dev/null` || j=
set -x
git fetch --unshallow
./git-set-file-times
case "${TARGET-}" in
  x32)
    CC="$CC -mx32"
    ;;
  x86)
    export DISTCHECK_CONFIGURE_FLAGS='--build=i686-pc-linux-gnu'
    CC="$CC -m32"
    ;;
esac
export CC_FOR_BUILD="$CC"
./bootstrap
./configure --enable-maintainer-mode ${ENABLE_GCC_WERROR-} ${DISTCHECK_CONFIGURE_FLAGS-}
make -k $j distcheck VERBOSE=${VERBOSE-}
