#!/bin/sh -e

j=-j`getconf _NPROCESSORS_ONLN 2> /dev/null` || j=
set -x
git fetch --unshallow
./git-set-file-times
./bootstrap
./configure --enable-maintainer-mode ${ENABLE_GCC_WERROR-}
make $j distcheck VERBOSE=${VERBOSE-}
