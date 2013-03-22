#!/bin/sh -e

#export CC="i686-gcc"
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

autoreconf -i -f
./configure #--enable-maintainer-mode
make CC="$CC" CFLAGS="$CFLAGS"
