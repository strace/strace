#!/bin/sh -ex

case "$CC" in
  gcc)
    ENABLE_GCC_WERROR=--enable-gcc-Werror
    ;;
  clang-*)
    sudo apt-get -qq update
    sudo apt-get -qq --no-install-suggests --no-install-recommends install -y \
      "$CC"
    # clang -mx32 fails with the following error:
    # clang: error: clang frontend command failed with exit code 70 (use -v to see invocation)
    export st_cv_mx32_runtime=no
    ;;
  musl-gcc)
    sudo add-apt-repository ppa:bortis/musl -y
    sudo apt-get -qq update
    sudo apt-get -qq --no-install-suggests --no-install-recommends install -y \
      musl-tools linux-musl-dev
    ;;
esac

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

$CC --version

git fetch --unshallow
./git-set-file-times
./bootstrap
./configure --enable-maintainer-mode ${ENABLE_GCC_WERROR-} ${DISTCHECK_CONFIGURE_FLAGS-}
j=-j`getconf _NPROCESSORS_ONLN 2> /dev/null` || j=
make -k $j distcheck VERBOSE=${VERBOSE-}
