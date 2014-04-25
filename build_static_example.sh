#!/bin/sh -e

BUILDFLAG=""

#BUILDFLAG="--build=i686"
#export CC="i686-gcc"
# -mpreferred-stack-boundary=2 can be used to prevent gcc 4.2.x
# from aligning stack to 16 bytes. (Which is gcc's way of supporting SSE).
# For me it saves about 6k of text segment.
# This may be unsafe if your libc expects 16 byte stack alignment
# on function entry.

export CC="x86_64-gcc"

export CFLAGS="-Os\
 -fomit-frame-pointer\
 -static\
 -static-libgcc\
 -ffunction-sections -fdata-sections\
 -falign-functions=1 -falign-jumps=1 -falign-labels=1 -falign-loops=1\
 -fno-unwind-tables\
 -fno-asynchronous-unwind-tables\
 -Wl,--gc-sections\
 -Wl,-Map=strace.mapfile\
"

./bootstrap
./configure $BUILDFLAG #--enable-maintainer-mode
make CC="$CC" CFLAGS="$CFLAGS"
