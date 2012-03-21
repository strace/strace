#!/bin/sh

export HDCMEGS=128

rm hdc.img 2>/dev/null
umount -d hdc.img.dir 2>/dev/null
rm -rf hdc.img.dir 2>/dev/null

dd if=/dev/zero of=hdc.img bs=1024 seek=$((HDCMEGS*1024-1)) count=1 &&
mke2fs -q -b 1024 -F -i 4096 hdc.img &&
tune2fs -j -c 0 -i 0 hdc.img &&
mkdir hdc.img.dir &&
mount -o loop hdc.img hdc.img.dir &&
cp -a hdc.dir/* hdc.img.dir &&
umount -d hdc.img.dir &&
rm -rf hdc.img.dir &&
true
